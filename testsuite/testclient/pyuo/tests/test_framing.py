#!/usr/bin/env python3

'''Pins the two packet framers in pyuo/net.py.

Framing is the one part of the client that has to survive being fed bytes in
whatever sizes the network hands over. Nothing in the shard suite exercises a
packet split across two reads - the loopback delivers these small packets whole
almost every time - so the resume path is only ever covered here.

The compressed framer is checked against an independent bit-at-a-time decoder
rather than against recorded output: the one in net.py walks a whole byte per
lookup and memoizes the result, which is a different algorithm reaching the same
answer, and only comparing the two says so.

Run: python tests/test_framing.py
'''

import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from pyuo import packets
from pyuo.net import Network


# ---------------------------------------------------------------- encoder

def buildCodes():
  '''! Derives the encoding table from the decoding tree.

  @return dict: byte value (or 'halt') -> list of bits
  '''
  tree = Network.DECOMPRESSION_TREE
  codes = {}
  seen = set()

  def walk(node, path):
    assert node not in seen, "the decompression tree is not a tree"
    seen.add(node)
    for bit in (0, 1):
      val = tree[node][bit]
      sub = path + [bit]
      if val == -256:
        codes['halt'] = sub
      elif val < 1:
        codes[-val] = sub
      else:
        walk(val, sub)

  walk(0, [])
  return codes


CODES = buildCodes()


def compress(raw):
  '''! Encodes one packet the way the server does: codewords, halt, then padding
  out to the end of the byte.
  '''
  bits = []
  for byte in raw:
    bits.extend(CODES[byte])
  bits.extend(CODES['halt'])
  while len(bits) % 8:
    bits.append(0)
  out = bytearray()
  for i in range(0, len(bits), 8):
    val = 0
    for bit in bits[i:i + 8]:
      val = (val << 1) | bit
    out.append(val)
  return bytes(out)


# ---------------------------------------------------------------- reference

def refDecompress(data):
  '''! Decodes a compressed stream one bit at a time, the obvious way.

  @return list of (packet, bytes consumed by that packet)
  '''
  tree = Network.DECOMPRESSION_TREE
  found = []
  node = 0
  out = bytearray()
  start = 0
  pos = 0
  while pos < len(data):
    byte = data[pos]
    halted = False
    for shift in range(7, -1, -1):
      val = tree[node][(byte >> shift) & 1]
      if val == -256:
        halted = True
        break
      if val < 1:
        out.append(-val)
        val = 0
      node = val
    pos += 1
    if halted:
      found.append((bytes(out), pos - start))
      out = bytearray()
      node = 0
      start = pos
  return found


# ---------------------------------------------------------------- fixtures

## A fixed-length packet and a variable-length one, by command byte. Real
## commands, because _plainLen() asks packets.classes how long each one is.
FIXED_CMD = 0x0b          # DamagePacket, 7 bytes
VAR_CMD = 0x1c            # SendSpeechPacket, length in the ushort at offset 1


def plainFixed(seed):
  cls = packets.classes[FIXED_CMD]
  body = bytes((seed + i) & 0xff for i in range(cls.length - 1))
  return bytes([FIXED_CMD]) + body


def plainVar(seed, size):
  assert size >= 3
  body = bytes((seed + i) & 0xff for i in range(size - 3))
  return bytes([VAR_CMD, (size >> 8) & 0xff, size & 0xff]) + body


def frames(net, data):
  '''! Feeds bytes and takes every whole frame the framer will give back '''
  net.feed(data)
  out = []
  while True:
    raw = net._nextPacket()
    if raw is None:
      return out
    out.append(raw)


# ---------------------------------------------------------------- checks

CHECKS = [0]
FAILURES = []


def check(cond, what):
  CHECKS[0] += 1
  if not cond:
    FAILURES.append(what)


def checkEq(got, want, what):
  CHECKS[0] += 1
  if got != want:
    FAILURES.append("{}: got {!r}, want {!r}".format(what, got, want))


# ---------------------------------------------------------------- plain

def testPlainWhole():
  '''One packet per feed, both length kinds.'''
  for i in range(20):
    net = Network()
    pkt = plainFixed(i) if i % 2 else plainVar(i, 12 + i)
    checkEq(frames(net, pkt), [pkt], "plain whole #%d" % i)
    checkEq(len(net.buf), 0, "plain whole #%d leaves nothing buffered" % i)


def testPlainCoalesced():
  '''Two packets in one segment - the case that used to trip an assertion,
  because the framer assumed the buffer held exactly one packet.'''
  for i in range(20):
    net = Network()
    a = plainFixed(i)
    b = plainVar(i, 9 + i)
    checkEq(frames(net, a + b), [a, b], "plain coalesced #%d" % i)


def testPlainSplitEverywhere():
  '''Split at every byte offset. A length prefix cut in half is the awkward one:
  _plainLen() cannot answer until three bytes are buffered.'''
  a = plainFixed(3)
  b = plainVar(7, 40)
  stream = a + b
  for cut in range(1, len(stream)):
    net = Network()
    got = frames(net, stream[:cut])
    got += frames(net, stream[cut:])
    checkEq(got, [a, b], "plain split at %d" % cut)


