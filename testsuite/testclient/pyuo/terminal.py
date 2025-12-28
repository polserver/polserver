#!/usr/bin/env python3

'''
A curses-based UO client
'''

import os
import sys
import time
import logging
import collections
import curses
import curses.textpad
import curses.panel

from pyuo import client
from pyuo import brain


class Ui(brain.Brain):
  ''' Handles the user interface '''

  colors = None
  last_text = ''

  def __init__(self, stdScreen, host, port, writeLog=False, logLevel=logging.INFO):
    for i in range(1, 8):
      curses.init_pair(i, i, 0)
    Ui.colors = {
      'red': curses.color_pair(curses.COLOR_RED),
      'green': curses.color_pair(curses.COLOR_GREEN),
      'yellow': curses.color_pair(curses.COLOR_YELLOW),
      'blue': curses.color_pair(curses.COLOR_BLUE),
      'magenta': curses.color_pair(curses.COLOR_MAGENTA),
      'cyan': curses.color_pair(curses.COLOR_CYAN),
      'white': curses.color_pair(curses.COLOR_WHITE),
    }
    curses.curs_set(0)

    # Main Screen and panel
    self.scr = stdScreen
    self.scr.clear()
    self.scr.border(0)
    self.scr.nodelay(True)
    self.scr.keypad(1)
    self.panel = curses.panel.new_panel(self.scr)

    # Init logging
    rootLog = logging.getLogger()
    rootLog.setLevel(logging.DEBUG)
    compactFmt = logging.Formatter('%(name)s.%(levelname)s: %(message)s')
    if writeLog:
      verboseFmt = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
      logFile = os.path.join(
        os.path.dirname(os.path.abspath(__file__)),
        os.path.splitext(os.path.basename(os.path.abspath(__file__)))[0]+'.log'
      )
      fileHandler = logging.FileHandler(logFile, mode='w')
      fileHandler.setFormatter(verboseFmt)
      rootLog.addHandler(fileHandler)
    self.log = logging.getLogger('ui')
    
    # Map window
    self.mwin = MapWindow(self.scr)

    # Status window
    self.swin = StatusWindow(self.scr)
    self.swin.updLabel('status', 'initializing')

    # Log window
    self.lwin = LogWindow(self.scr)
    self.logHandler = UiLogHandler(self.lwin, logLevel)
    self.logHandler.setFormatter(compactFmt)
    rootLog.addHandler(self.logHandler)

    self.refreshAll()
    
    self.updLogLvlDisplay()
    
    # Login
    while True:
      # Ask for username and password
      user = InputDialog(self.scr, 'Username:', 25).edit()
      pwd = InputDialog(self.scr, 'Password:', 25).edit()

      # Connect to server
      self.updStatus('logging in {}:{}'.format(host,port))
      self.refreshAll()
      cli = client.Client()
      try:
        servers = cli.connect(host, port, user, pwd)
      except client.LoginDeniedError as e:
        self.updStatus('login denied ({})'.format(e))
      except ConnectionRefusedError as e:
        self.updStatus(str(e))
      else:
        break

    self.updStatus('logged in as {}'.format(user))

    # Select server
    self.log.debug(servers)
    serverList = [ i['name'] for i in servers ]
    idx = SelectDialog(self.scr, 'Select a server', serverList, 30, 5).select()
    self.server = servers[idx]

    self.updStatus('selecting server {}'.format(self.server['name']))
    self.refreshAll()

    # Select character
    chars = cli.selectServer(self.server['idx'])
    self.log.debug(chars)
    charList = [ i['name'] for i in chars ]
    idx = SelectDialog(self.scr, 'Select a character', charList, 30, 5).select()
    self.char = chars[idx]

    self.updStatus('selecting character {}'.format(self.char['name']))
    self.refreshAll()

    # Start brain
    cli.selectCharacter(self.char['name'], idx)
    self.updStatus('entering Britannia...')
    super().__init__(cli)

  def init(self):
    self.timeout = 0.1
    p = self.client.player
    self.updStatus('playing {}@{}'.format(self.char['name'], self.server['name']))
    self.updVitals()
    self.updMisc(p.status, p.war, p.notoriety)
    self.updAspect(p.serial, p.graphic, p.color)
    self.updPosition(p.x, p.y, p.z, p.facing, p.notoriety)

  def loop(self):
    self.processInput()

  def refreshAll(self):
    self.scr.noutrefresh()
    self.mwin.noutrefresh()
    self.swin.noutrefresh()
    self.lwin.noutrefresh()
    curses.doupdate()

  def updStatus(self, text):
    self.log.info(text)
    self.swin.updLabel('status', text)
    self.swin.refresh()

  def updLogLvlDisplay(self):
    level = self.logHandler.getLevel()
    if level == logging.DEBUG:
      levelName = 'DEBUG'
    elif level == logging.INFO:
      levelName = 'INFO'
    elif level == logging.WARNING:
      levelName = 'WARNING'
    elif level == logging.ERROR:
      levelName = 'ERROR'
    elif level == logging.CRITICAL:
      levelName = 'CRITICAL'
    else:
      levelName = "UNKNOWN{}".format(level)
    ##TODO: move help on a dedicated panel to be shown at startup
    help = '(press: "v" to cycle; "enter" to talk, arrows to move)'
    self.lwin.updTitle("Verbosity: {}+ {}".format(levelName, help))
    self.lwin.refresh()

  def formatVal(self, val, hex=False):
    if val is None:
      return '?'
    if isinstance(val, int):
      return "0x{:02X}".format(val) if hex else str(val)
    return str(val)

  def updVitals(self):
    p = self.client.player
    fv = self.formatVal
    text = "hp {}/{}, mana {}/{}, stam {}/{}".format(
        fv(p.hp), fv(p.maxhp), fv(p.mana), fv(p.maxmana), fv(p.stam), fv(p.maxstam))
    self.swin.updLabel('vitals', text)
    self.swin.refresh()

  def updAspect(self, serial, graphic, color):
    fh = lambda i: self.formatVal(i, True)
    fv = self.formatVal
    text = "ser: {}, gra: {}, col: {}".format(
        fh(serial), fh(graphic), fv(color))
    self.swin.updLabel('aspect', text)
    self.swin.refresh()

  def updMisc(self, status, war, notoriety):
    stastr = []
    if status is None:
      stastr.append('?')
    else:
      if status & 0x01:
        stastr.append('unk1')
      if status & 0x02:
        stastr.append('modpd')
      if status & 0x04:
        stastr.append('poison')
      if status & 0x08:
        stastr.append('golden')
      if status & 0x10:
        stastr.append('unk10')
      if status & 0x20:
        stastr.append('unk20')
      if status & 0x40:
        stastr.append('war')
      if not len(stastr):
        stastr.append('normal')
    stastr = ','.join(stastr)

    if war is None:
      warstr = '?'
    elif war:
      warstr = '✓'
    else:
      warstr = '✗'

    if notoriety is None:
      notostr = '?'
    elif notoriety == client.Mobile.NOTO_INNOCENT:
      notostr = 'innocent'
    elif notoriety == client.Mobile.NOTO_FRIEND:
      notostr = 'friend'
    elif notoriety == client.Mobile.NOTO_ANIMAL:
      notostr = 'animal'
    elif notoriety == client.Mobile.NOTO_CRIMINAL:
      notostr = 'criminal'
    elif notoriety == client.Mobile.NOTO_ENEMY:
      notostr = 'enemy'
    elif notoriety == client.Mobile.NOTO_MURDERER:
      notostr = 'murderer'
    elif notoriety == client.Mobile.NOTO_INVUL:
      notostr = 'invul'
    else:
      notostr = 'unknown'

    text = "sta: {}, war: {}, noto: {}".format(stastr, warstr, notostr)
    self.swin.updLabel('misc', text)
    self.swin.refresh()

  def updPosition(self, x, y, z, facing, notoriety):
    self.mwin.updPosition(x, y, z, facing, notoriety)
    self.updObjects()

  def updObjects(self):
    objs = []
    for obj in self.client.objects.values():
      if isinstance(obj,client.Mobile) and obj.serial == self.player.serial:
        continue
      if obj.x is None or obj.y is None:
        continue
      objs.append(obj)
    self.mwin.updObjects(objs, self.client.view_range)
    self.mwin.refresh()

  def processInput(self):
    ''' Gets next character from the input, if any; discard the rest
    @return True if any input has been processed
    '''
    key = self.scr.getch()
    if key >= 0:
      if key == ord('v'):
        self.cycleLogLevel()
      elif key == ord('\n'):
        self.speak()
      elif key == curses.KEY_DOWN:
        self.move(client.Direction(client.Direction.S))
      elif key == curses.KEY_UP:
        self.move(client.Direction(client.Direction.N))
      elif key == curses.KEY_LEFT:
        self.move(client.Direction(client.Direction.W))
      elif key == curses.KEY_RIGHT:
        self.move(client.Direction(client.Direction.E))
      elif key == ord('l'):
        self.last_cmd()
      else:
        self.log.warning('Unknown command "%s"', curses.keyname(key).decode('ascii'))
    curses.flushinp()

  def cycleLogLevel(self):
    ''' Move to next log level '''
    level = self.logHandler.getLevel()
    # Debug omitted: too noisy, breaks screen
    if level == logging.INFO:
      newLevel = logging.WARNING
    elif level == logging.WARNING:
      newLevel = logging.ERROR
    elif level == logging.ERROR:
      newLevel = logging.CRITICAL
    elif level == logging.CRITICAL:
      newLevel = logging.INFO
    else:
      newLevel = logging.INFO
    self.logHandler.setLevel(newLevel)
    self.updLogLvlDisplay()

  def speak(self):
    ''' Asks input to the user and sends it as speech to the server '''
    text = InputDialog(self.scr, 'Write down your message:', 70).edit()
    self.last_text=text
    self.client.say(text)

  def move(self, dir):
    ''' Sends move request to the server '''
    self.client.move(dir)

  def last_cmd(self):
    self.client.say(self.last_text)

  # Client events ------------------------------------------------------------

  def onEvent(self, ev):
    if ev.type == brain.Event.EVT_HP_CHANGED:
      self.onHpChange(ev.old, ev.new)
    elif ev.type == brain.Event.EVT_MANA_CHANGED:
      self.onManaChange(ev.old, ev.new)
    elif ev.type == brain.Event.EVT_STAM_CHANGED:
      self.onStamChange(ev.old, ev.new)
    elif ev.type == brain.Event.EVT_SPEECH:
      self.onSpeech(ev.speech)
    elif ev.type == brain.Event.EVT_NOTORIETY:
      self.onNotorietyChange(ev.old, ev.new)
    elif ev.type == brain.Event.EVT_MOVED:
      self.onMovement(ev.oldx, ev.oldy, ev.oldz, ev.oldfacing,
            ev.x, ev.y, ev.z, ev.facing, ev.ack)
    elif ev.type == brain.Event.EVT_NEW_MOBILE:
      self.onNewMobile(ev.mobile)
    elif ev.type == brain.Event.EVT_NEW_ITEM:
      self.onNewItem(ev.item)
    elif ev.type == brain.Event.EVT_REMOVED_OBJ:
      self.onRemovedObj(ev.serial)
    elif ev.type == brain.Event.EVT_OWNCREATE:
      self.onOwnCreate()
    elif ev.type == brain.Event.EVT_BOAT_MOVED:
      self.onBoatMoved(ev)
    elif ev.type == brain.Event.EVT_CLIENT_CRASH:
      self.log.critical('Oops! Client crashed: {}'.format(ev.exception))
      raise RuntimeError('Oops! Client crashed')
    else:
        self.log.critical("Unknown event {}:{}".format(ev.typestr(),ev.type))
  
  def onOwnCreate(self):
    self.updVitals()
    self.onNotorietyChange(None,None)

  def onHpChange(self, old, new):
    self.updVitals()

  def onManaChange(self, old, new):
    self.updVitals()

  def onStamChange(self, old, new):
    self.updVitals()

  def onNotorietyChange(self, old, new):
    p = self.client.player
    self.updMisc(p.status, p.war, p.notoriety)
    self.updPosition(p.x, p.y, p.z, p.facing, p.notoriety)

  def onSpeech(self, speech):
    self.lwin.append(str(speech))

  def onMovement(self, oldx, oldy, oldz, oldfacing, x, y, z, facing, ack):
    self.updPosition(x, y, z, facing, self.client.player.notoriety)
    self.updVitals()

  def onNewMobile(self, mobile):
    self.updObjects()
  
  def onNewItem(self, item):
    self.updObjects()
  
  def onBoatMoved(self, ev):
    p = self.client.player
    self.updPosition(p.x, p.y, p.z, p.facing, p.notoriety)
    self.updObjects()

  def onRemovedObj(self, serial):
    self.mwin.removeObject(serial)
    self.mwin.refresh()

