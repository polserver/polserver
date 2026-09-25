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

  def __init__(self, client, id = None):
    '''! Initialize the object, must provide a connected client instance
    @param client Client: a client instance, already connected, will start it
    '''
    idstr=''if id is None else str(id)
    self.log = logging.getLogger('brain'+idstr)
    self.started = threading.Event()
    self.events = collections.deque()
    self.eventsLock = threading.Lock()
    ## Set by the client thread on an event and by the harness on a todo.
    self.wakeup = threading.Event()
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

    # Wait for client to start us, then initialize.
    #
    # The event behind this is only set when the login completes, and a login does not always
    # complete: a character creation the server refuses drops the connection instead. Waiting
    # forever for that would park this thread - which is the caller's, since __init__ runs the
    # brain inline - and testclient.py joins every one of those before it exits, so one refused
    # login used to hang the whole cmake pipeline on a 600s timeout with nothing said.
    self.log.info('Waiting for client to start')
    while not self.started.wait(timeout=1.0):
      if not self.client.is_alive():
        self.log.error('client stopped before its login completed; brain is giving up')
        return
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

      # Block on the wakeup rather than poll: this decides how fast a brain reacts.
      if self.timeout:
        deadline = time.monotonic() + self.timeout
        while True:
          self.processEvents()
          if self.hasWork():
            break
          remaining = deadline - time.monotonic()
          if remaining <= 0:
            break
          # Clear before the last look at the queues, so a wakeup in between is kept.
          self.wakeup.clear()
          if self.hasWork() or self.hasEvents():
            continue
          self.wakeup.wait(remaining)

  def onEvent(self, ev):
    raise RuntimeError('needs to be overridden')

  def processEvents(self):
    ''' Process event queue, internal '''
    with self.eventsLock:
      if not self.events:
        return
      # Swap rather than copy, so the producer waits only for the swap.
      events = self.events
      self.events = collections.deque()
    while events:
      self.onEvent(events.popleft())

  def hasEvents(self):
    ''' Whether anything is queued for processEvents(), internal '''
    with self.eventsLock:
      return len(self.events) > 0


  def event(self, ev):
    ''' Internal function, injects a single event, called from the client thread '''
    if not isinstance(ev, Event):
      raise RuntimeError("Unknown event, expecting an Event instance, got {}".format(type(ev)))

    with self.eventsLock:
      self.events.append(ev)
    self.wakeup.set()

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

  def hasWork(self):
    '''! Whether loop() has something waiting for it before the timeout is up

    A brain that decides for itself has nothing to answer here and waits out
    the whole timeout. One driven from outside overrides this, or every order
    it is given waits for the timeout to expire before it is even looked at.
    @return Return true to cut the wait short and run loop() now
    '''
    return False

