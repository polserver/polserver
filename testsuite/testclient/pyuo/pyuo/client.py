#!/usr/bin/env python3

'''
Python Ultima Online text client (experiment)
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

import sys
import collections
import threading
import struct
import logging
import socket
import ipaddress
import time
import traceback

from . import net
from . import packets
from . import brain


class status:
	''' Decorator, checks that status is valid '''

	def __init__(self, status):
		self.status = status

	def __call__(self, f):
		def wrapper(*args):
			if args[0].status != self.status:
				raise StatusError("Status {} not valid, need {}".format(args[0].status, self.status))
			return f(*args)
		return wrapper


def clientthread(f):
	''' Decorator, checks that method is run in the client thread only unless initing '''
	def wrapper(client, *args, **kwargs):
		if status == 'game':
			mythread = threading.current_thread()
			if mythread is not client:
				raise ThreadError("This must run in client thread only, currently in {}".format(mythread))
		return f(client, *args, **kwargs)
	return wrapper


def logincomplete(f):
	''' Decorator, check that login procedure is complete '''
	def wrapper(client, *args, **kwargs):
		if not client.lc or client.status != 'game':
			raise StatusError("Must complete login procedure before calling this")
		return f(client, *args, **kwargs)
	return wrapper


class UOBject:
	''' Base class for an UO Object '''

	def __init__(self, client):
		## Logging instance
		self.log = logging.getLogger(self.__class__.__name__)
		## Client reference
		self.client = client
		
		## Unique serial number
		self.serial = None
		## Graphic ID
		self.graphic = None
		## Color ID
		self.color = None
		## X coordinate
		self.x = None
		## Y coordinate
		self.y = None
		## z coordinate
		self.z = None
		## Facing
		self.facing = None


class Item(UOBject):
	''' Represents an item in the world '''

	def __init__(self, client, pkt=None):
		super().__init__(client)

		## Number of items in the stack
		self.amount = 1
		## Status flags
		self.status = None

		if pkt is not None:
			self.update(pkt)

	def update(self, pkt):
		''' Update from packet '''
		if not isinstance(pkt, packets.ObjectInfoPacket):
			raise ValueError("Expecting a DrawObjectPacket")
		self.serial = pkt.serial
		self.graphic = pkt.graphic
		self.amount = pkt.count
		self.x = pkt.x
		self.y = pkt.y
		self.z = pkt.z
		self.facing = pkt.facing
		self.color = pkt.color if pkt.color else 0
		self.status = pkt.flag

	def upgradeToContainer(self):
		''' Upgrade this item to a container '''
		self.__class__ = Container
		self.upgrade()

	def use(self):
		''' Uses the given item '''
		self.client.doubleClick(self)

	def __repr__(self):
		serial = hex(self.serial)
		graphic = hex(self.graphic)
		color = hex(self.color)
		return "{} Item {} graphic {} color {} at {},{},{} facing {}".format(
				self.amount, serial, graphic, color, self.x, self.y, self.z, self.facing )


class Container(Item):
	''' A special representation of item '''

	def __init__(self, client):
		super().__init__(client)
		self.upgrade()

	def upgrade(self):
		''' Called when an Item's class has just been changed to container '''

		## The Content, none if still not received
		self.content = None

	def addItem(self, pkt):
		''' Adds an item to container, from packet or dictionary '''
		if type(pkt) == dict:
			it = pkt
		elif isinstance(pkt, packets.AddItemToContainerPacket):
			it = pkt.__dict__
		else:
			raise ValueError("Expecting a AddItem(s)ToContainerPacket")

		if it['serial'] in self.client.objects.keys():
			item = self.client.objects[it['serial']]
		else:
			item = Item(self.client)
			item.serial = it['serial']
			self.client.objects[it['serial']] = item
		item.graphic = it['graphic']
		item.amount = it['amount']
		item.x = it['x']
		item.y = it['y']
		item.color = it['color']

		if self.content is None:
			self.content = []
		self.content.append(item)

	def __iter__(self):
		return self.content.__iter__()

	def __next__(self):
		return self.content.__next__()

	def __getitem__(self, key):
		return self.content[key]


