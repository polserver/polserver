#!/usr/bin/env python3

import asyncio
import atexit
import configparser
import logging
import json
import time
import os
import sys
import socket
import collections
import traceback

from pyuo import client
from pyuo import brain
from pyuo import packets
from pyuo.brain import Event

class TestBrain(brain.Brain):
  def __init__(self,client,server):
    self.server = server
    self.id = client.id
    self.server.addBrain(self)
    self.todos = collections.deque()
    super(TestBrain,self).__init__( client, self.id )

  async def init(self):
    self.setTimeout(0.2)
    self.server.addevent(brain.Event(brain.Event.EVT_INIT, clientid=self.id))

  async def loop(self):
    try:
      if not await self.processTodos():
        return True
    except Exception as e:
      self.log.exception(e)
      # Take the client down with it, the way the disconnect todo does. A brain that
      # walks away leaves a client nothing drives and nobody reads, and the process
      # cannot finish until that client is joined.
      self.client.addTodo(brain.Event(brain.Event.EVT_EXIT))
      return True

  def onEvent(self, ev):
    '''overwrite brain method to directly send the events to the server'''
    if ev.type == Event.EVT_CLIENT_CRASH:
      self.log.critical('Oops! Client crashed: {}'.format(ev.exception))
      raise RuntimeError('Oops! Client crashed')
    ev.clientid=self.id
    self.server.addevent(ev)
  
  def addTodo(self,ev):
    self.todos.append(ev)
    # The brain's main loop blocks until something is queued for it. Without this
    # every order waits out the loop timeout before it is even looked at.
    self.wakeup.set()

  def hasWork(self):
    '''the brain is driven entirely from the test script, so anything queued
    here is the only reason its main loop ever has to wake up early'''
    return len(self.todos) > 0

  def sendClientPacket(self, arg):
    '''! Sends one packet the client knows how to build
    @param arg: what to send. "packet" names it, the rest are that packet's own
                fields. Everything here is a well formed packet: a deliberately
                wrong one goes out through the raw_packet todo instead.
    '''
    name = arg['packet']
    if name == "say_ascii":
      # a text carried as bytes is how a case says something no string would hold
      self.client.sayAscii(arg['raw'] if 'raw' in arg else arg['text'],
        int(arg.get('type', 0)), int(arg.get('color', 50)), int(arg.get('font', 3)))
    elif name == "bulletin_board":
      self.client.bulletinBoard(int(arg['sub']), arg.get('body', []))
    elif name == "rename":
      self.client.renameChar(int(arg['serial']), arg['name'])
    elif name == "all_names":
      self.client.requestAllNames(int(arg['serial']))
    elif name == "char_profile":
      self.client.charProfile(int(arg['serial']), arg.get('text', None))
    elif name == "chat_button":
      self.client.chatButton(arg.get('name', ''))
    elif name == "ultima_messenger":
      self.client.ultimaMessenger(int(arg['serial']), int(arg.get('serial2', 0)))
    elif name == "unknown_c4":
      self.client.unknownC4(int(arg['serial']), int(arg.get('intensity', 0)))
    elif name == "uo_store":
      self.client.openUOStore()
    elif name == "public_house":
      self.client.publicHouseContent(int(arg.get('flag', 1)))
    elif name == "client_type":
      self.client.announceClientType(int(arg['type']))
    elif name == "seed":
      self.client.sendSeed(arg.get('version', None))
    elif name == "worldmap":
      locations = arg.get('locations', None)
      self.client.worldmapQuery(int(arg['sub']),
        None if locations is None else int(locations))
    elif name == "tooltips":
      self.client.requestTooltips([int(s) for s in arg['serials']])
    elif name == "tooltip":
      self.client.requestTooltip(int(arg['serial']), arg.get('lang', 'ENU'))
    elif name == "resync":
      self.client.resyncRequest()
    elif name == "skill_lock":
      self.client.skillLock(int(arg['skill']), int(arg['mode']))
    elif name == "help":
      self.client.requestHelp()
    elif name == "get_tip":
      self.client.getTip(int(arg.get('lasttip', 0)), bool(arg.get('next', 1)))
    elif name == "popup_request":
      self.client.generalInfo(packets.GeneralInfoPacket.SUB_POPUP_REQUEST,
        int(arg['serial']))
    elif name == "object_cache":
      self.client.generalInfo(packets.GeneralInfoPacket.SUB_MEGACLILOC,
        int(arg['serial']), 0)
    elif name == "house_design":
      self.client.generalInfo(packets.GeneralInfoPacket.SUB_HOUSE_DESIGN,
        int(arg['serial']))
    elif name == "stat_lock":
      self.client.generalInfo(packets.GeneralInfoPacket.SUB_STATLOCK,
        int(arg['stat']), int(arg['mode']))
    elif name == "close_status":
      self.client.generalInfo(packets.GeneralInfoPacket.SUB_CLOSESTATUS,
        int(arg['serial']))
    elif name == "screen_size":
      self.client.generalInfo(packets.GeneralInfoPacket.SUB_SCREENSIZE,
        int(arg['width']), int(arg['height']))
    elif name == "se_spam":
      self.client.generalInfo(packets.GeneralInfoPacket.SUB_SESPAM, int(arg['flag']))
    elif name == "flying":
      self.client.generalInfo(packets.GeneralInfoPacket.SUB_TOGGLE_FLYING)
    elif name == "answer_target":
      # eScript ints are signed, so a cursor id with its top bit set arrives negative
      self.client.answerTarget(int(arg['serial']), int(arg['cursorid']) & 0xffffffff,
        int(arg.get('type', 0)))
    elif name == "version":
      # the text of a version, which the core reads again every time it arrives
      self.client.sendVersion(arg['text'])
    elif name == "house_command":
      # The 0xD7 family: a serial and a subcommand, then whatever that
      # subcommand carries, in the order the packet wants it - a graphic and an
      # offset, then a z, or a floor number on its own. A serial that is not the
      # sender's own is how a case drives the spoof check the family shares.
      args = []
      if arg.get('graphic', None) is not None:
        args.append(int(arg['graphic']))
        args.append(int(arg['x']))
        args.append(int(arg['y']))
      if arg.get('z', None) is not None:
        args.append(int(arg['z']))
      if arg.get('floor', None) is not None:
        args.append(int(arg['floor']))
      serial = arg.get('serial', None)
      self.client.houseCommand(int(arg['sub']), *args,
        serial = None if serial is None else int(serial))
    elif name == "general_info":
      # a subcommand nothing handles, which the core still has to read past
      self.client.generalInfo(int(arg['sub']), arg.get('body', []))
    else:
      raise RuntimeError("unknown client packet '{}'".format(name))

  async def processTodos(self):
    if not len(self.todos):
      return True
    # Taken all at once and the queue left empty, so a todo raised while these are
    # being run is picked up on the next turn rather than halfway through this one.
    todos = self.todos.copy()
    self.todos.clear()
    while len(todos):
      res = todos.popleft()
      todo=res["todo"]
      arg=res.get("arg",None)
      self.log.info("got todo: %s->%s", todo, arg)
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
        bp=await self.client.player.openBackPack()
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
        # nothing orders one against the other.
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
      elif todo=="prompt_reply":
        # armed before the script raises the prompt, because that call suspends it
        self.client.next_prompt_reply=arg.get('text', '')
        self.server.addevent(
          brain.Event(brain.Event.EVT_PACKET_SENT,
            clientid = self.id
            ))
      elif todo=="place_multi":
        # armed before the script raises the cursor, because that call suspends it
        placed=await self.client.placeMulti(int(arg['x']), int(arg['y']), int(arg.get('z', 0)),
          int(arg.get('graphic', 0)))
        self.server.addevent(
          brain.Event(brain.Event.EVT_MULTI_PLACED,
            clientid = self.id,
            res = placed))
      elif todo=="client_packet":
        # One packet the client knows how to build, named by what it is. Acked,
        # because the test has to know it is on its way before it asserts what it
        # did - the todo travels the test connection while the packet goes down
        # the game socket. A caller already expecting something back asks for no
        # ack, and none is raised: an unwanted one would be thrown away in front
        # of what the case is really waiting for.
        self.sendClientPacket(arg)
        if arg.get('ack', 1):
          self.server.addevent(
            brain.Event(brain.Event.EVT_PACKET_SENT,
              clientid = self.id
              ))
      elif todo=="raw_packet":
        # Caller supplied bytes on the authenticated game socket, for a packet that
        # is deliberately wrong - one cut short, or one whose fields say something
        # it does not carry. A packet that is merely one the client has no method
        # for belongs in client_packet above.
        self.client.rawPacket(arg)
        self.server.addevent(
          brain.Event(brain.Event.EVT_PACKET_SENT,
            clientid = self.id
            ))
      elif todo=="sync":
        # A barrier. The answer is raised by the client task when the core's echo
        # comes back, not from here, which is the whole point: everything the core
        # had already queued for this client is ahead of that echo. Anything the
        # caller sent before this todo is ahead of the ping too, since both travel
        # this one connection and are drained in order.
        self.client.syncPing(arg)
      elif todo=="target":
        res=await self.client.waitForTarget(5)
        targettype=None
        if res is not None:
          targettype=res.type
          if res.what==client.Target.OBJECT:
            obj=await self.client.waitForObject(arg['serial'],5)
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
        res=await self.client.waitForTarget(5)
        if res is not None:
          res.cancel()
        self.server.addevent(
          brain.Event(brain.Event.EVT_CANCEL_TARGET,
            clientid = self.id,
            res = res is not None))
      elif todo=="disable_item_logging":
        self.client.addTodo(brain.Event(brain.Event.EVT_DISABLE_ITEM_LOGGING, value = arg))
      elif todo=="weather_events":
        # Off for everyone else: these two packets raised no event at all before,
        # and the rest of the suite is written against that silence.
        self.client.weather_events = arg
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