class CursesWinProxy:
  ''' Proxy class over curses window '''

  def __init__(self, nlines, ncols, beginy, beginx, parent=None):
    ''' Creates a new window (curses.newwin()),
    or subbwindow (parent.subwin()) if parent is given '''
    if parent:
      self.win = parent.subwin(nlines, ncols, beginy, beginx)
    else:
      self.win = curses.newwin(nlines, ncols, beginy, beginx)

  def panel(self):
    ''' Create and return a new panel based on this window '''
    return curses.panel.new_panel(self.win)

  def textbox(self):
    ''' Create and return a new TextBox based on this window '''
    return curses.textpad.Textbox(self.win)

  def sanitize(self, str):
    ''' Replaces control chars in the given string
    so that they don't interfere with curses '''
    out = ''
    for char in str:
      code = ord(char)
      if code < 0x20:
        out += chr(0x2400 + code)
      elif code == 0x7f:
        out += '␡'
      elif code == 0xff:
        out += '␠'
      else:
        out += char
    return out

  def addch(self, y, x, ch, attr=0):
    ''' Like self.win.addch() but with sanitization '''
    self.win.addch(y, x, self.sanitize(ch), attr)

  def addnstr(self, y, x, str, n, attr=0):
    ''' Like self.win.addnstr() but with sanitization '''
    self.win.addnstr(y, x, self.sanitize(str), n, attr)

  def addstr(self, y, x, str, attr=0):
    ''' Like self.win.addstr() but with sanitization '''
    self.win.addstr(y, x, self.sanitize(str), attr)

  def __getattr__(self, name):
    if name == 'win':
      return AttributeError()
    return getattr(self.win, name)

  def __setattr__(self, name, value):
    if name == 'win':
      super().__setattr__(name, value)
    else:
      setattr(self.win, name, value)

  def __delattr__(self, name):
    if name == 'win':
      super().__delattr__(name)
    else:
      return delattr(self.win, name)