class Mobile(UOBject):
	''' Represents a mobile in the world '''

	# Constants for equipment layers
	LAYER_NONE        = 0x00 #  0. Not used?
	LAYER_HAND1       = 0x01 #  1. One handed weapon.
	LAYER_HAND2       = 0x02 #  2. Two handed weapon, shield or misc.
	LAYER_SHOES       = 0x03 #  3. Shoes.
	LAYER_PANTS       = 0x04 #  4. Pants.
	LAYER_SHIRT       = 0x05 #  5. Shirt.
	LAYER_HELM        = 0x06 #  6. Helm or Hat.
	LAYER_GLOVES      = 0x07 #  7. Gloves.
	LAYER_RING        = 0x08 #  8. Ring.
	LAYER_TALISMAN    = 0x09 #  9. Talisman. (since POL097, Mondain's Legacy)
	LAYER_NECK        = 0x0a # 10. Neck.
	LAYER_HAIR        = 0x0b # 11. Hair
	LAYER_WAIST       = 0x0c # 12. Waist (half apron).
	LAYER_CHEST       = 0x0d # 13. Torso (inner) (chest armor).
	LAYER_WRIST       = 0x0e # 14. Bracelet.
	LAYER_PACK2       = 0x0f # 15. Unused (backpack, but ord. bp is 0x15).
	LAYER_BEARD       = 0x10 # 16. Facial hair.
	LAYER_TUNIC       = 0x11 # 17. Torso (middle) (tunic, sash etc.).
	LAYER_EARS        = 0x12 # 18. Earrings.
	LAYER_ARMS        = 0x13 # 19. Arms.
	LAYER_CAPE        = 0x14 # 20. Back (cloak). (Also Quivers in Mondain's Legacy)
	LAYER_PACK        = 0x15 # 21. Backpack
	LAYER_ROBE        = 0x16 # 22. Torso (outer) (robe)
	LAYER_SKIRT       = 0x17 # 23. Legs (outer) (skirt/robe).
	LAYER_LEGS        = 0x18 # 24. Legs (inner) (leg armor).
	LAYER_MOUNT       = 0x19 # 25. Mount (horse, ostard etc.).
	LAYER_VENDORSTOCK = 0x1a # 26. This vendor will sell and restock.
	LAYER_VENDOREXTRA = 0x1b # 27. This vendor will resell to players but not restock.
	LAYER_VENDORBUY   = 0x1c # 28. This vendor can buy from players but does not stock.
	LAYER_BANKBOX     = 0x1d # 29. Contents of bankbox
	LAYER_TRADE       = 0x1e # 30. Can be multiple of these, do not use directly.

	# Constants for notoriety
	NOTO_INNOCENT = 1
	NOTO_FRIEND   = 2
	NOTO_ANIMAL   = 3
	NOTO_CRIMINAL = 4
	NOTO_ENEMY    = 5
	NOTO_MURDERER = 6
	NOTO_INVUL    = 7

	def __init__(self, client, pkt=None):
		super().__init__(client)

		## Status flags
		## 0x00: Normal
		## 0x01: Unknown
		## 0x02: Can Alter Paperdoll
		## 0x04: Poisoned
		## 0x08: Golden Health
		## 0x10: Unknown
		## 0x20: Unknown
		## 0x40: War Mode
		self.status = None
		## War mode flag
		self.war = None
		## Notoriety
		## 0x1: Innocent (Blue)
		## 0x2: Friend (Green)
		## 0x3: Grey (Grey - Animal)
		## 0x4: Criminal (Grey)
		## 0x5: Enemy (Orange)
		## 0x6: Murderer (Red)
		## 0x7: Invulnerable (Yellow)
		self.notoriety = None
		## Hit Points
		self.hp = None
		## Max Hit Points
		self.maxhp = None
		## Mana
		self.mana = None
		## Max Mana
		self.maxmana = None
		## Stamina
		self.stam = None
		## Max Stamina
		self.maxstam = None
		## Equip serials list, by layer (None = unknown)
		self.equip = None

		if pkt is not None:
			self.update(pkt)

	def update(self, pkt):
		''' Update from packet '''
		if not isinstance(pkt, packets.UpdatePlayerPacket) and not isinstance(pkt, packets.DrawObjectPacket):
			raise ValueError("Expecting an UpdatePlayerPacket or DrawObjectPacket")
		self.serial = pkt.serial
		self.graphic = pkt.graphic
		self.x = pkt.x
		self.y = pkt.y
		self.z = pkt.z
		self.facing = pkt.facing
		self.color = pkt.color
		self.status = pkt.flag
		self.notoriety = pkt.notoriety

		# Handle equip
		if isinstance(pkt, packets.DrawObjectPacket):
			self.equip = {}
			for eq in pkt.equip:
				serial = eq['serial']
				if serial in self.client.objects.keys():
					item = self.client.objects[serial]
				else:
					item = Item(self.client)
					item.serial = eq['serial']
					self.client.objects[item.serial] = item
				item.graphic = eq['graphic']
				item.color = eq['color']

				self.equip[eq['layer']] = item

	def getEquipByLayer(self, layer):
		''' Returns item equipped in the given layer '''
		self.client.waitFor(lambda: self.equip is not None)
		return self.equip[layer]

	def __repr__(self):
		return "Mobile 0x{serial:02X} graphic 0x{graphic:02X} color 0x{color:02X} at {x},{y},{z} facing {facing}".format(**self.__dict__)