class Event:
  ''' An event sent from the client '''

  EVT_HP_CHANGED = 1
  EVT_MANA_CHANGED = 2
  EVT_STAM_CHANGED = 3
  EVT_SPEECH = 4
  EVT_NOTORIETY = 5
  EVT_MOVED = 6
  EVT_NEW_MOBILE = 7
  EVT_NEW_ITEM = 8
  EVT_REMOVED_OBJ = 9
  EVT_NEW_SUBSERVER = 10
  EVT_BOAT_MOVED = 11
  EVT_OWNCREATE = 12
  EVT_GUMP = 13
  EVT_OPEN_PAPERDOLL = 14
  EVT_AUTO_DELETE_OBJS = 15
  EVT_ATTACK = 16
  EVT_WAR_MODE = 17
  EVT_FIGHT_OCCURING = 18
  EVT_STATUS_BAR = 19
  EVT_TRADE = 20
  EVT_CLILOC = 21
  EVT_PARTY = 22
  EVT_HOUSE_DESIGN = 23
  EVT_HOUSE_EDIT = 24
  EVT_HOUSE_REV = 25
  EVT_SPELLBOOK = 26
  EVT_MAP = 27
  EVT_MAP_PIN = 28
  EVT_OUT_OF_RANGE_OBJ = 29
  EVT_OBJ_REVISION = 30
  EVT_EFFECT = 31
  EVT_CLOSE_WINDOW = 32
  EVT_TEXT_ENTRY = 33
  EVT_SELECT_COLOR = 34
  EVT_RESURRECT_MENU = 35
  EVT_RACE_CHANGER = 36
  EVT_BOOK = 37
  EVT_BOOK_PAGE = 38
  EVT_POPUP = 39
  EVT_VENDOR_SELL_LIST = 40

  EVT_EXIT = 100
  EVT_LIST_OBJS = 101
  EVT_OPEN_BACKPACK = 102
  EVT_TARGET = 103
  EVT_DISABLE_ITEM_LOGGING = 104
  EVT_DOUBLE_CLICK = 105
  EVT_LIFT_ITEM = 106
  EVT_MOVE_ITEM_REJECTED = 107
  EVT_DROP_ITEM = 108
  EVT_DROP_APPROVED = 109
  EVT_LIST_EQUIPPED_ITEMS = 110
  EVT_BOAT_MOVE = 111
  EVT_AOS_TOOLTIP = 112
  EVT_WEAR_ITEM = 113
  EVT_CANCEL_TARGET = 114
  EVT_BUY_ITEMS = 115
  EVT_SELL_ITEMS = 116
  EVT_RACE_CHANGE = 117
  EVT_GUMP_REPLY = 118
  EVT_DIALOG_REPLY = 119
  EVT_PACKET_SENT = 120
  EVT_ALL_NAMES = 121
  EVT_WORLDMAP = 122
  EVT_SOUND = 123
  EVT_MUSIC = 124
  EVT_DAMAGE = 125
  EVT_BUFF = 126
  EVT_CHAR_PROFILE = 127
  EVT_ANIMATION = 128
  EVT_QUEST_ARROW = 129
  EVT_MULTI_PLACEMENT = 130
  EVT_MENU = 131
  EVT_TIP_WINDOW = 132
  EVT_SEASON = 133
  EVT_SKILLS = 134
  EVT_MULTI_PLACED = 135
  EVT_REFRESH_OBJ = 136
  EVT_PROMPT = 137
  EVT_WEATHER = 138
  EVT_LIGHT = 139
  EVT_TOOLTIP = 140
  EVT_OPEN_URL = 141
  # The answer to a harness ping. See client_sync() in communication.inc.
  EVT_SYNC = 142

  EVT_INIT = 254
  EVT_CLIENT_CRASH = 255

  def __init__(self, type, **kwargs):
    self.type = type
    for k, v in kwargs.items():
      setattr(self, k, v)

  def typestr(self):
    ''' keep in sync with testscript
        unique type strings '''
    return TYPESTR.get(self.type)