class BaseWindowOrDialog:
  ''' Common utility class for a window or a dialog '''

  def __init__(self, parent, title=None):
    self.parent = parent
    self.title = title
    self.log = logging.getLogger('window')

  def border(self):
    ''' Redraw border (and title) '''
    self.win.border(0)
    if self.title is not None:
      self.win.addnstr(0, 2, self.title, self.width - 4)

  def updTitle(self, text):
    ''' Changes the window's displayed title '''
    self.title = text
    self.border()


class BaseWindow(BaseWindowOrDialog):
  ''' Common utility class for a screen subwindow '''

  def __init__(self, parent, height, width, top, left, title=None):
    super().__init__(parent, title)
    self.height = height
    self.width = width
    self.top = top
    self.left = left
    self.win = CursesWinProxy(self.height, self.width, self.top, self.left, self.parent)
    self.border()

  def refresh(self):
    self.win.refresh()

  def noutrefresh(self):
    self.win.noutrefresh()


class MapWindow(BaseWindow):
  ''' The mini map '''

  TOP = 0
  LEFT = 0
  MAP_SIZE = 192
  MAP_WATER_SIZE = 28

  def __init__(self, parent):
    ph, pw = parent.getmaxyx()
    self.width = pw
    self.height = ph - LogWindow.HEIGHT - StatusWindow.HEIGHT +1
    super().__init__(parent, self.height, self.width, self.TOP, self.LEFT)
    self.log.debug(f"Screensize {pw} {ph}")
    # Calculate center
    self.cx = int((self.width - 1) / 2)
    self.cy = int((self.height - 1) / 2)
    # Will store current position and other mobiles's positions
    self.pos = None
    self.opos = {}

  def updPosition(self, x, y, z, facing, notoriety):
    newpos = {'x':x, 'y':y, 'z':z, 'facing':facing, 'noto':notoriety}
    if self.pos == newpos:
      return
    self.pos = newpos

    self.updTitle("{},{},{}".format(x,y,z))
    color = self.notorietyColor(notoriety)
    self.win.addch(self.cy, self.cx, self.facingAsArrow(facing), color)

  def removeObject(self, serial):
    if serial in self.opos.keys():
      oldpos = self.opos[serial]
      del self.opos[serial]
      self.win.addch(oldpos['mapy'], oldpos['mapx'], ' ')

  def drawVisualRange(self, visrange):
    minx,maxx = max(0,self.cx-visrange),min(self.width-1,self.cx+visrange)
    miny,maxy = max(0,self.cy-visrange),min(self.height-1,self.cy+visrange)
    self.win.addch(miny,minx,'┌')
    self.win.addch(miny,maxx,'┐')
    self.win.addch(maxy,maxx,'┘')
    self.win.addch(maxy,minx,'└')
    for x in range(minx+1,maxx):
      self.win.addch(miny,x,'┈')
      self.win.addch(maxy,x,'┈')
    for y in range(miny+1,maxy):
      self.win.addch(y,minx,'┊')
      self.win.addch(y,maxx,'┊')

  def drawMap(self):
    for x in range(-self.cx+1,self.cx):
      for y in range(-self.cy+1,self.cy):
        mx=x+self.pos['x']
        my=y+self.pos['y']
        if x==0 and y ==0:
          continue
        if (mx<self.MAP_WATER_SIZE or my<self.MAP_WATER_SIZE
         or mx>self.MAP_SIZE-self.MAP_WATER_SIZE 
         or my>self.MAP_SIZE-self.MAP_WATER_SIZE):
          self.win.addch(y+self.cy,x+self.cx,'☱')

  def updObjects(self, objs, visrange):
    ''' Redraw all objs from the given list '''
    for x in range(1,self.width-1):
      for y in range(1,self.height-1):
        if x==self.cx and y== self.cy:
          continue
        self.win.addch(y,x,' ')

    self.drawMap()
    self.drawVisualRange(visrange)
    for obj in objs:
      relx = obj.x - self.pos['x']
      rely = obj.y - self.pos['y']
      mapx = self.cx + relx
      mapy = self.cy + rely

      if mapx >= 1 and mapx <= self.width-1 and mapy >= 1 and mapy <= self.height-1:
        # This is not out of screen, draw it
        opos = {'x':obj.x, 'y':obj.y, 'z':obj.z, 'facing':obj.facing,
            'mapx':mapx, 'mapy':mapy}
        self.opos[obj.serial] = opos
        if isinstance(obj,client.Mobile):
          fa = self.facingAsArrow(obj.facing)
          color = self.notorietyColor(obj.notoriety)
          self.win.addch(mapy, mapx, fa, color)
        elif isinstance(obj,client.Item):
          if obj.ismulti:
            self.win.addch(mapy,mapx,'★',Ui.colors['red'])
          else:
            self.win.addch(mapy,mapx,'◆',Ui.colors['yellow'])
      else:
        self.log.info('Mobile out of map at {},{}'.format(relx, rely))

  def facingAsArrow(self, facing):
    if facing == 0:
      return '↑'
    if facing == 1:
      return '↗'
    if facing == 2:
      return '→'
    if facing == 3:
      return '↘'
    if facing == 4:
      return '↓'
    if facing == 5:
      return '↙'
    if facing == 6:
      return '←'
    if facing == 7:
      return '↖'
    return '?'

  def notorietyColor(self, notoriety):
    if notoriety == client.Mobile.NOTO_INNOCENT:
      return Ui.colors['blue']
    if notoriety == client.Mobile.NOTO_FRIEND:
      return Ui.colors['green']
    if notoriety == client.Mobile.NOTO_ANIMAL:
      return Ui.colors['yellow']
    if notoriety == client.Mobile.NOTO_CRIMINAL:
      return Ui.colors['white']
    if notoriety == client.Mobile.NOTO_ENEMY:
      return Ui.colors['magenta']
    if notoriety == client.Mobile.NOTO_MURDERER:
      return Ui.colors['red']
    if notoriety == client.Mobile.NOTO_INVUL:
      return Ui.colors['cyan']
    return 0