class Player(Mobile):
	''' Represents the current player '''

	def __init__(self, client):
		super().__init__(client)

		## Current target serial
		self.target = None

	def openBackPack(self):
		''' Opens player's backpack, waits for it to be loaded '''
		bp = self.getEquipByLayer(self.LAYER_PACK)
		if not isinstance(bp, Container):
			self.client.doubleClick(bp)
			self.client.waitFor(lambda: isinstance(bp, Container))
			self.client.waitFor(lambda: bp.content is not None)
		return bp


class Target:
	''' Represents an active target '''

	# Constants for what
	OBJECT = packets.TargetCursorPacket.OBJECT
	LOCATION = packets.TargetCursorPacket.LOCATION

	# Constants for type
	NEUTRAL = packets.TargetCursorPacket.NEUTRAL
	HARMFUL = packets.TargetCursorPacket.HARMFUL
	HELPFUL = packets.TargetCursorPacket.HELPFUL

	def __init__(self, client, pkt):
		assert isinstance(pkt, packets.TargetCursorPacket)

		self.client = client

		self.what = pkt.what
		self.id = pkt.id
		self.type = pkt.type

	def target(self, obj):
		''' Sends a target for the given object '''
		po = packets.TargetCursorPacket()
		assert self.what == po.OBJECT
		po.fill(self.what, self.id, self.type, obj.serial)
		self.client.target = None
		self.client.send(po)


class Speech:
	''' Represents something that has been spoken '''

	# Constants for type
	SAY       = 0x00
	BROADCAST = 0x01
	EMOTE     = 0x02
	SYSTEM    = 0x06
	MESSAGE   = 0x07
	WHISPER   = 0x08
	YELL      = 0x09
	SPELL     = 0x0a
	GUILD     = 0x0d
	ALLIANCE  = 0x0e
	COMMAND   = 0x0f

	def __init__(self, client, pkt):
		if isinstance(pkt, packets.SendSpeechPacket):
			self.unicode = False
		elif isinstance(pkt, packets.UnicodeSpeechPacket):
			self.unicode = True
		else:
			assert False

		self.client = client

		self.serial = pkt.serial
		self.model = pkt.model
		self.type = pkt.type
		self.color = pkt.color
		self.font = pkt.font
		if self.unicode:
			self.lang = pkt.lang
		else:
			self.lang = None
		self.name = pkt.name
		self.msg = pkt.msg

	def typeName(self):
		''' Returns type as string '''
		if self.type == self.SAY:
			return "Say"
		elif self.type == self.BROADCAST:
			return "Broadcast"
		elif self.type == self.EMOTE:
			return "Emote"
		elif self.type == self.SYSTEM:
			return "System"
		elif self.type == self.MESSAGE:
			return "Message"
		elif self.type == self.WHISPER:
			return "Whisper"
		elif self.type == self.YELL:
			return "Yell"
		elif self.type == self.SPELL:
			return "Spell"
		elif self.type == self.GUILD:
			return "Guild Chat"
		elif self.type == self.ALLIANCE:
			return "Alliance Chat"
		elif self.type == self.COMMAND:
			return "Command prompt"
		return "Unknown message"

	def __str__(self):
		if self.type == self.SAY:
			return "{}: {}".format(self.name, self.msg)
		elif self.type == self.EMOTE:
			return "{} {}".format(self.name, self.msg)
		elif self.type == self.WHISPER:
			return "{} whispers: {}".format(self.name, self.msg)
		elif self.type == self.YELL:
			return "{} yells: {}".format(self.name, self.msg)
		return "[{}] {}: {}".format(self.typeName().upper(), self.name, self.msg)

	def __repr__(self):
		p = "u" if self.unicode else ""
		return '{} from 0x{:02X} ({}): {}"{}"'.format(
				self.typeName(), self.serial, self.name, p, self.msg)


class Direction:
	''' Represents a direction (movement, facing) '''

	# Constants for id (cardinal points)
	N  = 0
	NE = 1
	E  = 2
	SE = 3
	S  = 4
	SW = 5
	W  = 6
	NW = 7

	def __init__(self, id):
		if id < 0 or id > 7:
			raise ValueError('valid direction ids are 0-7')
		self.id = id