async def main(lifecycle):
  '''! Runs the control server for as long as the shard wants one.
  @return int: the process exit code
  '''
  serv = PolServer()
  try:
    await serv.start()
  except ShardGone as ex:
    # Not a failure: nothing asked for a client. Exit quietly so the pipeline ends with POL.
    lifecycle.info("LIFECYCLE %s", ex)
    return 0

  try:
    await serv.run()
  finally: # wake up the server and let it close first
    lifecycle.info("LIFECYCLE run() left, releasing the control connection")
    serv.send("{}")
    await serv.close_control()
    lifecycle.info("LIFECYCLE control connection closed")
  return 1 if serv.failed else 0


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
  ## Bounds a write to a shard that has stopped reading. Reads need no timeout of
  ## their own: the loop waits on the stream and on nothing else.
  SEND_TIMEOUT = 5.0

  ## Longest control line accepted. A raw_packet todo can carry a big one, and the
  ## stream default of 64KiB would turn that into an error instead of a todo.
  READ_LIMIT = 1 << 20

  ## The port the shard's clientconnection.src dials. It sits inside the range
  ## Windows hands out for outbound connections, so something else on the machine
  ## can be holding it when this starts.
  CONTROL_PORT = 50000

  ## How long to keep trying for it. The shard retries its side for ten seconds
  ## (testpkgs/client/setup.src), so waiting longer only delays the same failure.
  BIND_RETRY_SECS = 8

  def __init__(self):
    self.log = logging.getLogger('server')
    conf = configparser.ConfigParser()
    path=os.path.dirname(os.path.abspath(__file__))
    conf.read(os.path.join(path,'testclient.cfg'))
    self.lconf = conf['login']
    self.clients=[]
    self.tasks=[]
    self.brains=[]
    self.server=None
    self.reader=None
    self.writer=None
    ## Set once a client task has ended badly, so the process can say so on the way out
    self.failed=False
    self._accepted = asyncio.Event()

  async def start(self):
    ''' Takes the control port and waits for the shard to dial in '''
    self.server = await self._listen(self.CONTROL_PORT)
    await self._accept(self.lconf.getint('port'))

  async def _listen(self, port):
    # A bare bind here died before anything was logged, and the only trace was the
    # shard reporting that it could not connect - which says nothing about why. Name
    # the reason, and give a transient holder a chance to let go first.
    #
    # 127.0.0.1 and AF_INET, not "localhost": that name also resolves to ::1 on a
    # host with IPv6, and a server asked for both fails the whole bind when either
    # address is taken - which is the very case the retry below exists for.
    deadline = time.monotonic() + self.BIND_RETRY_SECS
    while True:
      try:
        return await asyncio.start_server(self._onControl, '127.0.0.1', port,
                                          family=socket.AF_INET, limit=self.READ_LIMIT)
      except OSError as ex:
        if time.monotonic() >= deadline:
          self.log.error("LIFECYCLE cannot listen on control port %d after %ds: %s",
                         port, self.BIND_RETRY_SECS, ex)
          raise
        await asyncio.sleep(0.25)

  async def _onControl(self, reader, writer):
    ''' Takes the shard's connection, of which there is only ever one '''
    if self.writer is not None:
      self.log.error("LIFECYCLE a second control connection arrived, refusing it")
      writer.close()
      return
    sock = writer.get_extra_info('socket')
    if sock is not None:
      # Small request/response messages in both directions - exactly the traffic
      # Nagle holds back waiting for an ack that the other side is delaying.
      sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    self.reader, self.writer = reader, writer
    self._accepted.set()

  async def _accept(self, gameport):
    '''! Waits for the shard to dial in, or decides that it never will.

    Checked once a second rather than waited out in one go: no test package here
    connects until the shard runs the one that drives this client, which can be
    minutes into a full run, so the wait cannot be bounded by a constant. What ends
    it is the shard going away -- the same condition deafclient.py and rawpeer.py
    stop on. Without that check a run which selects no client test
    (POLCORE_TEST_FILTER) pays the whole deadline, because cmake's execute_process
    waits on every stage of the pipeline and not just POL.

    Bounded all the same, unlike the waits in deafclient.py and rawpeer.py: those
    serve requests that all run long and must outlive nothing but the shard, while
    this one is the pipeline's first stage and holds the whole run open.
    '''
    started = time.monotonic()
    shard_seen = False
    while time.monotonic() - started < HARD_DEADLINE_SECS:
      try:
        await asyncio.wait_for(self._accepted.wait(), 1.0)
        return
      except asyncio.TimeoutError:
        if not game_port_free(gameport):
          shard_seen = True
        elif shard_seen:
          raise ShardGone('shard stopped without connecting a test client')
    raise ShardGone('shard never appeared within {}s'.format(HARD_DEADLINE_SECS))

  async def run(self):
    while True:
      try:
        line = await self.reader.readuntil(b'\r\n')
      except asyncio.IncompleteReadError:
        # Clean EOF: the shard went without saying so.
        self.log.info("LIFECYCLE control connection closed by the shard")
        return True
      except (OSError, asyncio.LimitOverrunError) as ex:
        self.log.error("LIFECYCLE control connection failed: %s", ex)
        return True

      try:
        res = json.loads(line[:-2].decode())
      except Exception as e:
        self.log.error('failed to receive: {} data: "{}"'.format(
          e, line.decode(errors='replace')))
        raise
      if not res.get("todo",None):
        continue
      clientid=res.get("id",None)
      todo=res["todo"]
      arg=res.get("arg",None)
      if todo in ("connect","createchar"):
        kwargs={}
        if todo=="createchar":
          # everything past the account is handed to the create packet, so a test can pin any
          # of the values the server validates without another todo per field
          kwargs["create"] = {k:v for k,v in res.items()
                              if k not in ("todo","account","psw","name","chrindex","id")}
        t = asyncio.create_task(
              self.startclient(res["account"],res["psw"],res["name"],res["chrindex"],
                               res["id"], **kwargs),
              name="client{}".format(res["id"]))
        t.add_done_callback(self._clientDone)
        self.tasks.append(t)
      elif todo=="exit":
        # Logged per step: this process is one stage of cmake's execute_process pipeline, so if a
        # client never ends, the join below holds up the whole job and the only symptom is a
        # 600s timeout with nothing said. The lines below name which client that was.
        self.log.info("LIFECYCLE exit requested, telling %d brains to disconnect",
                      len(self.brains))
        for b in self.brains:
          b.addTodo({"todo":"disconnect"})
        for i, t in enumerate(self.tasks):
          self.log.info("LIFECYCLE joining client task %d/%d (%s)",
                        i + 1, len(self.tasks), t.get_name())
          try:
            # Shielded, so the timeout below does not cancel the task before it has
            # been named in the log.
            await asyncio.wait_for(asyncio.shield(t), timeout=30)
          except asyncio.TimeoutError:
            # Bounded on purpose: a client that cannot finish is worth a loud line in the job
            # output, not a silent timeout on the whole pipeline. Cancelled rather than left
            # behind: a task still pending when the loop closes is a warning and a socket
            # nobody closed.
            self.log.error("LIFECYCLE client task %d/%d (%s) did not end within 30s, "
                           "cancelling it", i + 1, len(self.tasks), t.get_name())
            t.cancel()
            await asyncio.gather(t, return_exceptions=True)
          except Exception:
            pass  # _clientDone has already logged it and set self.failed
          else:
            self.log.info("LIFECYCLE joined client task %d/%d", i + 1, len(self.tasks))
        self.log.info("LIFECYCLE all client tasks joined")
        self.sendEvent(brain.Event(brain.Event.EVT_EXIT,clientid=0))
        self.log.info("LIFECYCLE run() returning")
        return
      else:
        for b in self.brains:
          if b.id == clientid:
            b.addTodo(res)
            break
        else:
          self.log.error("invalid clientid")

      # write() never blocks and never fails; this is where a shard that has stopped
      # reading is noticed, and it is the whole of what the old send timeout did.
      try:
        await asyncio.wait_for(self.writer.drain(), self.SEND_TIMEOUT)
      except (asyncio.TimeoutError, OSError) as ex:
        self.log.error("LIFECYCLE control connection stopped draining: %s", ex)
        return True

  async def startclient(self,user,psw,charname,charidx,id,create=None):
    c = client.Client(id)
    self.clients.append(c)
    await c.connect(self.lconf.get('ip'), self.lconf.getint('port'), user, psw)
    await c.selectServer(self.lconf.getint('serveridx'))
    if create is None:
      await c.selectCharacter(charname, charidx)
    else:
      await c.createCharacter(charname, charidx, **create)
    b = TestBrain(c,self)
    await client.supervise(self.log, c.start(b),
                           asyncio.create_task(b.run(), name="brain{}".format(id)))

  def _clientDone(self, task):
    ''' Says what became of a client, since nothing else looks at a task's result '''
    if task.cancelled():
      return
    exc = task.exception()
    if exc is None:
      self.log.info("LIFECYCLE %s finished", task.get_name())
      return
    self.failed = True
    self.log.critical("LIFECYCLE %s died: %s", task.get_name(),
        ''.join(traceback.format_exception(type(exc), exc, exc.__traceback__)))

  def addBrain(self, brain):
    self.brains.append(brain)

  def addevent(self,ev):
    ''' Passes a brain's event straight to the shard.

    Written where it is raised rather than queued for the control loop to flush:
    the brains and that loop are on the one event loop, so the order events are
    raised in is already the order they go out in, and a queue between them would
    only delay each one by however long the loop takes to come round.
    '''
    self.sendEvent(ev)

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
      res["speed"]=ev.speed
      res["duration"]=ev.duration
      res["adjust"]=ev.adjust
      res["explode"]=ev.explode
      # only the extended packet carries these, so their presence is also what
      # says which of the two the client was sent
      for name in ("hue", "rendermode", "effect3d", "effect3dexplode",
                   "effect3dsound", "itemid", "layer"):
        if hasattr(ev, name):
          res[name]=getattr(ev, name)
    elif ev.type==Event.EVT_SOUND:
      res["mode"]=ev.mode
      res["sound"]=ev.sound
      res["pos"]=[ev.x, ev.y, ev.z]
    elif ev.type==Event.EVT_MUSIC:
      res["music"]=ev.music
    elif ev.type==Event.EVT_DAMAGE:
      res["serial"]=ev.serial
      res["damage"]=ev.damage
    elif ev.type==Event.EVT_BUFF:
      res["serial"]=ev.serial
      res["icon"]=ev.icon
      res["show"]=1 if ev.show else 0
      res["duration"]=ev.duration
      res["cl_name"]=ev.cl_name
      res["cl_descr"]=ev.cl_descr
      res["name_arguments"]=ev.name_arguments
      res["desc_arguments"]=ev.desc_arguments
    elif ev.type==Event.EVT_QUEST_ARROW:
      res["active"]=1 if ev.active else 0
      res["pos"]=[ev.x, ev.y]
      res["arrowid"]=ev.arrowid
    elif ev.type==Event.EVT_PROMPT:
      res["serial"]=ev.serial
      res["msgid"]=ev.msgid
      res["unicode"]=1 if ev.unicode else 0
    elif ev.type==Event.EVT_REFRESH_OBJ:
      res["serial"]=ev.serial
      res["graphic"]=ev.graphic
      res["color"]=ev.color
    elif ev.type==Event.EVT_MULTI_PLACED:
      res["res"]=1 if ev.res else 0
    elif ev.type==Event.EVT_MULTI_PLACEMENT:
      res["allow"]=1 if ev.allow else 0
      res["cursorid"]=ev.cursorid
      res["multiid"]=ev.multiid
      res["xoffset"]=ev.xoffset
      res["yoffset"]=ev.yoffset
      res["hue"]=ev.hue
    elif ev.type==Event.EVT_MENU:
      res["menuid"]=ev.menuid
      res["title"]=ev.title
      res["entries"]=ev.entries
    elif ev.type==Event.EVT_TIP_WINDOW:
      res["flag"]=ev.flag
      res["tipid"]=ev.tipid
      res["text"]=ev.text
    elif ev.type==Event.EVT_SEASON:
      res["season"]=ev.season
      res["playsound"]=ev.playsound
    elif ev.type==Event.EVT_WEATHER:
      # named for the wire, not for pyuo's decoder: 0x65 is type, severity, aux
      res["weather"]=ev.weather
      res["severity"]=ev.num
      res["aux"]=ev.temp
    elif ev.type==Event.EVT_LIGHT:
      res["level"]=ev.level
    elif ev.type==Event.EVT_SKILLS:
      res["skills"]=len(ev.skills)
    elif ev.type==Event.EVT_ANIMATION:
      res["cmd"]=ev.cmd
      res["serial"]=ev.serial
      res["action"]=ev.action
      # the two packets carry different things beyond the action, so each reports its own
      for name in ("anim", "subaction", "frames", "repeat", "delay"):
        if hasattr(ev, name):
          res[name]=getattr(ev, name)
    elif ev.type==Event.EVT_CHAR_PROFILE:
      res["serial"]=ev.serial
      res["title"]=ev.title
      res["utext"]=ev.utext
      res["etext"]=ev.etext
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
        res['cmd']=ev.cmd
    elif (ev.type==Event.EVT_GUMP_REPLY or ev.type==Event.EVT_DIALOG_REPLY or
        ev.type==Event.EVT_PACKET_SENT):
      pass
    elif ev.type==Event.EVT_SYNC:
      res['token']=ev.token
    elif ev.type==Event.EVT_WORLDMAP:
      res['subcmd']=ev.subcmd
      res['locations']=1 if ev.locations else 0
      res['members']=ev.members
    elif ev.type==Event.EVT_ALL_NAMES:
      res['serial']=ev.serial
      res['name']=ev.name
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
      res['serial']=ev.serial
      res['text']=ev.text
    elif ev.type==Event.EVT_TOOLTIP:
      res['serial']=ev.serial
      res['text']=ev.text
    elif ev.type==Event.EVT_OPEN_URL:
      res['url']=ev.url
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
      raise NotImplementedError("Unknown event {}".format(ev.type))

    self.send(json.dumps(res))

  def send(self, data):
    try:
      # A stream write takes the whole line or raises; there is no short write to
      # truncate one of the big replies - a listing, a house design - and leave the
      # shard waiting on an event that was only half delivered.
      self.writer.write((data+"\n").encode())
    except Exception as e:
      self.log.error("failed to send: {} {}".format(e,data))
      pass

  async def close_control(self):
    # Closing a socket that still holds unread inbound bytes makes the stack send
    # RST, and a peer that gets one may drop what it has received but not yet read -
    # here, the last reply the shard is waiting on. Half-closing puts a FIN behind
    # that reply, and emptying the receive buffer is what leaves nothing to reset
    # over. Neither step waits for the shard: it is on its own way out, and this
    # process is a pipeline stage cmake waits on.
    try:
      await asyncio.wait_for(self.writer.drain(), self.SEND_TIMEOUT)
    except (asyncio.TimeoutError, OSError):
      pass  # nothing more can be done about a shard that stopped reading
    try:
      self.writer.write_eof()
    except (OSError, NotImplementedError):
      pass  # already gone, so there is nothing to see out
    try:
      while await asyncio.wait_for(self.reader.read(65536), 1.0):
        pass
    except (asyncio.TimeoutError, OSError, asyncio.IncompleteReadError):
      pass
    self.writer.close()
    try:
      await asyncio.wait_for(self.writer.wait_closed(), 5.0)
    except (asyncio.TimeoutError, OSError):
      pass

if __name__ == '__main__':
  # WARNING by default. The handlers below log a line per packet, and several of
  # them render an object to do it, all of it written to a pipe cmake is waiting
  # on - and none of it is what a failing test is read from. The shard keeps that
  # record instead: clientconnection.src buffers every message and dumps it only
  # when a test fails. Raise this to get the old running commentary back.
  logging.basicConfig(level=os.environ.get('POLCORE_TESTCLIENT_LOGLEVEL', 'WARNING').upper(),
          stream=sys.stderr,
          format="      %(name)s:%(message)s")

  # Whatever else happens, say when this process actually goes. It is one stage of a pipeline that
  # cmake waits on in full, so a stage that never exits looks exactly like a hung shard.
  # These two carry the LIFECYCLE lines, which are the only trace of a client that
  # died, so they say their piece whatever the level above is set to.
  logging.getLogger('testclient').setLevel(logging.INFO)
  logging.getLogger('server').setLevel(logging.INFO)
  lifecycle = logging.getLogger('testclient')
  atexit.register(lambda: lifecycle.info("LIFECYCLE process exiting"))
  lifecycle.info("LIFECYCLE process starting (pid %d)", os.getpid())

  sys.exit(asyncio.run(main(lifecycle)))