# Event names the test scripts match on. Kept in step with the EVT_* constants in
# testsuite/pol/testpkgs/client/communication.inc.
TYPESTR = {
  Event.EVT_INIT: "init",
  Event.EVT_HP_CHANGED: "hp_changed",
  Event.EVT_MANA_CHANGED: "mana_changed",
  Event.EVT_STAM_CHANGED: "stam_changed",
  Event.EVT_SPEECH: "speech",
  Event.EVT_NOTORIETY: "notoriety",
  Event.EVT_MOVED: "moved",
  Event.EVT_NEW_MOBILE: "new_mobile",
  Event.EVT_NEW_ITEM: "new_item",
  Event.EVT_REMOVED_OBJ: "removed_obj",
  Event.EVT_OUT_OF_RANGE_OBJ: "out_of_range_obj",
  Event.EVT_OBJ_REVISION: "obj_revision",
  Event.EVT_EFFECT: "effect",
  Event.EVT_SOUND: "sound",
  Event.EVT_MUSIC: "music",
  Event.EVT_DAMAGE: "damage",
  Event.EVT_BUFF: "buff",
  Event.EVT_CHAR_PROFILE: "char_profile",
  Event.EVT_ANIMATION: "animation",
  Event.EVT_QUEST_ARROW: "quest_arrow",
  Event.EVT_MULTI_PLACEMENT: "multi_placement",
  Event.EVT_MENU: "menu",
  Event.EVT_TIP_WINDOW: "tip_window",
  Event.EVT_TOOLTIP: "tooltip",
  Event.EVT_OPEN_URL: "open_url",
  Event.EVT_SEASON: "season",
  Event.EVT_SKILLS: "skills",
  Event.EVT_MULTI_PLACED: "multi_placed",
  Event.EVT_REFRESH_OBJ: "refresh_obj",
  Event.EVT_PROMPT: "prompt",
  Event.EVT_WEATHER: "weather",
  Event.EVT_LIGHT: "light",
  Event.EVT_CLOSE_WINDOW: "close_window",
  Event.EVT_GUMP_REPLY: "gump_reply",
  Event.EVT_DIALOG_REPLY: "dialog_reply",
  Event.EVT_PACKET_SENT: "packet_sent",
  Event.EVT_ALL_NAMES: "all_names",
  Event.EVT_WORLDMAP: "worldmap",
  Event.EVT_TEXT_ENTRY: "text_entry",
  Event.EVT_SELECT_COLOR: "select_color",
  Event.EVT_RESURRECT_MENU: "resurrect_menu",
  Event.EVT_RACE_CHANGER: "race_changer",
  Event.EVT_BOOK: "book",
  Event.EVT_BOOK_PAGE: "book_page",
  Event.EVT_POPUP: "popup",
  Event.EVT_VENDOR_SELL_LIST: "vendor_sell_list",
  Event.EVT_EXIT: "exit",
  Event.EVT_LIST_OBJS: "list_objs",
  Event.EVT_LIST_EQUIPPED_ITEMS: "list_equipped_items",
  Event.EVT_OPEN_BACKPACK: "open_bp",
  Event.EVT_TARGET: "target",
  Event.EVT_NEW_SUBSERVER: "new_subserver",
  Event.EVT_DISABLE_ITEM_LOGGING: "disable_item_logging",
  Event.EVT_BOAT_MOVED: "boat_moved",
  Event.EVT_OWNCREATE: "owncreate",
  Event.EVT_DOUBLE_CLICK: "double_click",
  Event.EVT_LIFT_ITEM: "lift_item",
  Event.EVT_MOVE_ITEM_REJECTED: "move_item_rejected",
  Event.EVT_DROP_ITEM: "drop_item",
  Event.EVT_WEAR_ITEM: "wear_item",
  Event.EVT_CANCEL_TARGET: "cancel_target",
  Event.EVT_BUY_ITEMS: "buy_items",
  Event.EVT_SELL_ITEMS: "sell_items",
  Event.EVT_RACE_CHANGE: "race_change",
  Event.EVT_BOAT_MOVE: "boat_move",
  Event.EVT_DROP_APPROVED: "drop_approved",
  Event.EVT_GUMP: "gump",
  Event.EVT_AOS_TOOLTIP: "aos_tooltip",
  Event.EVT_OPEN_PAPERDOLL: "open_paperdoll",
  Event.EVT_AUTO_DELETE_OBJS: "auto_delete_objs",
  Event.EVT_ATTACK: "attack",
  Event.EVT_WAR_MODE: "war_mode",
  Event.EVT_FIGHT_OCCURING: "fight_occuring",
  Event.EVT_STATUS_BAR: "status_bar",
  Event.EVT_TRADE: "trade",
  Event.EVT_CLILOC: "cliloc",
  Event.EVT_PARTY: "party",
  Event.EVT_HOUSE_DESIGN: "house_design",
  Event.EVT_HOUSE_EDIT: "house_edit",
  Event.EVT_HOUSE_REV: "house_rev",
  Event.EVT_SPELLBOOK: "spellbook",
  Event.EVT_MAP: "map",
  Event.EVT_MAP_PIN: "map_pin",
  Event.EVT_SYNC: "sync",
}