class Client(threading.Thread):
	''' The main client instance and thread

	Runs in a mixed threading mode: first initialize the client in the main
	thread, then call run() to move the client to a seperate thread.
	You can call the methods that are not decorated with @clientthread from
	any thread.
	'''

	## Minimum interval between two pings
	PING_INTERVAL = 30
	## Version sent to server
	VERSION = '5.0.9.1'
	## Language sent to server
	LANG = 'ENU'

	def __init__(self):
		super().__init__()
		# Change the thread name to better identify
		self.name = 'Client' + self.name

		## Send queue
		self.sendqueue = []
		## Lock for the send queue
		self.sendqueueLock = threading.Lock()

		## Dict info about last server connected to {ip, port, user, pass}
		self.server = None
		## Current client status, one of:
		## - disconnected: The client is not connected
		## - connected: Connected and logged in, server not selected
		## - loggedin: Connected to game server, character not selected
		self.status = 'disconnected'
		## Login complete, will be false during the initial fase of the game
		self.lc = False
		## When to send next ping
		self.ping = 0
		## Logger, for internal usage
		self.log = logging.getLogger('client')
		## Features sent with 0xb9 packet
		self.features = None
		## Flags sent with 0xa9 packet
		self.flags = None
		## Locations sent with 0xa9 packets
		self.locs = None
		## Last move sequence number used
		self.moveid = -1
		## Lock for incrementing moveId
		self.moveLock = threading.Lock()
		## Unacknowledged moves
		self.unmoves = collections.deque()

		## Reference to player, character instance
		self.player = None
		## Dictionary of Objects (Mobiles and Items) around, by serial
		self.objects = {}
		## Reference to current active target, if any
		self.target = None

		## Current Realm's width
		self.width = None
		## Current Realm's height
		self.height = None

		## Current cursor (0 = Felucca, unhued / BRITANNIA map. 1 = Trammel, hued gold / BRITANNIA map, 2 = (switch to) ILSHENAR map)
		self.cursor = None

	@status('disconnected')
	def connect(self, host, port, user, pwd):
		'''! Connnects to the server, returns a list of gameservers
			@param host string: Server IP address or hostname
			@param port string: Server port
			@param user string: Username
			@param pwd string: Password
			@return list of dicts{name, tz, full, idx, ip}
			@throws LoginDeniedError
		'''

		try:
			ip = ipaddress.ip_address(host)
		except ValueError:
			self.log.info('resolving %s', host)
			ip = ipaddress.ip_address(socket.gethostbyname(host))

		self.server = {
			'ip': ip,
			'port': port,
			'user': user,
			'pass': pwd,
		}

		self.log.info('connecting')
		self.net = net.Network(self.server['ip'], self.server['port'])

		# Send IP as key (will not use encryption)
		self.queue(ipaddress.ip_address(self.server['ip']).packed)

		# Send account login request
		self.log.info('logging in')
		po = packets.LoginRequestPacket()
		po.fill(self.server['user'], self.server['pass'])
		self.queue(po)

		# Flush send buffer
		self.send()

		# Get servers list
		pkt = self.receive((packets.ServerListPacket, packets.LoginDeniedPacket))
		if isinstance(pkt, packets.LoginDeniedPacket):
			self.log.error('login denied')
			raise LoginDeniedError(pkt.reason)

		self.log.debug("Received serverlist: %s", str(pkt.servers))

		self.status = 'connected'
		return pkt.servers

	@status('connected')
	def selectServer(self, idx):
		''' Selects the game server with the given idx '''
		self.log.info('selecting server %d', idx)
		self.queue(struct.pack('>BH', 0xa0, idx))
		self.send()

		pkt = self.receive(packets.ConnectToGameServerPacket)
		ip = '.'.join(map(str, pkt.ip))
		self.log.info("Connecting to gameserver ip %s, port %s (key %s)", ip, pkt.port, pkt.key)

		# Connect
		self.net.close()
		self.net = net.Network(ip, pkt.port)

		# Send key
		bkey = struct.pack('>I', pkt.key)
		self.queue(bkey)

		# Send login
		self.log.info('logging in')
		po = packets.GameServerLoginPacket()
		po.fill(pkt.key, self.server['user'], self.server['pass'])
		self.queue(po)

		# Flush send buffer
		self.send()

		# From now on, server will use compression
		self.net.compress = True

		# Get features packet
		pkt = self.receive(packets.EnableFeaturesPacket)
		self.features = pkt.features

		# Get character selection
		pkt = self.receive(packets.CharactersPacket)
		self.flags = pkt.flags
		self.locs = pkt.locs

		self.status = 'loggedin'
		return pkt.chars

	@status('loggedin')
	def selectCharacter(self, name, idx):
		''' Login the character with the given name '''
		self.log.info('selecting character #%d %s', idx, name)
		po = packets.LoginCharacterPacket()
		po.fill(name, idx)
		self.queue(po)
		self.send()

		self.status = 'game'

	@status('game')
	def start(self, ai):
		if not isinstance(ai, brain.Brain):
			raise RuntimeError("Unknown brain, expecting a Brain instance, got {}".format(type(ai)))
		self.brain = ai
		super().start()

	@status('game')
	@clientthread
	def run(self):
		''' Called by threading '''
		try:
			self.mainloop()
		except Exception as e:
			type, value, tb = sys.exc_info()
			msg = ''.join(traceback.format_exception(type, value, tb))
			self.log.critical(msg)
			self.brain.event(brain.Event(brain.Event.EVT_CLIENT_CRASH, exception=e))

	@status('game')
	@clientthread
	def mainloop(self):
		''' Starts the endless game loop '''
		self.ping = time.time() + self.PING_INTERVAL

		while True:
			pkt = self.receive(blocking=False)
			self.send()

			# Check if brain is alive
			if not threading.main_thread().is_alive():
				self.log.info("Brain died, terminating")
				break

			# Send ping if needed
			if self.lc and self.ping < time.time():
				po = packets.PingPacket()
				po.fill(0)
				self.queue(po)
				self.ping = time.time() + self.PING_INTERVAL

			# Process packet
			if pkt is None:
				time.sleep(0.01)
			else:
				self.handlePacket(pkt)

	@status('game')
	@clientthread
	def handlePacket(self, pkt):
		''' Handles an incoming packet '''

		if isinstance(pkt, packets.LoginDeniedPacket):
			self.log.error('login denied')
			raise LoginDeniedError(pkt.reason)

		elif isinstance(pkt, packets.PingPacket):
			self.log.debug("Server sent a ping back")

		elif isinstance(pkt, packets.CharLocaleBodyPacket):
			self.handleCharLocaleBodyPacket(pkt)

		elif isinstance(pkt, packets.DrawGamePlayerPacket):
			self.handleDrawGamePlayerPacket(pkt)

		elif isinstance(pkt, packets.DrawObjectPacket):
			self.handleDrawObjectPacket(pkt)

		elif isinstance(pkt, packets.ObjectInfoPacket):
			self.handleObjectInfoPacket(pkt)

		elif isinstance(pkt, packets.UpdatePlayerPacket):
			assert self.lc
			self.objects[pkt.serial].update(pkt)
			self.log.info("Updated mobile: %s", self.objects[pkt.serial])

		elif isinstance(pkt, packets.DeleteObjectPacket):
			assert self.lc
			if pkt.serial in self.objects:
				del self.objects[pkt.serial]
				self.log.info("Object 0x%X went out of sight", pkt.serial)
			else:
				self.log.warn("Server requested to delete 0x%X but i don't know it", pkt.serial)

		elif isinstance(pkt, packets.AddItemToContainerPacket):
			assert self.lc
			if isinstance(self.objects[pkt.container], Container):
				self.objects[pkt.container].addItem(pkt)
			else:
				self.log.warn("Ignoring add item 0x%X to non-container 0x%X", pkt.serial, pkt.container)

		elif isinstance(pkt, packets.AddItemsToContainerPacket):
			assert self.lc
			for it in pkt.items:
				if isinstance(self.objects[it['container']], Container):
					self.objects[it['container']].addItem(it)
				else:
					self.log.warn("Ignoring add item 0x%X to non-container 0x%X", it['serial'], it['container'])

		elif isinstance(pkt, packets.WarModePacket):
			assert self.player.war is None
			self.player.war = pkt.war

		elif isinstance(pkt, packets.AllowAttackPacket):
			assert self.lc
			self.player.target = pkt.serial
			self.log.info("Target set to 0x%X", self.player.target)

		elif isinstance(pkt, packets.UpdateHealthPacket):
			self.handleUpdateVitalPacket(pkt, 'hp', 'maxhp', brain.Event.EVT_HP_CHANGED)

		elif isinstance(pkt, packets.UpdateManaPacket):
			self.handleUpdateVitalPacket(pkt, 'mana', 'maxmana', brain.Event.EVT_MANA_CHANGED)

		elif isinstance(pkt, packets.UpdateStaminaPacket):
			self.handleUpdateVitalPacket(pkt, 'stam', 'maxstam', brain.Event.EVT_STAM_CHANGED)

		elif isinstance(pkt, packets.GeneralInfoPacket):
			self.handleGeneralInfoPacket(pkt)

		elif isinstance(pkt, packets.DrawContainerPacket):
			cont = self.objects[pkt.serial]
			assert isinstance(cont, Item)
			if not isinstance(cont, Container):
				# Upgrade the item to a Container
				cont.upgradeToContainer()

		elif isinstance(pkt, packets.TipWindowPacket):
			assert self.lc
			self.log.info("Received tip: %s", pkt.msg.replace('\r','\n'))

		elif isinstance(pkt, packets.SendSpeechPacket) or isinstance(pkt, packets.UnicodeSpeechPacket):
			speech = Speech(self, pkt)
			if self.lc:
				self.log.info(repr(speech))
			else:
				self.log.warn('EARLY %s', repr(speech))
			self.brain.event(brain.Event(brain.Event.EVT_SPEECH, speech=speech))

		elif isinstance(pkt, packets.TargetCursorPacket):
			assert self.target is None
			self.target = Target(self, pkt)

		elif isinstance(pkt, packets.CharacterAnimationPacket):
			assert self.lc
			# Just check that the object exists
			self.objects[pkt.serial]

		elif isinstance(pkt, packets.LoginCompletePacket):
			assert not self.lc
			assert self.player is not None
			self.lc = True
			# Send some initial info packets      ..
			self.requestSkills()
			self.sendVersion()
			# Original client also sends this now
			# bf 00 0d 00 05 00 00 03 20 01 00 00 a7 - General info 0x05
			self.sendClientType()
			# Original client also sends this now, seems to also send it again later
			# 34 ed ed ed ed 04 00 45 dd f5 - Get Player status something
			self.sendLanguage()
			self.singleClick(self.player)

			# Start the brain
			self.brain.started.set()

		elif isinstance(pkt, packets.MoveAckPacket) or isinstance(pkt, packets.MoveRejectPacket):
			self.handleMovePacket(pkt)

		elif isinstance(pkt, packets.Unk32Packet):
			self.log.warn("Unknown 0x32 packet received")

		elif isinstance(pkt, packets.ControlAnimationPacket):
			assert self.lc
			self.log.info('Ignoring animation packet')

		elif isinstance(pkt, packets.GraphicalEffectPacket):
			assert self.lc
			self.log.info('Graphical effect packet')

		elif isinstance(pkt, packets.PlaySoundPacket):
			assert self.lc
			self.log.info('Ignoring sound packet')

		elif isinstance(pkt, packets.SetWeatherPacket):
			self.log.info('Ignoring weather packet')

		elif isinstance(pkt, packets.OverallLightLevelPacket):
			self.log.info('Ignoring light level packet')

		elif isinstance(pkt, packets.SeasonInfoPacket):
			self.log.info('Ignoring season packet')

		else:
			self.log.warn("Unhandled packet {}".format(pkt.__class__))

	@status('game')
	@clientthread
	def handleCharLocaleBodyPacket(self, pkt):
		assert not self.lc

		assert self.player is None
		self.player = Player(self)

		assert self.player.serial is None
		self.player.serial = pkt.serial
		assert self.player.graphic is None
		self.player.graphic = pkt.bodyType
		assert self.player.x is None
		self.player.x = pkt.x
		assert self.player.y is None
		self.player.y = pkt.y
		assert self.player.z is None
		self.player.z = pkt.z
		assert self.player.facing is None
		self.player.facing = pkt.facing
		assert self.width is None
		self.width = pkt.widthM8 + 8
		assert self.height is None
		self.height = pkt.height

		assert self.player.serial not in self.objects.keys()
		self.objects[self.player.serial] = self.player

		self.log.info("Realm size: %d,%d", self.width, self.height)
		self.log.info("You are 0x%X and your graphic is 0x%X", self.player.serial, self.player.graphic)
		self.log.info("Position: %d,%d,%d facing %d", self.player.x, self.player.y, self.player.z, self.player.facing)

	@status('game')
	@clientthread
	def handleDrawGamePlayerPacket(self, pkt):
		assert self.player.serial == pkt.serial
		assert self.player.graphic == pkt.graphic
		assert self.player.x == pkt.x
		assert self.player.y == pkt.y
		assert self.player.z == pkt.z
		#assert self.player.facing == pkt.direction

		self.player.color = pkt.hue
		self.player.status = pkt.flag

		self.log.info("Your color is %d and your status is 0x%X", self.player.color, self.player.status)

	@status('game')
	@clientthread
	@logincomplete
	def handleDrawObjectPacket(self, pkt):
		if pkt.serial in self.objects.keys():
			self.objects[pkt.serial].update(pkt)
			self.log.info("Refreshed mobile: %s", self.objects[pkt.serial])
		else:
			mob = Mobile(self, pkt)
			self.objects[mob.serial] = mob
			self.log.info("New mobile: %s", mob)
			self.brain.event(brain.Event(brain.Event.EVT_NEW_MOBILE, mobile=mob))
			# Auto single click for new mobiles
			self.singleClick(mob)

	@status('game')
	@clientthread
	@logincomplete
	def handleObjectInfoPacket(self, pkt):
		if pkt.serial in self.objects.keys():
			self.objects[pkt.serial].update(pkt)
			self.log.info("Refresh item: %s", self.objects[pkt.serial])
		else:
			item = Item(self, pkt)
			self.log.info("New item: %s", item)
			self.objects[item.serial] = item

	@status('game')
	@clientthread
	@logincomplete
	def handleUpdateVitalPacket(self, pkt, attrName, maxAttrName, eventId):
		old = getattr(self.player, attrName)
		if self.player.serial == pkt.serial:
			setattr(self.player, maxAttrName, pkt.max)
			setattr(self.player, attrName, pkt.cur)
			self.log.info("My %s: %d/%d", attrName.upper(), pkt.cur, pkt.max)
		else:
			mob = self.objects[pkt.serial]
			setattr(mob, maxAttrName, pkt.max)
			setattr(mob, attrName, pkt.cur)
			self.log.info("0x%X's %s: %d/%d", pkt.serial, attrName.upper(), pkt.cur, pkt.max)
		cur = getattr(self.player, attrName)
		self.brain.event(brain.Event(eventId, old=old, new=cur))

	@status('game')
	@clientthread
	def handleGeneralInfoPacket(self, pkt):
		if pkt.sub == packets.GeneralInfoPacket.SUB_CURSORMAP:
			self.cursor = pkt.cursor
		elif pkt.sub == packets.GeneralInfoPacket.SUB_MAPDIFF:
			pass
		elif pkt.sub == packets.GeneralInfoPacket.SUB_PARTY:
			self.log.info("Ignoring party system data")
		else:
			self.log.warn("Unhandled GeneralInfo subpacket 0x%X", pkt.sub)

	@status('game')
	@clientthread
	@logincomplete
	def handleMovePacket(self, pkt):
		if isinstance(pkt, packets.MoveAckPacket):
			ack = True
		else:
			ack = False

		with self.moveLock:
			# Match first move packet to be ackowledged
			mpkt = self.unmoves.popleft()
			assert pkt.sequence == pkt.sequence

			if not ack:
				# Reset sequence counter after a reject
				self.moveid = -1

		oldx = self.player.x
		oldy = self.player.y
		oldz = self.player.z
		oldfacing = self.player.facing

		if ack:
			# Need to calculate (guess) the new position, since this
			# packets does not cointain position information
			if mpkt.direction == self.player.facing:
				# Moving in front of me, doing one step
				if mpkt.direction == Direction.N:
					self.player.y -= 1
				elif mpkt.direction == Direction.NE:
					self.player.y -= 1
					self.player.x += 1
				elif mpkt.direction == Direction.E:
					self.player.x += 1
				elif mpkt.direction == Direction.SE:
					self.player.y += 1
					self.player.x += 1
				elif mpkt.direction == Direction.S:
					self.player.y += 1
				elif mpkt.direction == Direction.SW:
					self.player.y += 1
					self.player.x -= 1
				elif mpkt.direction == Direction.W:
					self.player.x -= 1
				elif mpkt.direction == Direction.NW:
					self.player.y -= 1
					self.player.x -= 1
			else:
				# Changing direction, just change facing
				self.player.facing = mpkt.direction
		else:
			# The reject packet has the position so it's easier
			self.player.x = pkt.x
			self.player.y = pkt.y
			self.player.z = pkt.z
			self.player.facing = pkt.direction

		self.brain.event(brain.Event(brain.Event.EVT_MOVED,
				oldx=oldx, oldy=oldy, oldz=oldz, oldfacing=oldfacing,
				x=self.player.x, y=self.player.y, z=self.player.z,
				facing=self.player.facing, ack=ack))

		# Handle the notoriety
		if ack and self.player.notoriety != pkt.notoriety:
			old = self.player.notoriety
			self.player.notoriety = pkt.notoriety
			self.brain.event(brain.Event(brain.Event.EVT_NOTORIETY,
					old=old, new=self.player.notoriety))

	@logincomplete
	def sendVersion(self):
		''' Sends client version to server, should not send it twice '''
		po = packets.ClientVersionPacket()
		po.fill(self.VERSION)
		self.queue(po)

	@logincomplete
	def sendLanguage(self):
		''' Sends client lamguage to server, should not send it twice '''
		po = packets.GeneralInfoPacket()
		po.fill(po.SUB_LANG, self.LANG)
		self.queue(po)

	@logincomplete
	def sendClientType(self):
		''' Sends client type flag, should be sent only once at login '''
		po = packets.GeneralInfoPacket()
		po.fill(po.SUB_LOGIN)
		self.queue(po)

	@logincomplete
	def requestSkills(self):
		''' Requests skill info (0x3a packet) '''
		po = packets.GetPlayerStatusPacket()
		po.fill(po.TYP_SKILLS, self.player.serial)
		self.queue(po)

	@logincomplete
	def requestStatus(self):
		''' Requests basic status (0x11 packet) '''
		po = packets.GetPlayerStatusPacket()
		po.fill(po.TYP_BASE, self.player.serial)
		self.queue(po)

	@logincomplete
	def singleClick(self, obj):
		''' Sends a single click for the given object (Item/Mobile or serial) to server '''
		po = packets.SingleClickPacket()
		po.fill(obj if type(obj) == int else obj.serial)
		self.queue(po)

	@logincomplete
	def doubleClick(self, obj):
		''' Sends a single click for the given object (Item/Mobile or serial) to server '''
		po = packets.DoubleClickPacket()
		po.fill(obj if type(obj) == int else obj.serial)
		self.queue(po)

	@logincomplete
	def say(self, text, font=3, color=0):
		''' Say something, in unicode
		@param text string: Any unicode string
		@param font int: Font code, usually 3
		@param colot int: Font color, usually 0
		'''
		po = packets.UnicodeSpeechRequestPacket()
		po.fill(po.TYP_NORMAL, self.LANG, text, color, font)
		self.queue(po)

	@logincomplete
	def move(self, dir):
		''' Request the server to move one step in the given direction
		@param dir Direction: instance od the requested direction or int
		'''
		if isinstance(dir, Direction):
			pass
		elif isinstance(dir, int):
			dir = Direction(dir)
		else:
			raise ValueError('dir must be Direction or int')

		# Holding the lock until the packet is sent to avoid sending packets
		# in the wrong order
		with self.moveLock:
			self.moveid += 1
			if self.moveid > 0xff:
				self.moveid = 1
			po = packets.MoveRequestPacket()
			po.fill(dir.id, self.moveid)
			self.unmoves.append(po)
			self.queue(po)

	@logincomplete
	def waitForTarget(self, timeout=None):
		'''! Waits until a target cursor is requested and return it. If timeout is given, returns after timeout
		@param timeout float: Timeout, in seconds
		@return Target on success, None on timeout
		'''
		self.waitFor(lambda: self.target is not None, timeout)
		return self.target

	def waitFor(self, cond, timeout=None):
		'''! Utility function, waits until a condition is satisfied or until timeout expires
		@return True when consition succeeds, False on timeout
		'''
		wait = 0.0
		nextWarn = 5.0
		while not cond():
			time.sleep(0.01)
			wait += 0.01
			if timeout:
				if wait >= timeout:
					return False
			elif wait >= nextWarn:
				self.log.warn("Waiting for {}...".format(traceback.extract_stack(limit=2)[0]))
				nextWarn = wait + 5.0
		return True

	def queue(self, data):
		''' Puts a packet in the queue to be sent asap '''
		with self.sendqueueLock:
			self.sendqueue.append(data)

	@clientthread
	def send(self):
		''' Sends all packets in the queue '''
		with self.sendqueueLock:
			queue = self.sendqueue
			self.sendqueue = []

		for data in queue:
			self.net.send(data)

	@clientthread
	def receive(self, expect=None, blocking=True):
		'''! Receives next packet from the server
		@param expect Packet/list: If given, throws an exception if packet
		                           is not in the expected list/tuple
		@return Packet
		@throws UnexpectedPacketError
		'''
		pkt = self.net.recv(blocking=blocking)

		if expect and not isinstance(expect, tuple) and not isinstance(expect, list):
			expect = (expect, )

		if expect and pkt.cmd not in [x.cmd for x in expect]:
			err = "Expecting "
			if len(expect) == 1:
				err += "0x%0.2X packet" % expect.cmd
			else:
				err += "one of %s packets" % '/'.join(["0x%0.2X"%x.cmd for x in expect])
			err += ", got 0x%0.2X intead" % pkt.cmd
			raise UnexpectedPacketError(err)

		return pkt