class StatusWindow(BaseWindow):
  ''' The status window: wraps a ncurses window '''

  # Sizes and position
  HEIGHT = 5
  LEFT = 0
  LABELS = collections.OrderedDict([
    ('status', { 'x': 1, 'y': 1, 'l': 'status' }),
    ('vitals', { 'x': 1, 'y': 2, 'l': 'vitals' }),
    ('aspect', { 'x': 1, 'y': 3, 'l': 'aspect' }),
    ('misc',   { 'x': 1, 'y': 4, 'l': 'misc' }),
  ])

  def __init__(self, parent):
    ph, pw = parent.getmaxyx()
    self.width = pw
    self.top = ph - self.HEIGHT-LogWindow.HEIGHT
    super().__init__(parent, self.HEIGHT, self.width, self.top, self.LEFT)
    for k, label in self.LABELS.items():
      self.win.addstr(label['y'], label['x'], label['l'].upper()+':')

  def updLabel(self, name, text):
    ''' Updates a label '''
    try:
      label = self.LABELS[name]
    except KeyError:
      raise ValueError('Invalid label "{}". Valid labels are: {}'.format(
          name, ','.join(self.LABELS.keys())))
    x = len(label['l']) + 2
    maxLen = self.width - x - 2
    if len(text) > maxLen:
      text = text[:maxLen-1] + '…'
    self.win.addnstr(label['y'], label['x'] + x, ' ' * maxLen, maxLen)
    self.win.addnstr(label['y'], label['x'] + x, text, maxLen)


