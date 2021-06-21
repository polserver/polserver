#!/usr/bin/env python3

'''
Network classes for Python Ultima Online text client
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

import time
import socket
import struct
import ipaddress
import logging

from . import packets


class Network:
  ''' Network handler '''

  ## Decompression Tree, internal usage. Thanks to UOXNA project
  DECOMPRESSION_TREE = (
    # leaf0, leaf1, #node
    (2,1), (4,3), (0,5), (7,6), (9,8), #0-4
    (11,10), (13,12), (14,-256), (16,15), (18,17), #5-9
    (20,19), (22,21), (23,-1), (25,24), (27,26), #10-14
    (29,28), (31,30), (33,32), (35,34), (37,36), #15-19
    (39,38), (-64,40), (42,41), (44,43), (45,-6), #20-24
    (47,46), (49,48), (51,50), (52,-119), (53,-32), #25-29
    (-14,54), (-5,55), (57,56), (59,58), (-2,60), #30-34
    (62,61), (64,63), (66,65), (68,67), (70,69), #35-39
    (72,71), (73,-51), (75,74), (77,76), (-111,-101), #40-44
    (-97,-4), (79,78), (80,-110), (-116,81), (83,82), #45-49
    (-255,84), (86,85), (88,87), (90,89), (-10,-15), #50-54
    (92,91), (93,-21), (94,-117), (96,95), (98,97), #55-59
    (100,99), (101,-114), (102,-105), (103,-26), (105,104), #60-64
    (107,106), (109,108), (111,110), (-3,112), (-7,113), #65-69
    (-131,114), (-144,115), (117,116), (118,-20), (120,119), #70-74
    (122,121), (124,123), (126,125), (128,127), (-100,129), #75-79
    (-8,130), (132,131), (134,133), (135,-120), (-31,136), #80-84
    (138,137), (-234,-109), (140,139), (142,141), (144,143), #85-89
    (145,-112), (146,-19), (148,147), (-66,149), (-145,150), #90-94
    (-65,-13), (152,151), (154,153), (155,-30), (157,156), #95-99
    (158,-99), (160,159), (162,161), (163,-23), (164,-29), #100-104
    (165,-11), (-115,166), (168,167), (170,169), (171,-16), #105-109
    (172,-34), (-132,173), (-108,174), (-22,175), (-9,176), #110-114
    (-84,177), (-37,-17), (178,-28), (180,179), (182,181), #115-119
    (184,183), (186,185), (-104,187), (-78,188), (-61,189), #120-124
    (-178,-79), (-134,-59), (-25,190), (-18,-83), (-57,191), #125-129
    (192,-67), (193,-98), (-68,-12), (195,194), (-128,-55), #130-134
    (-50,-24), (196,-70), (-33,-94), (-129,197), (198,-74), #135-139
    (199,-82), (-87,-56), (200,-44), (201,-248), (-81,-163), #140-144
    (-123,-52), (-113,202), (-41,-48), (-40,-122), (-90,203), #145-149
    (204,-54), (-192,-86), (206,205), (-130,207), (208,-53), #150-154
    (-45,-133), (210,209), (-91,211), (213,212), (-88,-106), #155-159
    (215,214), (217,216), (-49,218), (220,219), (222,221), #160-164
    (224,223),(226,225), (-102,227), (228,-160), (229,-46), #165-169
    (230,-127), (231,-103), (233,232), (234,-60), (-76,235), #170-174
    (-121,236), (-73,237), (238,-149), (-107,239), (240,-35), #175-179
    (-27,-71), (241,-69), (-77,-89), (-118,-62), (-85,-75), #180-184
    (-58,-72), (-80,-63), (-42,242), (-157,-150), (-236,-139), #185-189
    (-243,-126), (-214,-142), (-206,-138), (-146,-240), (-147,-204), #190-194
    (-201,-152), (-207,-227), (-209,-154), (-254,-153), (-156,-176), #195-199
    (-210,-165), (-185,-172), (-170,-195), (-211,-232), (-239,-219), #200-204
    (-177,-200), (-212,-175), (-143,-244), (-171,-246), (-221,-203), #205-209
    (-181,-202), (-250,-173), (-164,-184), (-218,-193), (-220,-199), #210-214
    (-249,-190), (-217,-230), (-216,-169), (-197,-191), (243,-47), #215-219
    (245,244), (247,246), (-159,-148), (249,248), (-93,-92), #220-224
    (-225,-96), (-95,-151), (251,250), (252,-241), (-36,-161), #225-229
    (254,253), (-39,-135), (-124,-187), (-251,255), (-238,-162), #230-234
    (-38,-242), (-125,-43), (-253,-215), (-208,-140), (-235,-137), #235-239
    (-237,-158), (-205,-136), (-141,-155), (-229,-228), (-168,-213), #240-244
    (-194,-224), (-226,-196), (-233,-183), (-167,-231), (-189,-174), #245-249
    (-166,-252), (-222,-198), (-179,-188), (-182,-223), (-186,-180), #250-254
    (-247,-245) #255
  )

  def __init__(self, ip, port):
    '''! Connects to the socket
      @param ip IPv4Address: the IP object, from the ipaddress module
      @param port int: the port
    '''
    ## Logger, for internal usage
    self.log = logging.getLogger('net')
    ## Socket connection, for internal usage
    self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self.sock.connect((str(ip), port))
    ## Buffer, for internal usage
    self.buf = b''
    ## Wether to use compression or not
    self.compress = False

  def close(self):
    ''' Disconnects, makes this object unusable '''
    self.sock.close()

  def send(self, data):
    ''' Sends a packet or raw binary data '''
    if isinstance(data, packets.Packet):
      raw = data.encode()
      assert data.validated
    elif isinstance(data, bytes):
      raw = data
    else:
      raise ValueError('Expecting Packet or bytes')

    self.log.debug('-> 0x%0.2X, %d bytes\n"%s"', raw[0], len(raw), raw)
    self.sock.send(raw)

  def recv(self, force=False, blocking=True):
    '''! Reads next packet from the server
    Always returns a full packet (waiting for full packet is always blocking)

    @param force bool: Force wait, used internally
    @param blocking bool: Set blocking mode
    '''

    self.sock.setblocking(blocking)

    # Wait for a full packet
    if len(self.buf) < 1 or force:
      try:
        data = self.sock.recv(4096)
      except socket.error:
        if not blocking:
          return None
        else:
          raise

      if not len(data):
        raise RuntimeError("Disconnected");

      self.buf += data

    if self.compress:
      try:
        raw, size = self.decompress(self.buf)
      except NoFullPacketError:
        # Not enough data to make a full packet. Try again
        self.log.debug("No full packet. Waiting... (%d bytes in buffer)", len(self.buf))
        return self.recv(True)
    else:
      raw = self.buf
      size = len(self.buf)

    if not raw:
      raise NotImplementedError()

    cinfo = '{} compressed'.format(size) if self.compress else 'not compressed'
    self.log.debug('<- 0x%0.2X, %d bytes, %s\n"%s"', raw[0], len(raw), cinfo, raw)

    # Creates and instance of the packet from the buffer
    cmd = raw[0]
    if cmd not in packets.classes.keys():
      raise NotImplementedError(
          "Unknown packet 0x%0.2X, %d bytes\n%s" % (cmd, len(raw), raw))
    pktClass = packets.classes[cmd]
    pkt = pktClass()
    pkt.decode(raw)
    assert pkt.validated
    assert pkt.length == len(raw), hex(cmd)+" "+str(pkt.length)+" != "+ str(len(raw))

    # Remove the processed packet from the buffer the buffer
    self.buf = self.buf[size:]

    return pkt

  def decompress(self, buf):
    '''! Internal usage, decompress a packet (thanks to UltimaXNA project
    @return tuple (decompressed, compressed_size)
    '''
    node = 0
    leaf = 0
    leafVal = 0
    bitNum = 8
    srcPos = 0
    dest = b''

    while srcPos < len(buf):
      # Gets next bit
      leaf = ( buf[srcPos] >> ( bitNum - 1 ) ) & 1
      # Look into decompression table
      leafVal = self.DECOMPRESSION_TREE[node][leaf]

      # all numbers below 1 (0..-256) are codewords
      # if the halt codeword has been found, skip this byte
      if leafVal == -256:
        return ( dest, srcPos + 1)
      elif leafVal < 1:
        dest += bytes([0 - leafVal])
        leafVal = 0

      # Go for next bit, if its the end of the byte, go to the next byte
      bitNum -= 1
      node = leafVal
      if bitNum < 1:
        bitNum = 8;
        srcPos += 1

    raise NoFullPacketError("No full packet could be read")


class NoFullPacketError(Exception):
  ''' Exception thrown when no full packet is available '''
  pass