class StatusError(Exception):
	pass


class ThreadError(Exception):
	pass


class UnexpectedPacketError(Exception):
	pass


class LoginDeniedError(Exception):

	def __init__(self, code):
		self.code = code
		if code == 0x00:
			mex = "Incorrect name or password"
		elif code == 0x01:
			mex = "Someone is already using this account"
		elif code == 0x02:
			mex = "Your account has been blocked"
		elif code == 0x03:
			mex = "Your account credentials are invalid"
		elif code == 0x04:
			mex = "Communication problem"
		elif code == 0x05:
			mex = "The IGR concurrency limit has been met"
		elif code == 0x06:
			mex = "The IGR time limit has been met"
		elif code == 0x07:
			mex = "General IGR authentication failure"
		else:
			mex = "Unknown reason {:02X}".format(code)
		super().__init__(mex)


if __name__ == '__main__':
	import argparse

	parser = argparse.ArgumentParser()
	parser.add_argument('ip', help='Server ip')
	parser.add_argument('port', type=int, help='Server port')
	parser.add_argument('user', help='Username')
	parser.add_argument('pwd', help='Password')
	parser.add_argument('srvidx', type=int, help="Gameserver's Index")
	parser.add_argument('charidx', type=int, help="Character's Index")
	parser.add_argument('charname', help="Character's Name")
	parser.add_argument('-v', '--verbose', action='store_true', help='Show debug output')
	args = parser.parse_args()

	logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO)

	c = Client()
	servers = c.connect(args.ip, args.port, args.user, args.pwd)
	chars = c.selectServer(args.srvidx)
	c.selectCharacter(args.charname, args.charidx)
	c.play()
	print('done')
