#!/usr/bin/env python3

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
        self.client.say(arg)
      elif todo=="move":
        self.client.move(arg)
      elif todo=="list_objects":
        self.client.addTodo(brain.Event(brain.Event.EVT_LIST_OBJS))
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
      elif todo=="target":
        res=self.client.waitForTarget(5)
        targettype=None
        if res is not None:
          targettype=res.type
          if res.what==client.Target.OBJECT:
            res.target(self.client.objects[arg['serial']])
          else:
            res.targetLocation(arg['x'],arg['y'],arg['z'],arg['graphic'])
        self.server.addevent(
          brain.Event(brain.Event.EVT_TARGET,
            clientid = self.id,
            targettype = targettype,
            res = res is not None))
      elif todo=="disable_item_logging":
        self.client.addTodo(brain.Event(brain.Event.EVT_DISABLE_ITEM_LOGGING, value = arg))

    return True

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
    self.s.settimeout(30) # FIXME: we need a way to stop this process without a connection
    self.conn, addr = self.s.accept()
    self.conn.settimeout(0.1)

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
              args=(res["account"],res["psw"],res["name"],res["chrindex"], res["id"]))
          )
        self.threads[-1].start()
      elif todo=="exit":
        with self.clientLock:
          for b in self.brains:
            b.addTodo({"todo":"disconnect"})
        for t in self.threads:
          t.join()
        return
      else:
        with self.clientLock:
          for b in self.brains:
            if b.id == clientid:
              b.addTodo(res)
              break
          else:
            self.log.error("invalid clientid")

  def startclient(self,user,psw,charname,charidx,id):
    with self.clientLock:
      c = client.Client(id)
      self.clients.append(c)
    servers = c.connect(self.lconf.get('ip'), self.lconf.getint('port'), user, psw)
    chars = c.selectServer(self.lconf.getint('serveridx'))
    c.selectCharacter(charname, charidx)
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
    data=b''
    while True:
      r=self._recv()
      if r is None:
        return None
      data+=r
      if not len(data):
          return {}
      if data.endswith(b'\r\n'):
        break
    data=json.loads(data.decode().rstrip('\r\n'))
    return data

  def sendEvent(self, ev):
    '''serialization method for client events'''
    res={}
    res["id"]=ev.clientid
    res["type"]=ev.typestr()
    if ev.type==Event.EVT_INIT:
      pass
    elif (ev.type==Event.EVT_HP_CHANGED or
        ev.type==Event.EVT_MANA_CHANGED or
        ev.type==Event.EVT_STAM_CHANGED or
        ev.type==Event.EVT_NOTORIETY):
      res["new"]=ev.new
      if hasattr(ev,"serial"):
        res["serial"]=ev.serial
    elif ev.type==Event.EVT_SPEECH:
      res["msg"]=ev.speech.msg
    elif ev.type==Event.EVT_MOVED:
      res["ack"]=ev.ack
      res["pos"]=[ev.x, ev.y, ev.z, ev.facing]
    elif (ev.type==Event.EVT_NEW_MOBILE or
          ev.type==Event.EVT_NEW_ITEM):
      obj = ev.mobile if ev.type==Event.EVT_NEW_MOBILE else ev.item
      res["serial"]=obj.serial
      res["pos"]=[obj.x, obj.y, obj.z, obj.facing]
      res["graphic"]=obj.graphic
    elif ev.type==Event.EVT_REMOVED_OBJ:
      res["serial"]=ev.serial
    elif ev.type==Event.EVT_LIST_OBJS:
      res["objs"]=[]
      for _,o in ev.objs.items():
        res["objs"].append(
              {'serial':o.serial,
               'pos':[o.x,o.y,o.z,o.facing],
               'graphic':o.graphic}
        )
        if hasattr(o,"parent") and o.parent is not None:
          res["objs"][-1]["parent"]=o.parent.serial
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
  
  serv = PolServer()
  try:
    serv.run()
  finally: # wake up the server and let it close first
    serv.send("{}")
    time.sleep(1)
    serv.conn.close()

