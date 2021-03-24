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
    with self.server.eventsLock:
      self.server.brains.append(self)
    self.todos = collections.deque()
    self.todosLock = threading.Lock()
    super(TestBrain,self).__init__( client, self.id )

  def init(self):
    self.setTimeout(0.2)
    self.server.addevent(brain.Event(brain.Event.EVT_INIT, clientid=self.id))

  def loop(self):
    if not self.processTodos():
      return True

  def onEvent(self, ev):
    if ev.type == Event.EVT_CLIENT_CRASH:
      self.log.critical('Oops! Client crashed: {}'.format(ev.exception))
      raise RuntimeError('Oops! Client crashed')
    if (ev.type==Event.EVT_HP_CHANGED or
        ev.type==Event.EVT_MANA_CHANGED or
        ev.type==Event.EVT_STAM_CHANGED):
      self.log.info("ev : {}, {}, {}".format(ev.typestr(),ev.new,ev.serial))
    ev.clientid=self.id
    self.server.addevent(ev)
  
  def addTodo(self,ev):
    with self.todosLock:
      self.todos.append(ev)

  def processTodos(self):
    while True:
      with self.todosLock:
        if not len(self.todos):
          return True
        res = self.todos.popleft()
        todo=res["todo"]
        arg=res.get("arg",None)
        self.log.info("got todo: {}->{}".format(todo,arg))
        if todo=="disconnect":
          self.client.exit()
          return False
        elif todo=="speech":
          self.client.say(arg)
        elif todo=="move":
          self.client.move(arg)
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
    self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self.s.bind(('localhost', 50000))
    self.s.listen(1)
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
              args=(res["account"],res["psw"],res["name"],res["chrindex"]))
          )
        self.threads[-1].start()
      elif todo=="exit":
        for b in self.brains:
          b.addTodo({"todo":"disconnect"})
        for t in self.threads:
          t.join()
        return
      else:
        if clientid is None:
          self.log.error("invalid clientid")
          continue
        self.brains[clientid].addTodo(res)

  def startclient(self,user,psw,charname,charidx):
    with self.eventsLock:
      c = client.Client(len(self.clients))
      self.clients.append(c)
    servers = c.connect(self.lconf.get('ip'), self.lconf.getint('port'), user, psw)
    chars = c.selectServer(self.lconf.getint('serveridx'))
    c.selectCharacter(charname, charidx)
    TestBrain(c,self)

  def addevent(self,ev):
    with self.eventsLock:
      self.events.append(ev)

  def brainevents(self):
    while True:
      with self.eventsLock:
        if not len(self.events):
          return
        ev = self.events.popleft()
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
    else:
      raise NotImplementedError("Unknown event {}",format(ev.type))

    if res:
      self.send(json.dumps(res))

  def send(self, data):
    try:
      self.conn.send((data+"\n").encode())
      self.log.info("send:"+data)
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