class LogWindow(BaseWindow):
  ''' The log window '''

  HEIGHT = 3
  LEFT = 0

  def __init__(self, parent):
    ph, pw = parent.getmaxyx()
    self.top = ph - self.HEIGHT
    self.width = pw
    super().__init__(parent, self.HEIGHT, self.width, self.top, self.LEFT)
    self.lines = collections.deque([], self.height-2)

  def append(self, text):
    ''' Appends a log line '''
    self.lines.append(text)
    self.win.clear()
    self.border()
    for idx, l in enumerate(self.lines):
      self.win.addnstr(idx+1, 2, l, self.width-2)
    self.win.refresh()


class BaseDialog(BaseWindowOrDialog):
  ''' Common utility class for a dialog '''

  def __init__(self, parent, title):
    super().__init__(parent, title)

  def draw(self, width, height):
    ''' Draw the base window centered '''
    ph, pw = self.parent.getmaxyx()
    height = min(ph,height)
    width = min(pw,width)
    self.top = int( (ph-height) / 2 )
    self.left = int( (pw-width) / 2 )
    self.width = width
    self.height = height

    self.win = CursesWinProxy(self.height, self.width, self.top, self.left)
    self.panel = self.win.panel()
    self.border()

  def undraw(self):
    self.win.clear()
    self.panel.hide()
    self.win.refresh()