def testPlainPartialIsQuiet():
  '''A prefix yields nothing and corrupts nothing.'''
  pkt = plainVar(1, 30)
  for cut in range(1, len(pkt)):
    net = Network()
    checkEq(frames(net, pkt[:cut]), [], "plain prefix %d yields nothing" % cut)
    checkEq(frames(net, pkt[cut:]), [pkt], "plain prefix %d completes" % cut)


# ---------------------------------------------------------------- compressed

def compressedNet():
  net = Network()
  net.compress = True
  return net


def testCompressedWhole():
  for i in range(20):
    net = compressedNet()
    pkt = plainFixed(i) if i % 2 else plainVar(i, 12 + i)
    checkEq(frames(net, compress(pkt)), [pkt], "compressed whole #%d" % i)
    checkEq(len(net.buf), 0, "compressed whole #%d leaves nothing buffered" % i)


def testCompressedBackToBack():
  '''Several packets in one segment. The halt codeword is the only thing saying
  where one ends, so this is what proves the framer does not stop at the first.'''
  for n in (2, 3, 8):
    pkts = [plainFixed(i) for i in range(n)]
    net = compressedNet()
    stream = b''.join(compress(p) for p in pkts)
    checkEq(frames(net, stream), pkts, "compressed %d back to back" % n)


def testCompressedSplitEverywhere():
  '''Split at every byte offset - the resume path, byte by byte.'''
  pkts = [plainFixed(5), plainVar(9, 24)]
  stream = b''.join(compress(p) for p in pkts)
  for cut in range(1, len(stream)):
    net = compressedNet()
    got = frames(net, stream[:cut])
    got += frames(net, stream[cut:])
    checkEq(got, pkts, "compressed split at %d" % cut)


def testCompressedRandomSplits():
  '''Fed in random chunks, which is what a busy socket actually does.'''
  rnd = random.Random(20260920)
  pkts = [plainVar(i, 8 + (i * 7) % 60) for i in range(12)]
  stream = b''.join(compress(p) for p in pkts)
  for trial in range(30):
    net = compressedNet()
    got = []
    pos = 0
    while pos < len(stream):
      step = rnd.randint(1, 9)
      got += frames(net, stream[pos:pos + step])
      pos += step
    checkEq(got, pkts, "compressed random split trial %d" % trial)


def testCompressedOneBytePerFeed():
  '''The worst case the resume state has to survive.'''
  pkts = [plainFixed(2), plainFixed(4)]
  stream = b''.join(compress(p) for p in pkts)
  net = compressedNet()
  got = []
  for i in range(len(stream)):
    got += frames(net, stream[i:i + 1])
  checkEq(got, pkts, "compressed one byte per feed")


def testCompressedDifferential():
  '''The memoized whole-byte walk against a plain bit-at-a-time one: same
  packets, and the same number of bytes consumed for each.'''
  rnd = random.Random(1077)
  for trial in range(40):
    pkts = [bytes([FIXED_CMD]) + bytes(rnd.randrange(256) for _ in range(6))
            for _ in range(rnd.randint(1, 6))]
    stream = b''.join(compress(p) for p in pkts)

    want = refDecompress(stream)
    checkEq([p for p, _ in want], pkts, "reference agrees with the encoder #%d" % trial)

    net = compressedNet()
    net.feed(stream)
    got = []
    while True:
      before = len(net.buf)
      raw = net._nextPacket()
      if raw is None:
        break
      got.append((raw, before - len(net.buf)))
    checkEq(got, want, "differential #%d" % trial)


def testCompressedEmptyPacket():
  '''A halt codeword and nothing else. Degenerate, but it must not wedge the
  decoder or be mistaken for "not yet".'''
  net = compressedNet()
  checkEq(frames(net, compress(b'')), [b''], "compressed empty packet")


# ---------------------------------------------------------------- decode wiring

def testRecvNowaitDecodes():
  '''recvNowait() frames and decodes without going near a socket.'''
  po = packets.PingPacket()
  po.fill(0x42)
  raw = po.encode()

  net = Network()
  net.feed(raw)
  pkt = net.recvNowait()
  check(isinstance(pkt, packets.PingPacket), "recvNowait returns a PingPacket")
  checkEq(pkt.seq, 0x42, "recvNowait keeps the sequence")
  checkEq(net.recvNowait(), None, "recvNowait on an empty buffer")

  net = compressedNet()
  net.feed(compress(raw))
  pkt = net.recvNowait()
  check(isinstance(pkt, packets.PingPacket), "recvNowait decompresses too")
  checkEq(pkt.seq, 0x42, "recvNowait keeps the sequence through compression")


# ----------------------------------------------------------------

def main():
  tests = [v for k, v in sorted(globals().items()) if k.startswith('test')]
  for t in tests:
    t()
  if FAILURES:
    print("FAILED {} of {} checks".format(len(FAILURES), CHECKS[0]))
    for f in FAILURES[:40]:
      print("  " + f)
    if len(FAILURES) > 40:
      print("  ... and {} more".format(len(FAILURES) - 40))
    return 1
  print("ok - {} checks in {} tests".format(CHECKS[0], len(tests)))
  return 0


if __name__ == '__main__':
  sys.exit(main())
