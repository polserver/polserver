#!/usr/bin/env python3

import atexit
import configparser
import logging
import json
import time
import os
import sys
import socket
import threading
import collections

from pyuo import client
from pyuo import brain
from pyuo.brain import Event

class TestBrain(brain.Brain):
  def __init__(self,client,server):
    self.server = server
    self.id = client.id
    self.server.addBrain(self)
    self.todos = collections.deque()
    self.todosLock = threading.Lock()
    super(TestBrain,self).__init__( client, self.id )

  def init(self):
    self.setTimeout(0.2)
    self.server.addevent(brain.Event(brain.Event.EVT_INIT, clientid=self.id))

  def loop(self):
    try:
      if not self.processTodos():
        return True
    except Exception as e:
      self.log.exception(e)
      return True

  def onEvent(self, ev):
    '''overwrite brain method to directly send the events to the server'''
    if ev.type == Event.EVT_CLIENT_CRASH:
      self.log.critical('Oops! Client crashed: {}'.format(ev.exception))
      raise RuntimeError('Oops! Client crashed')
    ev.clientid=self.id
    self.server.addevent(ev)
  
  def addTodo(self,ev):
    with self.todosLock:
      self.todos.append(ev)

  def hasWork(self):
    '''the brain is driven entirely from the test script, so anything queued
    here is the only reason its main loop ever has to wake up early'''
    with self.todosLock:
      return len(self.todos) > 0

  def processTodos(self):
    with self.todosLock:
      if not len(self.todos):
        return True
      todos = self.todos.copy()
      self.todos.clear()
    while len(todos):
      res = todos.popleft()
      todo=res["todo"]
      arg=res.get("arg",None)
      self.log.info("got todo: {}->{}".format(todo,arg))
      if todo=="disconnect":
        self.client.addTodo(brain.Event(brain.Event.EVT_EXIT))
        return False
      elif todo=="speech":
        if isinstance(arg, str):
          self.client.say(arg)
        else:
          self.client.say(arg['text'], tokens = arg.get('tokens', None),
            type = arg.get('type', None))
      elif todo=="move":
        self.client.move(arg)
      elif todo=="list_objects":
        self.client.addTodo(brain.Event(brain.Event.EVT_LIST_OBJS, parent = arg))
      elif todo=="list_equipped_items":
        self.client.addTodo(brain.Event(brain.Event.EVT_LIST_EQUIPPED_ITEMS, serial = arg))
      elif todo=="open_backpack":
        bp=self.client.player.openBackPack()
        content=0
        if bp is not None:
          content=len(bp.content)
          bp=bp.serial
        self.server.addevent(
          brain.Event(brain.Event.EVT_OPEN_BACKPACK,
            clientid = self.id,
            serial = bp,
            contentlen = content))
      elif todo=="attack":
        self.client.attack(arg)
      elif todo=="war_mode":
        self.client.warMode(arg)
      elif todo=="request_status":
        # a bare serial asks for the basic status, a dict can pick the request
        # type as well (0x04 basic, 0x05 skills)
        if isinstance(arg, dict):
          self.client.requestStatus(arg.get('serial', None), arg.get('type', None))
        else:
          self.client.requestStatus(arg)
      elif todo=="single_click":
        # answered with the name text of what was clicked, in the colour the
        # server picked for it - so there is no event of its own to raise here
        self.client.singleClick(int(arg))
      elif todo=="double_click":
        self.client.doubleClick(arg)
        self.server.addevent(
          brain.Event(brain.Event.EVT_DOUBLE_CLICK,
            clientid = self.id,
            serial = arg))
      elif todo=="lift_item":
        if isinstance(arg, int):
          serial = arg
          amount = 1
        else:
          serial = arg['serial']
          amount = arg['amount']

        self.client.lift(serial, amount)
        self.server.addevent(
          brain.Event(brain.Event.EVT_LIFT_ITEM,
            clientid = self.id,
            serial = serial,
            amount = amount))
      elif todo=="boat_move":
        self.client.boat_move(arg['serial'], arg['direction'], arg['speed'])
        self.server.addevent(
          brain.Event(brain.Event.EVT_BOAT_MOVE,
            clientid = self.id,
            serial = arg['serial'],
            direction = arg['direction'],
            speed = arg['speed'],
            ))
      elif todo=="drop_item":
        self.client.drop(arg['serial'], arg['x'], arg['y'], arg['z'], arg['dropped_on_serial'])
        self.server.addevent(
          brain.Event(brain.Event.EVT_DROP_ITEM,
            clientid = self.id,
            serial = arg['serial']
            ))
      elif todo=="wear_item":
        self.client.wear(arg['item_serial'], arg['layer'], arg['player_serial'])
        self.server.addevent(
          brain.Event(brain.Event.EVT_WEAR_ITEM,
            clientid = self.id,
            item_serial = arg['item_serial'],
            layer = arg['layer'],
            player_serial = arg['player_serial']
            ))
      elif todo=="trade":
        if isinstance(arg, int):
          self.client.secureTrade(arg)
        else:
          self.client.secureTrade(arg['action'], arg.get('flag', 0))
      elif todo=="gump_reply":
        # With a gumpid, answer that gump right now - the server is free never to have
        # sent it. Without one, arm the answer the next gump to arrive gets, which is how
        # a suspending SendDialogGump is answered with more than a button.
        if 'gumpid' in arg:
          self.client.gumpReply(int(arg['gumpid']), int(arg.get('button', 0)),
            serial = arg.get('serial', None),
            switches = arg.get('switches', None), texts = arg.get('texts', None),
            short = bool(arg.get('short', 0)),
            claim_switches = arg.get('claim_switches', None),
            claim_texts = arg.get('claim_texts', None))
        else:
          self.client.next_gump_reply = arg
        # Acked, because arming has to be known to have happened before the gump is asked for:
        # the todo travels the test connection while the gump comes down the game socket, and
        # the two are read by different threads.
        self.server.addevent(
          brain.Event(brain.Event.EVT_GUMP_REPLY,
            clientid = self.id
            ))
      elif todo=="popup_select":
        # answers a menu right now, which the server is free never to have shown
        self.client.popupSelect(int(arg['serial']), int(arg['tag']))
      elif todo=="book":
        # read a page, write one, or write the title and author back
        action = arg.get('action', 'read')
        if action == 'title':
          self.client.bookTitle(int(arg['serial']), arg.get('title', ''), arg.get('author', ''))
        else:
          self.client.bookPage(int(arg['serial']), int(arg['page']),
            arg['lines'] if action == 'write' else None)
      elif todo=="dialog_reply":
        # Arms the answer the next dialog of that kind gets: "textentry", "color" or
        # "resurrect". Acked for the same reason the gump reply is.
        self.client.next_dialog_reply[arg['kind']] = arg
        self.server.addevent(
          brain.Event(brain.Event.EVT_DIALOG_REPLY,
            clientid = self.id
            ))
      elif todo=="race_change":
        self.client.raceChange(arg['bodyhue'], arg['hairid'], arg['hairhue'],
                               arg['beardid'], arg['beardhue'])
        self.server.addevent(
          brain.Event(brain.Event.EVT_RACE_CHANGE,
            clientid = self.id
            ))
      elif todo=="buy_items":
        # arg['items'] is a list of {serial, amount, layer}; layer is what the client echoes back
        # from the vendor window and the core ignores it, so it defaults to 0.
        items = [(i.get('layer', 0), i['serial'], i['amount']) for i in arg['items']]
        self.client.buy(arg['vendor_serial'], items)
        self.server.addevent(
          brain.Event(brain.Event.EVT_BUY_ITEMS,
            clientid = self.id,
            vendor_serial = arg['vendor_serial']
            ))
      elif todo=="sell_items":
        items = [(i['serial'], i['amount']) for i in arg['items']]
        self.client.sell(arg['vendor_serial'], items)
        self.server.addevent(
          brain.Event(brain.Event.EVT_SELL_ITEMS,
            clientid = self.id,
            vendor_serial = arg['vendor_serial']
            ))
      elif todo=="party":
        # the arguments a party command takes, in the order the packet wants
        # them: a serial, then a text, or a loot flag on its own
        args=[]
        if arg.get('serial', None) is not None:
          args.append(int(arg['serial']))
        if arg.get('text', None) is not None:
          args.append(arg['text'])
        if arg.get('canloot', None) is not None:
          args.append(int(arg['canloot']))
        self.client.party(int(arg['partycmd']), *args)
      elif todo=="house":
        # the arguments a house command takes, in the order the packet wants
        # them: a graphic, an offset, then a z, or a floor number on its own
        args=[]
        if arg.get('graphic', None) is not None:
          args.append(int(arg['graphic']))
          args.append(int(arg['x']))
          args.append(int(arg['y']))
        if arg.get('z', None) is not None:
          args.append(int(arg['z']))
        if arg.get('floor', None) is not None:
          args.append(int(arg['floor']))
        serial=arg.get('serial', None)
        self.client.houseCommand(int(arg['sub']), *args,
          serial = None if serial is None else int(serial))
      elif todo=="cast":
        # a book serial picks the "cast out of this book" form of the text
        # command, and select the spellbook gump's route instead of either
        bookserial=arg.get('bookserial', None)
        self.client.castSpell(int(arg['spellid']),
          bookserial = None if bookserial is None else int(bookserial),
          select = bool(arg.get('select', 0)))
      elif todo=="spellbook":
        self.client.openSpellbook()
      elif todo=="map_pin":
        self.client.mapPin(int(arg['serial']), int(arg['action']),
          int(arg.get('pinidx', 0)), int(arg.get('x', 0)), int(arg.get('y', 0)))
      elif todo=="client_version":
        self.client.sendVersion(arg)
      elif todo=="target":
        res=self.client.waitForTarget(5)
        targettype=None
        if res is not None:
          targettype=res.type
          if res.what==client.Target.OBJECT:
            obj=self.client.waitForObject(arg['serial'],5)
            if obj is None:
              self.log.error("asked to target object 0x{:X}, which this client "
                             "was never told about".format(arg['serial']))
            else:
              res.target(obj)
          else:
            res.targetLocation(arg['x'],arg['y'],arg['z'],arg['graphic'])
        self.server.addevent(
          brain.Event(brain.Event.EVT_TARGET,
            clientid = self.id,
            targettype = targettype,
            res = res is not None))
      elif todo=="cancel_target":
        res=self.client.waitForTarget(5)
        if res is not None:
          res.cancel()
        self.server.addevent(
          brain.Event(brain.Event.EVT_CANCEL_TARGET,
            clientid = self.id,
            res = res is not None))
      elif todo=="disable_item_logging":
        self.client.addTodo(brain.Event(brain.Event.EVT_DISABLE_ITEM_LOGGING, value = arg))
      elif todo=="aos_tooltip":
        self.client.getAOSTooltip(arg[0],arg[1])
      elif todo=="auto_delete_objs":
        self.client.auto_delete_objs = arg
        self.server.addevent(
          brain.Event(brain.Event.EVT_AUTO_DELETE_OBJS,
            clientid = self.id,
            state = self.client.auto_delete_objs))

    return True

