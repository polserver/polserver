#!/usr/bin/env python3

import configparser
import logging
import json
import time
import os
import sys
import socket

from pyuo import client
from pyuo import brain
from pyuo.brain import Event


class TestBrain(brain.Brain):

  def __init__(self,client,server):
    self.server = server
    super(TestBrain,self).__init__( client )

  def init(self):
    self.setTimeout(0.2)


  def loop(self):
    res=self.server.recv()
    if res is None:
        return True
    if not res.get("todo",None):
        return
    todo=res["todo"]
    arg=res.get("arg",None)
    self.log.info("got todo: {}->{}".format(todo,arg))
    if todo=="disconnect":
        self.client.exit()
        return True
    elif todo=="speech":
        self.client.say(arg)
    elif todo=="move":
        self.client.move(arg)

  def onEvent(self, ev):
    if ev.type == Event.EVT_CLIENT_CRASH:
      self.log.critical('Oops! Client crashed:', ev.exception)
      raise RuntimeError('Oops! Client crashed')  
    self.server.sendEvent(ev)

class PolServer:
  def __init__(self):
    self.log = logging.getLogger('brain')
    self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self.s.bind(('localhost', 50000))
    self.s.listen(1)
    self.conn, addr = self.s.accept()
    self.conn.settimeout(0.1)
  
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
    if ev.type==Event.EVT_HP_CHANGED:
      res["hp_changed"]=ev.new
    elif ev.type==Event.EVT_MANA_CHANGED:
      res["mana_changed"]=ev.new
    elif ev.type==Event.EVT_STAM_CHANGED:
      res["stam_changed"]=ev.new
    elif ev.type==Event.EVT_SPEECH:
      res["speech"]=ev.speech.msg
    elif ev.type==Event.EVT_NOTORIETY:
      res["notoriety"]=ev.new
    elif ev.type==Event.EVT_MOVED:
      res["move"]=ev.ack
      res["args"]=[ev.x, ev.y, ev.z, ev.facing]
    elif ev.type==Event.EVT_NEW_MOBILE:
      res["new_mobile"]=ev.mobile.serial
      res["pos"]=[ev.mobile.x, ev.mobile.y, ev.mobile.z, ev.mobile.facing]
      res["graphic"]=ev.mobile.graphic
    elif ev.type==Event.EVT_NEW_ITEM:
      res["new_item"]=ev.item.serial
      res["pos"]=[ev.item.x, ev.item.y, ev.item.z, ev.item.facing]
      res["graphic"]=ev.item.graphic
    else:
      raise NotImplementedError("Unknown event {}",format(ev.type))

    if res:
      self.send(json.dumps(res))

  def send(self, data):
    try:
      self.conn.send((data+"\n").encode())
    except:
      pass

if __name__ == '__main__':
  logging.basicConfig(level=logging.INFO, stream=sys.stderr)

  # Read configuration
  conf = configparser.ConfigParser()
  path=os.path.dirname(os.path.abspath(__file__))
  conf.read(os.path.join(path,'testclient.cfg'))
  lconf = conf['login']
  
  serv =PolServer()
  try:
    # Login to the server
    c = client.Client()
    servers = c.connect(lconf.get('ip'), lconf.getint('port'), lconf.get('user'), lconf.get('pass'))
    chars = c.selectServer(lconf.getint('serveridx'))
    c.selectCharacter(lconf.get('charname'), lconf.getint('charidx'))
    TestBrain(c,serv)
  finally: # wake up the server and let it close first
    serv.send("{}")
    time.sleep(1)
    serv.conn.close()

