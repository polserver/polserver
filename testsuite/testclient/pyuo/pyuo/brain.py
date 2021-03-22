#!/usr/bin/env python3

'''
AI classes for Python Ultima Online text client
Copyright (C) 2015-2016 Gabriele Tozzi

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
'''

import threading
import logging
import time
import collections


class Brain:
  ''' This is the Brain for the client, the code that takes decisions

  Usually runs in the main thread, starts the client thread.
  '''

  def __init__(self, client):
    '''! Initialize the object, must provide a connected client instance
    @param client Client: a client instance, already connected, will start it
    '''
    self.log = logging.getLogger('brain')
    self.started = threading.Event()
    self.events = collections.deque()
    self.eventsLock = threading.Lock()
    ## Client reference
    self.client = client
    ## Reference to current player
    self.player = None
    ## Reference to list of known objects
    self.objects = None
    ## Default timeout while waiting for events
    self.timeout = 5

    client.start(self)
    self.run()

  def run(self):
    ''' This is the main Brain thread entry point, contains the main loop, internal '''

    # Wait for client to start us, then initialize
    self.log.info('Waiting for client to start')
    self.started.wait()
    self.log.info('Client started')
    self.player = self.client.player
    self.objects = self.client.objects
    self.init()

    # Enter main loop
    while True:
      if not self.client.is_alive():
        self.processEvents()
        # Should not reach this point
        self.log.critical("Client crashed and didn't tell me.")
        raise RuntimeError("Client crashed and didn't tell me.")

      if self.loop():
        self.log.info('Main loop terminated.')
        break

      # Wait for events
      self.processEvents()
      if not len(self.events) and self.timeout:
        start = time.time()
        timeout = start + self.timeout
        while time.time() < timeout:
          self.processEvents()
          time.sleep(0.01)

  def onEvent(self, ev):
    if ev.type == Event.EVT_HP_CHANGED:
      self.onHpChange(ev.old, ev.new)
    elif ev.type == Event.EVT_MANA_CHANGED:
      self.onManaChange(ev.old, ev.new)
    elif ev.type == Event.EVT_STAM_CHANGED:
      self.onStamChange(ev.old, ev.new)
    elif ev.type == Event.EVT_SPEECH:
      self.onSpeech(ev.speech)
    elif ev.type == Event.EVT_NOTORIETY:
      self.onNotorietyChange(ev.old, ev.new)
    elif ev.type == Event.EVT_MOVED:
      self.onMovement(ev.oldx, ev.oldy, ev.oldz, ev.oldfacing,
            ev.x, ev.y, ev.z, ev.facing, ev.ack)
    elif ev.type == Event.EVT_NEW_MOBILE:
      self.onNewMobile(ev.mobile)
    elif ev.type == Event.EVT_CLIENT_CRASH:
      self.log.critical('Oops! Client crashed:', ev.exception)
      raise RuntimeError('Oops! Client crashed')
    else:
      raise NotImplementedError("Unknown event {}",format(ev.type))

  def processEvents(self):
    ''' Process event queue, internal '''
    while True:
      with self.eventsLock:
        if not len(self.events):
          return
        ev = self.events.popleft()
        self.onEvent(ev)


  def event(self, ev):
    ''' Internal function, injects a single event, called from the client thread '''
    if not isinstance(ev, Event):
      raise RuntimeError("Unknown event, expecting an Event instance, got {}".format(type(ev)))

    with self.eventsLock:
      self.events.append(ev)

  def setTimeout(self, timeout):
    ''' Sets the new timeout in seconds for the main loop '''
    self.timeout = timeout


  ###################################
  # Methods intended to be overridden
  ###################################

  def init(self):
    ''' Called just once before first loop '''
    self.log.debug('Brain inited')

  def loop(self):
    '''! This is called once every main loop iteration, the main brain's loop
    @return Return true to terminate the program
    '''
    self.log.debug('Brain running, nothing to do...')

  def onHpChange(self, old, new):
    ''' Called when HP amount changes '''
    self.log.debug('HP changed from {} to {}'.format(old, new))

  def onManaChange(self, old, new):
    ''' Called when HP amount changes '''
    self.log.debug('MANA changed from {} to {}'.format(old, new))

  def onStamChange(self, old, new):
    ''' Called when HP amount changes '''
    self.log.debug('STAM changed from {} to {}'.format(old, new))

  def onNotorietyChange(self, old, new):
    ''' Called when notoriety changes '''
    self.log.debug('NOTORIETY changed from {} to {}'.format(old, new))

  def onMovement(self, oldx, oldy, oldz, oldfacing, x, y, z, facing, ack):
    ''' Called when movement has been confirmed '''
    self.log.debug('MOVEMENT {} {},{},{}.{} -> {},{},{}.{}'.format(
        'ack' if ack else 'rejected', oldx, oldy, oldz,
        oldfacing, x, y, z, facing))

  def onNewMobile(self, mobile):
    ''' Called when a new mobile is in sight '''
    self.log.debug('NEW MOBILE: {}'.format(mobile))

  def onSpeech(self, speech):
    ''' Called when somebody said something or on a sys or global chat message '''
    self.log.debug('SPEECH received: {}'.format(speech))


class Event:
  ''' An event sent from the client '''

  EVT_HP_CHANGED = 1
  EVT_MANA_CHANGED = 2
  EVT_STAM_CHANGED = 3
  EVT_SPEECH = 4
  EVT_NOTORIETY = 5
  EVT_MOVED = 6
  EVT_NEW_MOBILE = 7

  EVT_CLIENT_CRASH = 255

  def __init__(self, type, **kwargs):
    self.type = type
    for k, v in kwargs.items():
      setattr(self, k, v)