class InputDialog(BaseDialog):
  ''' Shows an input dialog centered on the parent '''

  def __init__(self, parent, title, maxLen):
    super().__init__(parent, title)
    self.maxLen = maxLen

  def edit(self):
    width = self.maxLen + 2
    height = 3

    self.draw(width, height)
    self.win.refresh()

    ewin = CursesWinProxy(1, self.maxLen, self.top+1, self.left+1)
    ewin.refresh()

    box = ewin.textbox()

    # Let the user edit until Ctrl-G is struck.
    curses.curs_set(2)
    box.edit(lambda key: self.onKey(key))
    curses.curs_set(0)

    res = box.gather().strip()

    ewin.clear()
    self.undraw()

    return res

  def onKey(self, key):
    if key == ord('\n'):
      return 7 #Ctrl+G
    return key


class SelectDialog(BaseDialog):
  ''' Allows the user to select an element from a list '''

  def __init__(self, parent, title, elems, minWidth=None, minHeight=None):
    super().__init__(parent, title)
    self.elems = elems
    self.minWidth = minWidth
    self.minHeight = minHeight

  def select(self):
    ''' Make the user perform the selection, return selected index '''
    maxLen = max([len(i) for i in self.elems] + [self.minWidth])
    height = max(len(self.elems), self.minHeight)
    self.draw(maxLen+2, height+2)
    self.win.keypad(1)

    self.curidx = 0
    self.drawList()

    curses.flushinp()
    while True:
      key = self.win.getch()
      if key in (curses.KEY_ENTER, ord('\n')):
        break
      elif key == curses.KEY_UP:
        if self.curidx > 0:
          self.curidx -= 1
          self.drawList()
      elif key == curses.KEY_DOWN:
        if self.curidx < len(self.elems) - 1:
          self.curidx += 1
          self.drawList()

    self.undraw()
    return self.curidx

  def drawList(self):
    for idx, elem in enumerate(self.elems):
      attr = curses.A_REVERSE if idx == self.curidx else 0
      self.win.addnstr(idx+1, 1, elem, self.width-2, attr)
    self.win.refresh()