class ShardGone(Exception):
  '''The shard ran and stopped without ever asking for a client.'''


# Backstop if the shard never appears at all; ctest allows 600s.
HARD_DEADLINE_SECS = 540


def game_port_free(port):
  '''True once nothing is listening on the game port, i.e. the shard is gone.

  The bind has to use the wildcard address: POL's game listener binds INADDR_ANY, and
  Windows lets a socket take a specific address while another holds the wildcard on the
  same port, so a 127.0.0.1 probe reports "free" the whole time the shard is up. Bind
  rather than connect, so the probe never makes the core build a client for it.
  '''
  probe = socket.socket()
  try:
    probe.bind(('0.0.0.0', port))
    return True
  except OSError:
    return False
  finally:
    probe.close()


class PolServer:
  def __init__(self):
    self.log = logging.getLogger('server')
    conf = configparser.ConfigParser()
    path=os.path.dirname(os.path.abspath(__file__))
    conf.read(os.path.join(path,'testclient.cfg'))
    self.lconf = conf['login']
    self.clients=[]
    self.threads=[]
    self.brains=[]
    self.events = collections.deque()
    self.eventsLock = threading.Lock()
    self.clientLock = threading.Lock()
    self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self.s.bind(('localhost', 50000))
    self.s.listen(1)
    # Poll rather than wait out one long timeout: no test package here connects until the
    # shard runs the one that drives this client, which can be minutes into a full run, so
    # the wait cannot be bounded by a constant. What ends it is the shard going away -- the
    # same condition deafclient.py and rawpeer.py stop on. Without it a run that selects no
    # client test (POLCORE_TEST_FILTER) pays this timeout in full, because cmake's
    # execute_process waits on every stage of the pipeline, not just POL.
    self.s.settimeout(1.0)
    self.conn = self._accept(self.lconf.getint('port'))
    # How long run() sits in recv() before it gets to flush the brains' events
    # back to the script. Events are queued by the brain threads and only go
    # out between two reads, so this is a floor on how fast the script can be
    # told anything - it is a poll interval, not a deadline, and the read
    # returns the moment a byte arrives either way.
    self.conn.settimeout(0.02)
    self.buf=b''

  def _accept(self, gameport):
    started = time.monotonic()
    shard_seen = False
    while time.monotonic() - started < HARD_DEADLINE_SECS:
      try:
        conn, _ = self.s.accept()
        return conn
      except socket.timeout:
        if not game_port_free(gameport):
          shard_seen = True
        elif shard_seen:
          raise ShardGone('shard stopped without connecting a test client')
    raise ShardGone('shard never appeared within {}s'.format(HARD_DEADLINE_SECS))

  def run(self):
    while True:
      self.brainevents()
      res=self.recv()
      if res is None:
        return True
      if not res.get("todo",None):
        continue
      clientid=res.get("id",None)
      todo=res["todo"]
      arg=res.get("arg",None)
      if todo=="connect":
        self.threads.append(
          threading.Thread(target=self.startclient,
              args=(res["account"],res["psw"],res["name"],res["chrindex"], res["id"]),
              daemon=True)
          )
        self.threads[-1].start()
      elif todo=="createchar":
        # everything past the account is handed to the create packet, so a test can pin any of
        # the values the server validates without another todo per field
        opts = {k:v for k,v in res.items()
                if k not in ("todo","account","psw","name","chrindex","id")}
        self.threads.append(
          threading.Thread(target=self.startclient,
              args=(res["account"],res["psw"],res["name"],res["chrindex"], res["id"]),
              kwargs={"create":opts}, daemon=True)
          )
        self.threads[-1].start()
      elif todo=="exit":
        # Logged per step: this process is one stage of cmake's execute_process pipeline, so if a
        # client thread never ends, the join below holds up the whole job and the only symptom is
        # a 600s timeout with nothing said. The lines below name which thread that was.
        self.log.info("LIFECYCLE exit requested, telling %d brains to disconnect",
                      len(self.brains))
        with self.clientLock:
          for b in self.brains:
            b.addTodo({"todo":"disconnect"})
        for i, t in enumerate(self.threads):
          self.log.info("LIFECYCLE joining client thread %d/%d (%s)",
                        i + 1, len(self.threads), t.name)
          t.join(timeout=30)
          if t.is_alive():
            # Bounded on purpose: a client that cannot finish is worth a loud line in the job
            # output, not a silent timeout on the whole pipeline. The threads are daemons, so
            # leaving this one behind does not stop the process from exiting.
            self.log.error("LIFECYCLE client thread %d/%d (%s) did not end within 30s, "
                           "leaving it behind", i + 1, len(self.threads), t.name)
          else:
            self.log.info("LIFECYCLE joined client thread %d/%d", i + 1, len(self.threads))
        self.log.info("LIFECYCLE all client threads joined")
        self.sendEvent(brain.Event(brain.Event.EVT_EXIT,clientid=0))
        self.log.info("LIFECYCLE run() returning")
        return
      else:
        with self.clientLock:
          for b in self.brains:
            if b.id == clientid:
              b.addTodo(res)
              break
          else:
            self.log.error("invalid clientid")

  def startclient(self,user,psw,charname,charidx,id,create=None):
    with self.clientLock:
      c = client.Client(id)
      self.clients.append(c)
    servers = c.connect(self.lconf.get('ip'), self.lconf.getint('port'), user, psw)
    chars = c.selectServer(self.lconf.getint('serveridx'))
    if create is None:
      c.selectCharacter(charname, charidx)
    else:
      c.createCharacter(charname, charidx, **create)
    TestBrain(c,self)

  def addBrain(self, brain):
    with self.clientLock:
      self.brains.append(brain)

  def addevent(self,ev):
    with self.eventsLock:
      self.events.append(ev)

  def brainevents(self):
    with self.eventsLock:
      if not len(self.events):
        return
      events=self.events.copy()
      self.events.clear()
    while len(events):
      ev = events.popleft()
      self.sendEvent(ev)

  def _recv(self):
    try:
      data = self.conn.recv(1)
    except socket.timeout:
        return b''
    except Exception as e:
      self.log.info("err {}".format(e))
      self.conn.close()
      return None
    return data
    
  def recv(self):
    while b'\r\n' not in self.buf:
      r=self._recv()
      if r is None:
        return None
      self.buf+=r
      if not len(self.buf):
          return {}
    try:
      data=json.loads(self.buf[:self.buf.index(b'\r\n')].decode())
    except Exception as e:
      self.log.error('failed to receive: {} data: "{}" buffer: "{}"'.format(
        e,
        self.buf[:self.buf.index(b'\r\n')].decode(),
        self.buf.decode()))
      raise e
    self.buf=self.buf[self.buf.index(b'\r\n')+2:]
    return data

  def sendEvent(self, ev):
    '''serialization method for client events'''
    res={}
    res["id"]=ev.clientid
    res["type"]=ev.typestr()
    if ev.type==Event.EVT_INIT or ev.type==Event.EVT_EXIT:
      pass
    elif (ev.type==Event.EVT_HP_CHANGED or
        ev.type==Event.EVT_MANA_CHANGED or
        ev.type==Event.EVT_STAM_CHANGED or
        ev.type==Event.EVT_NOTORIETY):
      res["new"]=ev.new
      if hasattr(ev,"old"):
        res["old"]=ev.old
      if hasattr(ev,"serial"):
        res["serial"]=ev.serial
    elif ev.type==Event.EVT_SPEECH or ev.type==Event.EVT_CLILOC:
      # everything the speech packets carry: which of them it was decides
      # what the core chose, and the rest is what the sending function set
      res["msg"]=ev.speech.msg
      res["serial"]=ev.speech.serial
      res["model"]=ev.speech.model
      res["texttype"]=ev.speech.type
      res["color"]=ev.speech.color
      res["font"]=ev.speech.font
      res["name"]=ev.speech.name
      res["lang"]=ev.speech.lang if ev.speech.lang else ""
      res["unicode"]=1 if ev.speech.unicode else 0
      if ev.type==Event.EVT_CLILOC:
        # a cliloc message has no text of its own: the number is the message
        # and "msg" carries the arguments filling its placeholders
        res["cliloc"]=ev.speech.cliloc
        # the affix variant adds a plain string of its own, a name as often as
        # not, and says whether it goes before the text or after it
        if ev.speech.affix is not None:
          res["affix"]=ev.speech.affix
          res["prepend"]=1 if ev.speech.prepend else 0
    elif ev.type==Event.EVT_MOVED:
      res["ack"]=ev.ack
      res["pos"]=[ev.x, ev.y, ev.z, ev.facing]
    elif (ev.type==Event.EVT_NEW_MOBILE or
          ev.type==Event.EVT_NEW_ITEM):
      obj = ev.mobile if ev.type==Event.EVT_NEW_MOBILE else ev.item
      res["serial"]=obj.serial
      res["pos"]=ev.pos
      res["graphic"]=obj.graphic
      res["status"]=obj.status
      res["playerpos"]=ev.playerpos
      # how the server told this client to colour that mobile: a guild ally is
      # drawn as a friend, a guild enemy as an enemy
      if getattr(obj, "notoriety", None) is not None:
        res["notoriety"]=obj.notoriety
    elif ev.type==Event.EVT_REMOVED_OBJ:
      res["serial"]=ev.serial
      res["oldpos"]=ev.oldpos
    elif ev.type==Event.EVT_OUT_OF_RANGE_OBJ:
      res["serial"]=ev.serial
      res["pos"]=ev.pos
      res["playerpos"]=ev.playerpos
    elif ev.type==Event.EVT_OBJ_REVISION:
      res["serial"]=ev.serial
      res["revision"]=ev.revision
    elif ev.type==Event.EVT_EFFECT:
      # "cmd" tells the plain effect packet from the extended one, "kind" is the
      # effect type: 0 moving, 1 lightning, 2 fixed at a place, 3 following an
      # object. The two positions are the endpoints the client draws between.
      res["cmd"]=ev.cmd
      res["kind"]=ev.kind
      res["serial"]=ev.serial
      res["target"]=ev.target
      res["graphic"]=ev.graphic
      res["pos"]=[ev.x, ev.y, ev.z]
      res["tpos"]=[ev.tx, ev.ty, ev.tz]
    elif ev.type==Event.EVT_LIST_OBJS:
      res["objs"]=[]
      for _,o in ev.objs.items():
        res["objs"].append(
              {'serial':o.serial,
               'pos':[o.x,o.y,o.z,o.facing],
               'graphic':o.graphic}
        )
        if hasattr(o,"attackable"):
          res["objs"][-1]["attackable"]=o.attackable
        if getattr(o,"notoriety",None) is not None:
          res["objs"][-1]["notoriety"]=o.notoriety
        # mobiles carry no amount; for an item it is the stack size, except in
        # a spellbook drawn the pre-AOS way, where it is the spell number
        if getattr(o,"amount",None) is not None:
          res["objs"][-1]["amount"]=o.amount
        if hasattr(o,"parent") and o.parent is not None:
          res["objs"][-1]["parent"]=o.parent.serial
    elif ev.type==Event.EVT_LIST_EQUIPPED_ITEMS:
      res["objs"]=[]
      if ev.owner is not None and hasattr(ev.owner, 'equip') and ev.owner.equip is not None:
        for k,o in ev.owner.equip.items():
          res["objs"].append(
                {'serial':o, 'layer':k} if isinstance(o, int) else
                {'serial':o.serial,
                 'color':o.color,
                 'layer':k,
                 'graphic':o.graphic}
          )
    elif ev.type==Event.EVT_OPEN_BACKPACK:
      res["serial"]=ev.serial
      res["content_count"]=ev.contentlen
    elif ev.type==Event.EVT_TARGET:
      res["res"]=ev.res
      res["targettype"]=ev.targettype
    elif ev.type==Event.EVT_NEW_SUBSERVER:
      pass
    elif ev.type==Event.EVT_DISABLE_ITEM_LOGGING:
      pass
    elif ev.type==Event.EVT_BOAT_MOVED:
      res['serial']=ev.boat.serial
      res["pos"]=[ev.boat.x, ev.boat.y, ev.boat.z]
    elif ev.type==Event.EVT_OWNCREATE:
      pass
    elif ev.type==Event.EVT_DOUBLE_CLICK:
      res['serial']=ev.serial
    elif ev.type==Event.EVT_LIFT_ITEM:
      res['serial']=ev.serial
      res['amount']=1
    elif ev.type==Event.EVT_MOVE_ITEM_REJECTED:
      res['reason']=ev.reason
    elif ev.type==Event.EVT_BOAT_MOVE:
      res['serial']=ev.serial
      res['direction']=ev.direction
      res['speed']=ev.speed
    elif ev.type==Event.EVT_DROP_ITEM:
      res['serial']=ev.serial
    elif ev.type==Event.EVT_WEAR_ITEM:
      res['item_serial']=ev.item_serial
      res['layer']=ev.layer
      res['player_serial']=ev.player_serial
    elif ev.type==Event.EVT_CANCEL_TARGET:
      res["res"]=ev.res
    elif ev.type==Event.EVT_DROP_APPROVED:
      pass
    elif ev.type==Event.EVT_GUMP:
      if hasattr(ev,"gumpid"):
        res['gumpid']=ev.gumpid
        res['buttonid']=ev.buttonid
      else:
        res['commands']=ev.commands
        res['texts']=ev.texts
    elif ev.type==Event.EVT_GUMP_REPLY or ev.type==Event.EVT_DIALOG_REPLY:
      pass
    elif ev.type==Event.EVT_VENDOR_SELL_LIST:
      res['serial']=ev.serial
      res['items']=ev.items
    elif ev.type==Event.EVT_POPUP:
      res['serial']=ev.serial
      res['format']=ev.format
      res['entries']=ev.entries
    elif ev.type==Event.EVT_BOOK:
      res['serial']=ev.serial
      res['title']=ev.title
      res['author']=ev.author
      res['writable']=ev.writable
      res['npages']=ev.npages
    elif ev.type==Event.EVT_BOOK_PAGE:
      res['serial']=ev.serial
      res['pages']=ev.pages
      res['pagedata']=ev.pagedata
    elif ev.type==Event.EVT_TEXT_ENTRY:
      res['text']=ev.text
      res['text2']=ev.text2
      res['cancel']=ev.cancel
      res['style']=ev.style
      res['maximum']=ev.maximum
    elif ev.type==Event.EVT_SELECT_COLOR:
      res['serial']=ev.serial
      res['graphic']=ev.graphic
    elif ev.type==Event.EVT_RESURRECT_MENU:
      res['choice']=ev.choice
    elif ev.type==Event.EVT_RACE_CHANGER:
      res['gender']=ev.gender
      res['race']=ev.race
    elif ev.type==Event.EVT_CLOSE_WINDOW:
      res['windowtype']=ev.windowtype
      res['serial']=ev.serial
    elif ev.type==Event.EVT_AOS_TOOLTIP:
      res['text']=ev.text
    elif ev.type==Event.EVT_OPEN_PAPERDOLL:
      res['serial']=ev.serial
      res['text']=ev.text
      res['flags']=ev.flags
    elif ev.type==Event.EVT_AUTO_DELETE_OBJS:
      res['state']=ev.state
    elif ev.type==Event.EVT_ATTACK:
      res['serial']=ev.serial
    elif ev.type==Event.EVT_WAR_MODE:
      res['war']=ev.war
    elif ev.type==Event.EVT_FIGHT_OCCURING:
      res['attacker']=ev.attacker
      res['defender']=ev.defender
    elif ev.type==Event.EVT_TRADE:
      res['action']=ev.action
      res['serial']=ev.serial
      res['cont1']=ev.cont1
      res['cont2']=ev.cont2
      res['name']=ev.name
    elif ev.type==Event.EVT_PARTY:
      # what the subcommand carries: the member list for a list, the member that
      # left plus the ones remaining for a removal, the speaker and the text for
      # a message, the leader for an invitation
      res['partycmd']=ev.partycmd
      res['members']=ev.members
      if ev.serial is not None:
        res['serial']=ev.serial
      if ev.msg is not None:
        res['msg']=ev.msg
    elif ev.type==Event.EVT_STATUS_BAR:
      res['serial']=ev.serial
      res['name']=ev.name
      res['hp']=ev.hp
      res['maxhp']=ev.maxhp
    elif ev.type==Event.EVT_BUY_ITEMS or ev.type==Event.EVT_SELL_ITEMS:
      res['vendor_serial']=ev.vendor_serial
    elif ev.type==Event.EVT_RACE_CHANGE:
      pass
    elif ev.type==Event.EVT_HOUSE_DESIGN:
      # what the header claims and what the planes actually carried are both
      # here on purpose: a design that disagrees with itself is the kind of
      # thing only decoding the packet can catch
      res['serial']=ev.serial
      res['revision']=ev.revision
      res['numtiles']=ev.numtiles
      res['planecount']=ev.planecount
      res['planes']=[{k:v for k,v in p.items() if k != 'tiles'} for p in ev.planes]
      res['tiles']=ev.tiles
    elif ev.type==Event.EVT_HOUSE_EDIT:
      res['serial']=ev.serial
      res['action']=ev.action
      res['editing']=ev.editing
    elif ev.type==Event.EVT_HOUSE_REV:
      res['serial']=ev.serial
      res['revision']=ev.revision
    elif ev.type==Event.EVT_SPELLBOOK:
      res['serial']=ev.serial
      res['graphic']=ev.graphic
      res['firstspell']=ev.firstspell
      res['contents']=ev.contents
    elif ev.type==Event.EVT_MAP:
      res['serial']=ev.serial
      res['gumpart']=ev.gumpart
      res['xwest']=ev.xwest
      res['ynorth']=ev.ynorth
      res['xeast']=ev.xeast
      res['ysouth']=ev.ysouth
      res['gumpwidth']=ev.gumpwidth
      res['gumpheight']=ev.gumpheight
      # only the 0xf5 packet carries one, so this says which one arrived
      res['facetid']=ev.facetid
    elif ev.type==Event.EVT_MAP_PIN:
      res['serial']=ev.serial
      res['action']=ev.action
      res['pinidx']=ev.pinidx
      res['x']=ev.x
      res['y']=ev.y
    else:
      raise NotImplementedError("Unknown event {}",format(ev.type))

    self.send(json.dumps(res))

  def send(self, data):
    try:
      self.conn.send((data+"\n").encode())
    except Exception as e:
      self.log.error("failed to send: {} {}".format(e,data))
      pass

if __name__ == '__main__':
  logging.basicConfig(level=logging.INFO, stream=sys.stderr,
          format="      %(name)s:%(message)s")

  # Whatever else happens, say when this process actually goes. It is one stage of a pipeline that
  # cmake waits on in full, so a stage that never exits looks exactly like a hung shard.
  lifecycle = logging.getLogger('testclient')
  atexit.register(lambda: lifecycle.info("LIFECYCLE process exiting"))
  lifecycle.info("LIFECYCLE process starting (pid %d)", os.getpid())

  try:
    serv = PolServer()
  except ShardGone as ex:
    # Not a failure: nothing asked for a client. Exit quietly so the pipeline ends with POL.
    lifecycle.info("LIFECYCLE %s", ex)
    sys.exit(0)

  try:
    serv.run()
  finally: # wake up the server and let it close first
    lifecycle.info("LIFECYCLE run() left, releasing the control connection")
    serv.send("{}")
    time.sleep(1)
    serv.conn.close()
    lifecycle.info("LIFECYCLE control connection closed")

