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


class TestBrain(brain.Brain):

  def __init__(self,client,server):
    self.server = server
    super(TestBrain,self).__init__( client )

  def init(self):
    self.setTimeout(0.2)


  def loop(self):
    res=self.server.recv()
    if res==False:
        return True
    if not res.get("todo",None):
        return
    todo=res["todo"]
    arg=res.get("arg",None)
    self.log.info("got todo: {}->{}".format(todo,arg))
    if todo=="disconnect":
        self.server.conn.close()
        return True
    elif todo=="speech":
        self.client.say(arg)
    elif todo=="move":
        self.client.move(arg)

  def onMovement(self, oldx, oldy, oldz, oldfacing, x, y, z, facing, ack):
    self.log.info('MOVEMENT {} {},{},{}.{} -> {},{},{}.{}'.format(
				'ack' if ack else 'rejected', oldx, oldy, oldz,
				oldfacing, x, y, z, facing))
    self.server.send(json.dumps({"move":ack}))

  def onSpeech(self, speech):
    self.log.debug('SPEECH received: {}'.format(speech))
    self.server.send(json.dumps({"speech":speech.msg}))

class PolServer:
  def __init__(self):
    self.log = logging.getLogger('brain')
    self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self.s.bind(('localhost', 50000))
    self.s.listen(1)
    self.conn, addr = self.s.accept()
    self.conn.settimeout(0.1)
  
  def recv(self):
    try:
      data = self.conn.recv(4)
    except socket.timeout:
        return {}
    except Exception as e:
      self.log.info("err {}".format(e))
      self.conn.close()
      return False
    try:
      length=int(data.decode(),16)
    except:
      return {}
    try:
      data = self.conn.recv(length).decode()
      data=json.loads(data)
    except:
      data = False
    return data

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
  # Login to the server
  c = client.Client()
  servers = c.connect(lconf.get('ip'), lconf.getint('port'), lconf.get('user'), lconf.get('pass'))
  chars = c.selectServer(lconf.getint('serveridx'))
  c.selectCharacter(lconf.get('charname'), lconf.getint('charidx'))
  TestBrain(c,serv)