class UiLogHandler(logging.Handler):
  ''' Custom logging handler '''

  def __init__(self, lwin, level):
    super().__init__()
    self.lwin = lwin
    self.level = level

  def emit(self, record):
    if record.levelno < self.level:
      return

    lines = self.format(record).splitlines()
    for line in lines:
      self.lwin.append(line)

  def getLevel(self):
    ''' Gets current log level '''
    return self.level

  def setLevel(self, level):
    ''' Sets new log level '''
    self.level = level


if __name__ == '__main__':
  import argparse
  import traceback

  # Parse command line
  parser = argparse.ArgumentParser()
  parser.add_argument('host', help="Server's IP address or host name")
  parser.add_argument('port', type=int, help="Server's port")
  parser.add_argument('-l', '--log', action='store_true',
      help="Write a verbose log")
  parser.add_argument('-v', '--verbose', action='store_true',
      help="Increase initial verbosity")
  parser.add_argument('-q', '--quiet', action='store_true',
      help="Decrease initial verbosity")
  args = parser.parse_args()

  if args.verbose:
    logLevel = logging.INFO
  elif args.quiet:
    logLevel = logging.ERROR
  else:
    logLevel = logging.WARNING

  # Redirect stderr so it doesn't interfere with curses
  if args.log:
    errFile = os.path.join(
      os.path.dirname(os.path.abspath(__file__)),
      os.path.splitext(os.path.basename(os.path.abspath(__file__)))[0]+'.err'
    )
    sys.stderr = open(errFile, 'wt')

  try:
    curses.wrapper(Ui, args.host, args.port, args.log, logLevel)
  except Exception as e:
    type, value, tb = sys.exc_info()
    msg = ''.join(traceback.format_exception(type, value, tb))
    logging.critical(msg)
    raise
  finally:
    curses.endwin()
