use os;
use polsys;

include "communication";

// Sending a packet as the client.
//
// Everything the client knows how to build goes out through send_client_packet(): the packet is
// named and handed its fields, and the test client builds it the way it builds every other packet
// it sends. Adding a packet here is adding a method to the client, not bytes to a test - see
// sendClientPacket() in testclient.py for the names and what each one takes.
//
// What is left for send_packet() is only a packet that is deliberately wrong: one cut short, or
// one whose fields say something the packet does not carry. Those are built with the shard's own
// Packet object - CreatePacket(), SetInt8() and friends - and read back out as bytes, because the
// test connection carries JSON and cannot hand a Packet object over.
//
// The one rule the core enforces on the other end: a packet whose handler was registered with a
// fixed size has to be exactly that long, because the core reads that many bytes no matter what
// was sent. A short one swallows the beginning of whatever comes next, which shows up as the
// client being disconnected rather than as a failure of the case that sent it.

// Sends a packet the client knows, and waits for the client to say it is away. What the shard
// makes of it is up to the caller to wait for; this only orders the two.
//
// A caller that is already expecting something back passes wait := 0. waitForClient() throws away
// what it is not waiting for, so a case that has a packet in flight would lose it to the ack; the
// client raises no ack at all when it is not asked for one, rather than leaving it in the queue.
function send_client_packet( clientcon, id, packet, wait := 1 )
  if ( !wait )
    packet.+ack := 0;
  endif
  clientcon.sendevent( struct{ todo := "client_packet", arg := packet, id := id } );
  if ( !wait )
    return 1;
  endif
  var ev := waitForClient( id, { EVT_PACKET_SENT } );
  if ( !ev )
    return ret_error( $"client {id} never acknowledged a {packet.packet} packet" );
  endif
  return 1;
endfunction

// Reads a packet back out as the bytes it is.
function packet_bytes( pkt )
  var bytes := array{};
  var size := pkt.GetSize();
  for i := 0 to size - 1
    bytes.append( pkt.GetInt8( i ) );
  endfor
  return bytes;
endfunction

// Puts bytes on the wire as they are, for a packet no client would send.
function send_packet( clientcon, id, pkt )
  clientcon.sendevent( struct{ todo := "raw_packet", arg := packet_bytes( pkt ), id := id } );
  var ev := waitForClient( id, { EVT_PACKET_SENT } );
  if ( !ev )
    return ret_error( $"client {id} never acknowledged a raw packet" );
  endif
  return 1;
endfunction
