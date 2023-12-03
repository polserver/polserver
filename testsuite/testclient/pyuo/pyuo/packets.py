#!/usr/bin/env python3

'''
Network packets classes for Python Ultima Online text client
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

import struct
import logging
import ipaddress
import zlib
import sys
import inspect


################################################################################
# Please keep the base classes first then packet classes sorted by packet ID/cmd
################################################################################


class Packet():
  ''' Base class for packets '''

  def __init__(self):
    assert self.cmd
    self.log = logging.getLogger('packet')
    self.validated = False

  def fill(self):
    ''' Fills the packet with the given data, sort of delayed constructor '''
    raise NotImplementedError('This packet cannot be sent')

  def decode(self, buf):
    '''! Parses the data from the given buffer into this packet instance
    @see decodeChild
    @param buf binary: The binary buffer, as received from server
    '''
    self.buf = buf
    self.readCount = 0
    cmd = self.duchar()
    if cmd != self.cmd:
      raise RuntimeError("Invalid data for this packet {} <> {}".format(cmd, self.cmd))

    self.decodeChild()

    # Validate the process
    if self.length != self.readCount:
      self.log.debug(self.__dict__)
      raise RuntimeError("Len mismatch on incomingpacket 0x{:02x} ({} <> {})".format(
          self.cmd, self.length, self.readCount))
    self.validated = True

  def decodeChild(self):
    ''' Derived classes must ovveride this method to do the decoding '''
    raise NotImplementedError('this method must be overridden')

  def encode(self):
    '''! Encodes the data into a buffer and returns it
    @see encodeChild
    @return binary: The binary buffer, ready to be sent to server
    '''
    self.buf = b''
    self.euchar(self.cmd)

    # Used by self.eulen()
    self.lenIdx = None

    self.encodeChild()

    # Replace length if needed
    if self.lenIdx is not None:
      self.buf = self.buf[:self.lenIdx] + struct.pack('>H', len(self.buf)) + self.buf[self.lenIdx+2:]
    del self.lenIdx

    # Validate the process
    if self.length != len(self.buf):
      raise RuntimeError("Len mismatch on outgoing packet 0x{:02x} ({} <> {})".format(
          self.cmd, self.length, len(self.buf)))
    self.validated = True

    return self.buf

  def encodeChild(self):
    ''' Derived classes must ovveride this method to do the decoding '''
    raise NotImplementedError('this method must be overridden')

  # Decode methods -----------------------------------------------------------

  def rpb(self, num):
    ''' Returns the given number of characters from the receive buffer '''
    if num > len(self.buf):
      raise EOFError("Trying to read {} bytes, but only {} left in buffer".format(num, len(self.buf)))
    self.readCount += num
    ret = self.buf[:num]
    self.buf = self.buf[num:]
    return ret

  def duchar(self):
    ''' Returns next unsngned byte from the receive buffer '''
    return struct.unpack('B', self.rpb(1))[0]

  def dschar(self):
    ''' Returns next signed byte from the receive buffer '''
    return struct.unpack('b', self.rpb(1))[0]

  def dushort(self):
    ''' Returns next unsigned short from the receive buffer '''
    return struct.unpack('>H', self.rpb(2))[0]
  
  def dsshort(self):
    ''' Returns next signed short from the receive buffer '''
    return struct.unpack('>h', self.rpb(2))[0]

  def duint(self):
    ''' Returns next unsigned int from the receive buffer '''
    return struct.unpack('>I', self.rpb(4))[0]

  def dstring(self, length):
    ''' Returns next string of the given length from the receive buffer '''
    return self.varStr(self.rpb(length))

  def ducstring(self, length):
    ''' Returns next unicode string of the given length from the receive buffer '''
    if length % 2:
      raise ValueError('Length must be a multiple of 2')
    return self.varUStr(self.rpb(length))

  def dip(self):
    ''' Returns next string ip address from the receive buffer '''
    return struct.unpack('BBBB', self.rpb(4))

  # Encode methods -----------------------------------------------------------

  def eulen(self):
    ''' Special value: will place there an ushort containing packet length '''
    self.lenIdx = len(self.buf)
    self.eushort(0)

  def euchar(self, val):
    ''' Add an unsigned char (byte) to the packet '''
    if not isinstance(val, int):
      raise TypeError("Expected int, got {}".format(type(val)))
    if val < 0 or val > 255:
      raise ValueError("Byte {} out of range".format(val))
    self.buf += struct.pack('B', val)

  def eschar(self, val):
    ''' Add a signed char (byte) to the packet '''
    if not isinstance(val, int):
      raise TypeError("Expected int, got {}".format(type(val)))
    if val < -128 or val > 127:
      raise ValueError("Byte {} out of range".format(val))
    self.buf += struct.pack('b', val)

  def eushort(self, val):
    ''' Adds an unsigned short to the packet '''
    if not isinstance(val, int):
      raise TypeError("Expected int, got {}".format(type(val)))
    if val < 0 or val > 0xffff:
      raise ValueError("UShort {} out of range".format(val))
    self.buf += struct.pack('>H', val)

  def euint(self, val):
    ''' Adds and unsigned int to the packet '''
    if not isinstance(val, int):
      raise TypeError("Expected int, got {}".format(type(val)))
    if val < 0 or val > 0xffffffff:
      raise ValueError("UInt {} out of range".format(val))
    self.buf += struct.pack('>I', val)

  def estring(self, val, length, unicode=False):
    ''' Adds a string to the packet '''
    if not isinstance(val, str):
      raise TypeError("Expected str, got {}".format(type(val)))
    if len(val) > length:
      raise ValueError('String "{}" too long'.format(val))
    self.buf += self.fixStr(val, length, unicode)

  def eip(self, val):
    ''' Adds an ip to the packet '''
    if not isinstance(val, str):
      raise TypeError("Expected str, got {}".format(type(val)))
    self.buf += ipaddress.ip_address(val).packed

  # Utility methods ----------------------------------------------------------

  @staticmethod
  def fixStr(string, length, unicode=False):
    ''' Convert a str to fixed length, return bytes '''
    ##TODO: Better handling on unicode
    enc = string.encode('ascii')
    ret = b''
    for i in range(0,length):
      if unicode:
        ret += b'\x00'
      try:
        ret += bytes([enc[i]])
      except IndexError:
        ret += b'\x00'
    return ret

  @staticmethod
  def varStr(byt):
    ''' Convert bytes into a variable-length string '''
    try:
      dec = byt.decode('utf8')
    except UnicodeDecodeError:
      dec = byt.decode('iso8859-15')
    return Packet.nullTrunc(dec)

  @staticmethod
  def varUStr(byt):
    ''' Convert unicode bytes into a variable-length string '''
    dec = byt.decode('utf_16_be')
    return Packet.nullTrunc(dec)

  @staticmethod
  def nullTrunc(dec):
    ''' Truncates the given string before first null char
    (just like the original client '''
    zero = dec.find('\x00')
    if zero >= 0:
      #for char in dec[zero:]:
      #  if char != '\x00':
      #    logging.warning('Truncating string "%s"', dec)
      #    break
      dec = dec[:zero]
    return dec


class UpdateVitalPacket(Packet):
  ''' Just an utility base class '''

  length = 9

  def decodeChild(self):
    self.serial = self.duint()
    self.max = self.dushort()
    self.cur = self.dushort()


class SerialOnlyPacket(Packet):
  ''' Utility class for a simple packet carrying only a serial '''

  length = 5

  def fill(self, serial):
    '''!
    @param serial int: The object serial
    '''
    self.serial = serial

  def encodeChild(self):
    self.euint(self.serial)

  def decodeChild(self):
    self.serial = self.duint()


################################################################################
# Packets for here on, sorted by ID/cmd
################################################################################


class MoveRequestPacket(Packet):
  ''' Requests movement '''

  cmd = 0x02
  length = 7

  def fill(self, direction, sequence):
    '''!
    @param direction int: The direction code (0-7)
    @param sequence int: The sequence code (0-255)
    '''
    self.direction = direction
    self.sequence = sequence

  def encodeChild(self):
    self.euchar(self.direction)
    self.euchar(self.sequence)
    self.euint(0) #Fastwalk prevention key


class DoubleClickPacket(SerialOnlyPacket):
  ''' Notify server of a doble click on something '''

  cmd = 0x06

class LiftItemPacket(Packet):
  ''' Notify server of a lift on an item '''

  cmd = 0x07
  length = 7

  def fill(self, serial, amount):
    self.type = type
    self.serial = serial
    self.amount = amount

  def encodeChild(self):
    self.euint(self.serial)
    self.eushort(self.amount)

  def decodeChild(self):
    self.serial = self.duint()
    self.amount = self.dushort()

class DropItemPacket(Packet):
  ''' Notify server of a drop on an item '''

  cmd = 0x08
  length = 15

  def fill(self, serial, x, y, z, dropped_on_serial):
    self.type = type
    self.serial = serial
    self.x = x
    self.y = y
    self.z = z
    self.dropped_on_serial = 0xFFFFFFFF if dropped_on_serial == -1 else dropped_on_serial

  def encodeChild(self):
    self.euint(self.serial)
    self.eushort(self.x)
    self.eushort(self.y)
    self.euchar(self.z)
    self.euchar(0) #  Backpack grid index
    self.euint(self.dropped_on_serial)

  def decodeChild(self):
    self.serial = self.duint()
    self.x = self.dushort()
    self.y = self.dushort()
    self.z = self.duchar()
    self.duchar() # Backpack grid index
    self.dropped_on_serial = self.duint()

class SingleClickPacket(SerialOnlyPacket):
  ''' Notify server of a single click on something '''

  cmd = 0x09


class StatusBarInfoPacket(Packet):
  ''' Sends status bar info '''

  cmd = 0x11

  def decodeChild(self):
    self.length = self.dushort()
    self.serial = self.duint()
    self.name = self.dstring(30)
    self.hp = self.dushort()
    self.maxhp = self.dushort()
    self.canrename = self.duchar()
    flag = self.duchar()
    if flag == 0:
      return
    ## Sex and race: 0 = Human Male, 1 = Human female, 2 = Elf Male, 3 = Elf Female
    self.gener = self.duchar()
    self.str = self.dushort()
    self.dex = self.dushort()
    self.int = self.dushort()
    self.stam = self.dushort()
    self.maxstam = self.dushort()
    self.mana = self.dushort()
    self.maxmana = self.dushort()
    self.gold = self.duint()
    self.ar = self.dushort()
    self.weight = self.dushort()
    if flag >= 5:
      self.maxweight = self.dushort()
      ## Race: 1 = Human, 2 = Elf, 3 = Gargoyle
      self.race = self.duchar()
    if flag >= 3:
      self.statcap = self.dushort()
      self.followers = self.duchar()
      self.maxfollowers = self.duchar()
    if flag >= 4:
      self.rfire = self.dushort()
      self.rcold = self.dushort()
      self.rpoison = self.dushort()
      self.renergy = self.dushort()
      self.luck = self.dushort()
      self.mindmg = self.dushort()
      self.maxdmg = self.dushort()
      self.tithing = self.duint()
    if flag >= 6:
      self.hitinc = self.dushort()
      self.swinginc = self.dushort()
      self.dmginc = self.dushort()
      self.lrc = self.dushort()
      self.hpregen = self.dushort()
      self.stamregen = self.dushort()
      self.manaregen = self.dushort()
      self.reflectphysical = self.dushort()
      self.enhancepot = self.dushort()
      self.definc = self.dushort()
      self.spellinc = self.dushort()
      self.fcr = self.dushort()
      self.fc = self.dushort()
      self.lmc = self.dushort()
      self.strinc = self.dushort()
      self.dexinc = self.dushort()
      self.intinc = self.dushort()
      self.hpinc = self.dushort()
      self.staminc = self.dushort()
      self.manainc = self.dushort()
      self.maxhpinc = self.dushort()
      self.maxstaminc = self.dushort()
      self.maxmanainc = self.dushort()


class ObjectInfoPacket(Packet):
  ''' Draws an item '''

  cmd = 0x1a

  def decodeChild(self):
    self.length = self.dushort()
    self.serial = self.duint()
    self.graphic = self.dushort()
    if self.serial & 0x80000000:
      self.count = self.dushort()
    else:
      self.count = None
    if self.graphic & 0x8000:
      self.graphic += self.duchar()
    x = self.dushort()
    y = self.dushort()
    if x & 0x8000:
      self.facing = self.dschar()
    else:
      self.facing = None
    self.z = self.dschar()
    if y & 0x8000:
      self.color = self.dushort()
    else:
      self.color = None
    if y & 0x4000:
      self.flag = self.duchar()
    else:
      self.flag = None
    self.x = x & 0x7fff
    self.y = y & 0x3fff


class CharLocaleBodyPacket(Packet):
  ''' Server giving info about Char, Locale, and Body '''

  cmd = 0x1b
  length = 37

  def decodeChild(self):
    self.serial = self.duint()
    unk = self.duint() # Uknown
    self.bodyType = self.dushort()
    self.x = self.dushort()
    self.y = self.dushort()
    unk = self.duchar() # Unknown
    self.z = self.dschar()
    self.facing = self.dschar()
    unk = self.duint() # Unknown
    unk = self.duint() # Unknown
    unk = self.dschar() # Unknown
    self.widthM8 = self.dushort()
    self.height = self.dushort()
    unk = self.dushort() # Unknown
    unk = self.duint() # Unknown


class SendSpeechPacket(Packet):
  ''' Send(Receive) Speech '''

  cmd = 0x1c

  def decodeChild(self):
    self.length = self.dushort()
    self.serial = self.duint()
    self.model = self.dushort()
    self.type = self.duchar()
    self.color = self.dushort()
    self.font = self.dushort()
    self.name = self.dstring(30)
    self.msg = self.dstring(self.length-44)


class DeleteObjectPacket(SerialOnlyPacket):
  ''' Object went out of sight '''

  cmd = 0x1d


class ControlAnimationPacket(Packet):
  ''' Control Animation '''

  cmd = 0x1e
  length = 4

  def decodeChild(self):
    self.duchar() # Unknown
    self.duchar() # Unknown
    self.duchar() # Unknown


class DrawGamePlayerPacket(Packet):
  ''' Draw game player '''

  cmd = 0x20
  length = 19

  def decodeChild(self):
    self.serial = self.duint()
    self.graphic = self.dushort()
    self.duchar() # unknown
    self.hue = self.dushort()
    self.flag = self.duchar()
    self.x = self.dushort()
    self.y = self.dushort()
    self.dushort() # unknown
    self.direction = self.dschar()
    self.z = self.dschar()


class MoveRejectPacket(Packet):
  ''' Reject move request and update position '''

  cmd = 0x21
  length = 8

  def decodeChild(self):
    self.sequence = self.duchar()
    self.x = self.dushort()
    self.y = self.dushort()
    self.direction = self.dschar()
    self.z = self.dschar()


class MoveAckPacket(Packet):
  ''' Acnowledge move request and update notoriety '''

  cmd = 0x22
  length = 3

  def decodeChild(self):
    self.sequence = self.duchar()
    self.notoriety = self.duchar()


class DrawContainerPacket(Packet):
  ''' Draws a container's gump '''

  cmd = 0x24
  length = 9 # 7090 length

  def decodeChild(self):
    self.serial = self.duint()
    self.gump = self.dushort()
    self.dushort()


class AddItemToContainerPacket(Packet):
  ''' Adds a single item to a container '''

  cmd = 0x25
  length = 21 # 7090 length

  def decodeChild(self):
    self.serial = self.duint()
    self.graphic = self.dushort()
    self.offset = self.duchar()
    self.amount = self.dushort()
    self.x = self.dushort()
    self.y = self.dushort()
    self.slotindex = self.duchar()
    self.container = self.duint()
    self.color = self.dushort()

class MoveItemRejectedPacket(Packet):
  ''' Reject move item (lift or drop) request'''

  cmd = 0x27
  length = 2

  def decodeChild(self):
    self.reason = self.duchar()

class ApproveDropItemPacket(Packet):
  ''' Drop item approved'''

  cmd = 0x29
  length = 1

  def decodeChild(self):
      pass

class MobAttributesPacket(Packet):
  ''' Informs about a Mobile's attributes '''

  cmd = 0x2d
  length = 17

  def decodeChild(self):
    self.serial = self.duint()
    self.hits_max = self.dushort()
    self.hits_current = self.dushort()
    self.mana_max = self.dushort()
    self.mana_current = self.dushort()
    self.stam_max = self.dushort()
    self.stam_current = self.dushort()


class Unk32Packet(Packet):
  ''' Unknown packet '''

  cmd = 0x32
  length = 2

  def decodeChild(self):
    self.duchar()


class GetPlayerStatusPacket(Packet):
  ''' Requests player status '''

  ## God client
  TYP_GOD  = 0x00
  ## Basic Status (Packet 0x11)
  TYP_BASE = 0x04
  ## Skill info (Packet 0x3a)
  TYP_SKILLS = 0x05

  cmd = 0x34
  length = 10

  def fill(self, type, serial):
    '''
    @param type int: What to request (see TYP_ constants)
    @param serial: int: The character's serial
    '''
    self.type = type
    self.serial = serial

  def encodeChild(self):
    self.euint(0xedededed) #Pattern (unknown)
    self.euchar(self.type)
    self.euint(self.serial)


class SendSkillsPacket(Packet):
  ''' When received contains a single skill or full list of skills
  When sent by client, sets skill lock for a single skill '''

  cmd = 0x3a

  def decodeChild(self):
    self.length = self.dushort()
    typ = self.duchar() # 0x00 full list, 0xff single skill, 0x02 full with caps, 0xdf single with caps
    self.skills = {}
    while True:
      try:
        id = self.dushort()
      except EOFError:
        break
      else:
        if not id:
          break
      assert id not in self.skills
      self.skills[id] = {
        'id': id,
        'val': self.dushort(), # Current value, in tenths
        'base': self.dushort(), # Base value, in tenths
        'lock': self.duchar(), # Lock status 0 = up, 1 = down, 2 =locked
        'cap': self.dushort() if typ == 0x02 or typ == 0xdf else None
      }


class AddItemsToContainerPacket(Packet):
  ''' Adds multiple items to a container '''

  cmd = 0x3c

  def decodeChild(self):
    self.length = self.dushort()
    itemNum = self.dushort()
    self.items = []
    for i in range(0, itemNum):
      self.items.append({
        'serial': self.duint(),
        'graphic': self.dushort(),
        'unknown': self.duchar(),
        'amount': self.dushort(),
        'x': self.dushort(),
        'y': self.dushort(),
        'slot': self.duchar(), #7090 version
        'container': self.duint(),
        'color': self.dushort(),
      })


class OverallLightLevelPacket(Packet):
  ''' Overall Light Level '''

  cmd = 0x4f
  length = 2

  def decodeChild(self):
    self.level = self.duchar()


class PlaySoundPacket(Packet):
  ''' Play Sound Effect '''

  cmd = 0x54
  length = 12

  def decodeChild(self):
    self.mode = self.duchar()
    self.model = self.dushort()
    self.dushort() # unknown
    self.x = self.dushort()
    self.y = self.dushort()
    self.z = self.dushort()


class LoginCompletePacket(Packet):
  ''' Login Complete '''

  cmd = 0x55
  length = 1

  def decodeChild(self):
    pass


class LoginCharacterPacket(Packet):
  ''' Selects the character during login '''

  cmd = 0x5d
  length = 73

  def fill(self, name, idx):
    '''!
    @param name string: The character name
    @param int: The character slot index
    '''
    self.name = name
    self.idx = idx

  def encodeChild(self):
    self.euint(0xedededed) #Pattern1
    self.estring(self.name, 30)
    self.eushort(0x0000)   #unknown0
    self.euint(0x00000000) #clientflag
    self.euint(0x00000000) #unknown1
    self.euint(0x0000001d) #login count
    self.euint(0x00000000) # unknown2
    self.euint(0x00000000) # unknown2
    self.euint(0x00000000) # unknown2
    self.euint(0x00000000) # unknown2
    self.euint(self.idx)
    self.eip('127.0.0.1')


class SetWeatherPacket(Packet):
  ''' Sets Weather '''

  cmd = 0x65
  length = 4

  def decodeChild(self):
    self.type = self.duchar()
    self.num = self.duchar()
    self.temp = self.duchar()


class TargetCursorPacket(Packet):
  ''' Requesting/Answering a target '''

  # Constants for what
  OBJECT = 0
  LOCATION = 1

  # Constants for type
  NEUTRAL = 0
  HARMFUL = 1
  HELPFUL = 2

  cmd = 0x6c
  length = 19

  def fill(self, what, id, type, serial, x=0, y=0, z=0, graphic=0):
    '''!
    @param what int: what to target, see constants
    @param id int: The id of the target that we are answering to
    @param type int: The target type, see constants
    @param serial int: Serial of the targetted object
    @param x int: X coordinate of the targetted location
    @param y int: Y coordinate of the targetted location
    @param z int: Z coordinate of the targetted location
    @param graphic int: Graphic of the targetted static tile
    '''
    self.what = what
    self.id = id
    self.type = type
    self.serial = serial
    self.x = x
    self.y = y
    self.z = z
    self.graphic = graphic

  def encodeChild(self):
    self.euchar(self.what)
    self.euint(self.id)
    self.euchar(self.type)
    self.euint(self.serial)
    self.eushort(self.x)
    self.eushort(self.y)
    self.euchar(0) # unknown
    self.eschar(self.z)
    self.eushort(self.graphic)

  def decodeChild(self):
    ## 0 = object, 1 = location
    self.what = self.duchar()
    self.id = self.duint()
    ## 0 = Neutral, 1 = Harmful, 2 = Helpful, 3 = Cancel (server sent)
    self.type = self.duchar()

    # Following data ignored when sent my server
    self.duint() # Clicked on
    self.dushort() # x
    self.dushort() # y
    self.duchar() # unknown
    self.dschar() # z
    self.dushort() # graphic (if static tile)


class PlayMidiPacket(Packet):
  ''' Play Midi Music '''

  cmd = 0x6d
  length = 3

  def decodeChild(self):
    self.music = self.dushort()


class CharacterAnimationPacket(Packet):
  ''' Play an animation for a character '''

  cmd = 0x6e
  length = 14

  def decodeChild(self):
    self.serial = self.duint()
    self.action = self.dushort()
    self.duchar() # unknown
    self.frames = self.duchar()
    self.repeat = self.dushort()
    self.backwards = self.duchar()
    self.repeat = self.duchar()
    self.delay = self.duchar()


class GraphicalEffectPacket(Packet):
  ''' Play a generic graphical effect '''

  cmd = 0x70
  length = 28

  def decodeChild(self):
    self.direction = self.duchar()
    self.serial = self.duint()
    self.target = self.duint()
    self.graphic = self.dushort()
    self.x = self.dushort()
    self.y = self.dushort()
    self.z = self.dschar()
    self.tx = self.dushort()
    self.ty = self.dushort()
    self.tz = self.dschar()
    self.speed = self.duchar()
    self.duration = self.duchar()
    self.dushort() # Unknown
    self.adjust = self.duchar()
    self.explode = self.duchar()


class WarModePacket(Packet):
  ''' Request/Set war mode '''

  cmd = 0x72
  length = 5

  def decodeChild(self):
    self.war = self.duchar()
    self.duchar() # unknown
    self.duchar() # unknown
    self.duchar() # unknown


class PingPacket(Packet):
  ''' Ping request/reply '''

  cmd = 0x73
  length = 2

  def fill(self, seq):
    '''!
    @param seq int: Sequence number
    '''
    self.seq = seq

  def encodeChild(self):
    self.euchar(self.seq)

  def decodeChild(self):
    self.seq = self.duchar()

class NewSubServerPacket(Packet):
  ''' new subserver '''

  cmd = 0x76
  length = 16

  def decodeChild(self):
    self.x = self.dushort()
    self.y = self.dushort()
    self.z = self.dushort()
    self.unk = self.duchar()
    self.serverx = self.dushort()
    self.servery = self.dushort()
    self.serverwidth = self.dushort()
    self.serverheight = self.dushort()

class UpdatePlayerPacket(Packet):
  ''' Updates a mobile '''

  cmd = 0x77
  length = 17

  def decodeChild(self):
    self.serial = self.duint()
    self.graphic = self.dushort()
    self.x = self.dushort()
    self.y = self.dushort()
    self.z = self.dschar()
    self.facing = self.dschar()
    self.color = self.dushort()
    self.flag = self.duchar()
    self.notoriety = self.duchar()


class DrawObjectPacket(Packet):
  ''' Draws a mobile '''

  cmd = 0x78

  def decodeChild(self):
    self.length = self.dushort()
    self.serial = self.duint()
    self.graphic = self.dushort()
    self.x = self.dushort()
    self.y = self.dushort()
    self.z = self.dschar()
    self.facing = self.dschar()
    self.color = self.dushort()
    self.flag = self.duchar()
    self.notoriety = self.duchar()
    self.equip = []
    while True:
      serial = self.duint()
      if not serial:
        break
      graphic = self.dushort()
      layer = self.duchar()
      if graphic & 0x8000:
        color = self.dushort()
      else:
        color = 0
      self.equip.append({
        'serial': serial,
        'graphic': graphic,
        'layer': layer,
        'color': color,
      })
#    if not len(self.equip):
#      self.duchar() # unused/closing


class SeedPacket(Packet):
  ''' login seed to server '''

  cmd = 0xEF
  length = 21

  def fill(self, ip,version):
    '''!
    @param version string: The version
    '''

    self.ip = ip
    self.version = [int(_) for _ in version.split('.')]

  def encodeChild(self):
    self.euchar(self.ip[0])
    self.euchar(self.ip[1])
    self.euchar(self.ip[2])
    self.euchar(self.ip[3])
    self.euint(self.version[0])
    self.euint(self.version[1])
    self.euint(self.version[2])
    self.euint(self.version[3])

class LoginRequestPacket(Packet):
  ''' Login request to server '''

  cmd = 0x80
  length = 62

  def fill(self, account, password, nlk=0):
    '''!
    @param account string: The username
    @param password string: The password
    @param nlk byte: NextLoginKey value from uo.cfg on client machine
    '''
    self.account = account
    self.password = password
    self.nlk = nlk

  def encodeChild(self):
    self.estring(self.account, 30)
    self.estring(self.password, 30)
    self.euchar(self.nlk)


class LoginDeniedPacket(Packet):
  ''' Login Denied '''

  cmd = 0x82
  length = 2

  def decodeChild(self):
    self.reason = self.duchar()


class ConnectToGameServerPacket(Packet):
  ''' Login server is requesting to connect to the game server '''

  cmd = 0x8c
  length = 11

  def decodeChild(self):
    self.ip = self.dip()
    self.port = self.dushort()
    self.key = self.duint()


class GameServerLoginPacket(Packet):
  ''' Login request to game server '''

  cmd = 0x91
  length = 65

  def fill(self, key, account, password):
    '''!
    @param key: The key used
    @param account string: The username
    @param password string: The password
    '''
    self.key = key
    self.account = account
    self.password = password

  def encodeChild(self):
    self.euint(self.key)
    self.estring(self.account, 30)
    self.estring(self.password, 30)


class UpdateHealthPacket(UpdateVitalPacket):
  ''' Updates current health '''

  cmd = 0xa1


class UpdateManaPacket(UpdateVitalPacket):
  ''' Updates current mana '''

  cmd = 0xa2


class UpdateStaminaPacket(UpdateVitalPacket):
  ''' Updates current stamina '''

  cmd = 0xa3


class TipWindowPacket(Packet):
  ''' Tip/Notice Window '''

  cmd = 0xa6

  def decodeChild(self):
    self.length = self.dushort()
    self.flag = self.duchar()
    self.tipid = self.duint()
    msgSize = self.dushort()
    self.msg = self.dstring(msgSize)


class ServerListPacket(Packet):
  ''' Receive server list '''

  cmd = 0xa8

  def decodeChild(self):
    self.length = self.dushort()
    self.flag = self.duchar()
    self.numServers = self.dushort()
    self.servers = []
    for i in range(0, self.numServers):
      self.servers.append({
        'idx': self.dushort(),
        'name': self.dstring(32),
        'full': self.duchar(),
        'tz': self.duchar(),
        'ip': self.dip(),
      })


class CharactersPacket(Packet):
  ''' Gets lists of characters and starting locations from server '''

  cmd = 0xa9

  def decodeChild(self):
    self.length = self.dushort()
    self.numChars = self.duchar()
    self.chars = []
    for i in range(0, self.numChars):
      self.chars.append({
        'name': self.dstring(30),
        'pass': self.dstring(30),
      })
    self.numLocs = self.duchar()
    self.locs = []
    for i in range(0, self.numLocs):
      self.locs.append({
        'idx': self.duchar(),
        'name': self.dstring(31),
        'area': self.dstring(31),
      })
    self.flags = self.duint()


class AllowAttackPacket(SerialOnlyPacket):
  ''' Allow/Refuse attack '''

  cmd = 0xaa


class UnicodeSpeechRequestPacket(Packet):
  ''' Unicode speech request packet '''

  ## Normal speech
  TYP_NORMAL = 0x00
  ## Broadcast/System
  TYP_BROADCAST = 0x01
  ## Emote
  TYP_EMOTE = 0x02
  ## System/Lower corner
  TYP_SYSTEM = 0x06
  ## Message/Corner with name
  TYP_MESSAGE = 0x07
  ## Whisper
  TYP_WHISPER = 0x08
  ## Yell
  TYP_YELL = 0x09
  ## Spell
  TYP_SPELL =0x0a
  ## Guild Chat
  TYP_GUILD = 0xd
  ## Alliance Chat
  TYP_ALLIANCE = 0x0e
  ## Command Prompts
  TYP_COMMAND = 0x0f
  ## Encoded with speech tokens
  TYP_ENCODED = 0xc0

  cmd = 0xad

  def fill(self, type, lang, text, color, font, tokens=None):
    '''!
    @param type int: Speech type, see TYP_ constants
    @param lang string: Three letter language code
    @param text string: What to say
    @param color int: Color code
    @param font int: Font code
    @param tokens list of ints: speech.mul ids
    '''
    self.type = type
    self.lang = lang
    self.text = text
    self.color = color
    self.font = font
    self.tokens = tokens

    self.length = 1 + 2 + 1 + 2 + 2 + 4
    if tokens:
      token_byte_length = ((((1 + len(tokens)) * 12) + 7) & (-8)) / 8
      self.length = self.length + token_byte_length + len(self.text)+1
    else:
      self.length = self.length + len(self.text)*2+2

  def encodeChild(self):
    self.eulen()
    if self.tokens:
      self.euchar(self.type | UnicodeSpeechRequestPacket.TYP_ENCODED)
    else:
      self.euchar(self.type)
    self.eushort(self.color)
    self.eushort(self.font)
    assert len(self.lang) == 3
    self.estring(self.lang, 4)
    if self.tokens:
      self.encodeTokens()
    else:
      self.estring(self.text, len(self.text) + 1, True)

  def encodeTokens(self):
    code_bytes = []
    length = len(self.tokens)
    code_bytes.append(length >> 4)
    num3 = length & 15
    flag = False
    index = 0
    while index < length:
      keyword_id = self.tokens[index]
      if flag:
        code_bytes.append(keyword_id >> 4)
        num3 = keyword_id & 15
      else:
        code_bytes.append(((num3 << 4) | ((keyword_id >> 8) & 15)))
        code_bytes.append(keyword_id)
      index = index + 1
      flag = not flag
    if not flag:
      code_bytes.append(num3 << 4)

    for code_byte in code_bytes:
      self.euchar(code_byte)
    self.estring(self.text, len(self.text) + 1)

class UnicodeSpeechPacket(Packet):
  ''' Receive an unicode speech '''

  cmd = 0xae

  def decodeChild(self):
    self.length = self.dushort()
    self.serial = self.duint()
    self.model = self.dushort()
    self.type = self.duchar()
    self.color = self.dushort()
    self.font = self.dushort()
    self.lang = self.dstring(4)
    self.name = self.dstring(30)
    self.msg = self.ducstring(self.length-48)


class SendGumpDialogPacket(Packet):
  ''' Receiving a gump from the server '''

  cmd = 0xb0

  def decodeChild(self):
    self.length = self.dushort()
    self.serial = self.duint()
    self.gumpid = self.duint()
    self.x = self.duint()
    self.y = self.duint()
    cmdLen = self.dushort()
    self.commands = self.dstring(cmdLen)
    textLines = self.dushort()
    self.texts = []
    for i in range(0, textLines):
      tlen = self.dushort() # In unicode 2-bytes chars
      self.texts.append(self.ducstring(tlen*2))
    self.duchar() # Trailing byte? TODO: check this


class EnableFeaturesPacket(Packet):
  ''' Used to enable client features '''

  cmd = 0xb9
  length = 5

  def decodeChild(self):
    self.features =self.duint()


class SeasonInfoPacket(Packet):
  ''' Seasonal Information Packet '''

  cmd = 0xbc
  length = 3

  def decodeChild(self):
    self.flag = self.duchar()
    self.sound = self.duchar()


class ClientVersionPacket(Packet):
  ''' Send client version to the server '''

  cmd = 0xbd

  def fill(self, version):
    '''!
    @param version string: The client, version, as string
    '''
    self.version = version
    self.length = 1 + 2 + len(version)+1

  def encodeChild(self):
    self.eulen()
    self.estring(self.version, len(self.version)+1)


class GeneralInfoPacket(Packet):
  ''' This packet does a lot of different things, based on subcommand '''

  ## Initialize fastwalk prevention
  SUB_FASTWALK = 0x01
  ## Add a key to fastwalk stack
  SUB_ADDFWKEY = 0x02
  ## Close generic gump
  SUB_CLOSEGUMP = 0x04
  ## Screen size
  SUB_SCREENSIZE = 0x05
  ## Party system
  SUB_PARTY = 0x06
  ## Set cursor hue / set map
  SUB_CURSORMAP = 0x08
  ## Wrestling stun
  SUB_STUN = 0x0a
  ## CLient language
  SUB_LANG = 0x0b
  ## Closed status gump
  SUB_CLOSESTATUS = 0x0c
  ## 3D Action
  SUB_3DACT = 0x0e
  ## MegaCliLoc
  SUB_MEGACLILOC = 0x10
  ## Send House Revision State
  SUB_HOUSE_REV = 0x1d
  ## Login
  SUB_LOGIN = 0x1f
  ## Enable map-diff files
  SUB_MAPDIFF = 0x18

  cmd = 0xbf

  def fill(self, sub, *args):
    '''!
    @param sub int: The subcommand, see SUB_ constants
    @param *args: Variable number of arguments, depending on sub:
                  - SUB_LOGIN: no more arguments needed
                  - SUB_LANG:
                    - lang string: The language name
    '''
    self.sub = sub

    def checkArgLen(expLen):
      if len(args) != expLen:
        raise TypeError("Subcommand {:02x} takes {} positional argument(s) " + \
            "but {} were given".format(self.sub, expLen, len(args)))

    if self.sub == self.SUB_LOGIN:
      checkArgLen(0)
      self.length = 5 + 2

    elif self.sub == self.SUB_LANG:
      checkArgLen(1)
      self.lang = args[0]
      self.length = 5 + len(self.lang)+1

    else:
      raise NotImplementedError('Subcommand {:02x} not implemented to send'.format(self.sub))

  def encodeChild(self):
    self.eulen()
    self.eushort(self.sub)

    if self.sub == self.SUB_LOGIN:
      self.eushort(0x0000)

    elif self.sub == self.SUB_LANG:
      self.estring(self.lang, len(self.lang)+1)

    else:
      raise NotImplementedError('Subcommand {:02x} not implemented yet'.format(self.sub))

  def decodeChild(self):
    self.length = self.dushort()
    self.sub = self.dushort()

    if self.sub == self.SUB_FASTWALK:
      self.keys = []
      for i in range(0, 6):
        self.keys.append(self.duint())

    elif self.sub == self.SUB_ADDFWKEY:
      self.key = self.duint()

    elif self.sub == self.SUB_CLOSEGUMP:
      self.gumpid = self.duint()
      self.buttonid = self.duint()

    elif self.sub == self.SUB_SCREENSIZE:
      unk = self.dushort()
      self.x = self.dushort()
      self.y = self.dushort()
      unk = self.dushort()

    elif self.sub == self.SUB_PARTY:
      self.data = self.rpb(len(self.buf))

    elif self.sub == self.SUB_CURSORMAP:
      self.cursor = self.duchar()

    elif self.sub == self.SUB_STUN:
      raise NotImplementedError("This should no longer be used")

    elif self.sub == self.SUB_LANG:
      self.lang = self.dstring(3)

    elif self.sub == self.SUB_CLOSESTATUS:
      self.serial = self.duint()

    elif self.sub == self.SUB_3DACT:
      self.animation = self.duint()

    elif self.sub == self.SUB_MAPDIFF:
      mapNum = self.duint()
      self.maps = []
      for i in range(0, mapNum):
        self.maps.append({
          'mpatches': self.duint(),
          'spatches': self.duint(),
        })

    elif self.sub == self.SUB_MEGACLILOC:
      self.serial = self.duint()
      self.revision = self.duint()

    elif self.sub == self.SUB_HOUSE_REV:
      self.serial = self.duint()
      self.rev = self.duint()

    else:
      raise NotImplementedError("Subcommand 0x%0.2X not implemented yet." % self.sub)


class ClilocMsgPacket(Packet):
  ''' A CliLoc message (predefined messages with an unique ID) '''

  cmd = 0xc1

  def decodeChild(self):
    self.length = self.dushort()
    self.id = self.duint()
    self.body = self.dushort()
    self.type = self.duchar()
    self.hue = self.dushort()
    self.font = self.dushort()
    self.msg = self.duint()
    self.speaker_name = self.dstring(30)
    self.unicode_string = self.rpb(self.length-48)


class MegaClilocRevPacket(Packet):
  ''' SE Introduced Revision '''

  cmd = 0xdc
  length = 9

  def decodeChild(self):
    self.serial = self.duint()
    self.revision = self.duint()


class CompressedGumpPacket(Packet):
  ''' Receiving a compressed gump from the server '''

  cmd = 0xdd

  def decodeChild(self):
    self.length = self.dushort()
    self.serial = self.duint()
    self.gumpid = self.duint()
    self.x = self.duint()
    self.y = self.duint()
    cLen = self.duint()
    dLen = self.duint()
    commands = zlib.decompress(self.rpb(cLen-4))
    assert len(commands) == dLen
    self.commands=commands[:-1].decode().split(' }{ ')
    if len(self.commands):
      self.commands[0]=self.commands[0].strip('{ ')
      self.commands[-1]=self.commands[-1].strip(' }')

    textLines = self.duint()
    ctxtLen = self.duint()
    dtxtLen = self.duint()
    texts = zlib.decompress(self.rpb(ctxtLen-4))
    assert len(texts) == dtxtLen
    self.texts=[]
    for i in range(textLines):
        tlen=struct.unpack('>H',texts[:2])[0]
        self.texts.append(texts[2:tlen*2+2].decode('utf_16_be'))
        texts=texts[tlen*2+2:]
    #self.duchar() # Trailing byte?

class CloseGumpResponsePacket(Packet):
  ''' close gump '''

  cmd = 0xb1

  def fill(self,serial,gumpid):
    self.serial=serial
    self.gumpid =gumpid
    self.length=23

  def encodeChild(self):
    self.eulen()
    self.euint(self.serial)
    self.euint(self.gumpid)
    self.euint(0) #button id
    self.euint(0) #switch count
    self.euint(0) #string count


class NewObjectInfoPacket(Packet):
  ''' Draws an item '''

  cmd = 0xf3
  length = 26 #7090 version

  def decodeChild(self):
    self.dushort()
    self.duchar()
    self.serial = self.duint()
    self.graphic = self.dushort()
    self.duchar()
    self.count = self.dushort()
    self.count = self.dushort()
    x = self.dushort()
    y = self.dushort()
    self.z = self.dschar()
    self.facing = self.dschar()
    self.color = self.dushort()
    self.flag = self.duchar()
    self.dushort()
    self.x = x & 0x7fff
    self.y = y & 0x3fff

class SmoothBoatPacket(Packet):
  ''' boat/item update '''

  cmd = 0xf6

  def decodeChild(self):
    self.length = self.dushort()
    self.serial = self.duint()
    self.speed = self.duchar()
    self.dir = self.duchar()
    self.facing = self.duchar()
    self.x = self.dushort()
    self.y = self.dushort()
    self.z = self.dsshort()
    self.count = self.dushort()
    self.objs=[]
    for i in range(self.count):
      try:
        self.objs.append({
         'serial':self.duint(),
         'x':self.dushort(),
         'y':self.dushort(),
         'z':self.dsshort(),
        })
      except Exception as e:
        self.log.error('failed to read obj {} of {} pktlen {}'.format(i,self.count,self.length))
        break


class HealthBarStatusUpdate(Packet):
  ''' Health bar status update (KR) '''

  cmd = 0x17

  def decodeChild(self):
    self.length = self.dushort()
    self.serial = self.duint()
    self.dushort()
    self.color = self.dushort()
    self.flags = self.duchar()

################################################################################
# Build packet list when this module is imported, must stay at the end
################################################################################


''' Builds list of currectly defined packets, as a dict of classes sorted by ID '''
classes = {}
for name, obj in inspect.getmembers(sys.modules[__name__]):
  if inspect.isclass(obj) and hasattr(obj, 'cmd'):
    cmd = obj.cmd
    if cmd in classes.keys():
      raise RuntimeError("Duplicate packet 0x{:02x}".format(cmd))
    classes[cmd] = obj
