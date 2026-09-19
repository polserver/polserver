use os;
use polsys;
use uo;

include "clientpkt";
include "communication";

// Shared by the vendor tests: answering a merchant's window the way the client would, waiting for
// what the core made of the answer, and leaving the pack the way it was found. The client
// connection belongs to the test file, so it is passed in.

// Answers a sell window. The ack only says the packet went out; what the core did with it has to
// be waited for separately.
function sell( clientcon, vendor_serial, items )
  Clear_Event_Queue();
  clientcon.sendevent( struct{ todo := EVT_SELL_ITEMS,
                               arg := struct{ vendor_serial := vendor_serial, items := items },
                               id := 0 } );
  return waitForClient( 0, { EVT_SELL_ITEMS } );
endfunction

// Answers a sell window and returns what the vendor was told about the sale. Telling the vendor is
// the last thing the core does with an answer, so the rest of the sale is over by then.
function sell_heard( clientcon, vendor_serial, items )
  Clear_Event_Queue();
  clientcon.sendevent( struct{ todo := EVT_SELL_ITEMS,
                               arg := struct{ vendor_serial := vendor_serial, items := items },
                               id := 0 } );
  return wait_for_report( EVT_SELL_ITEMS );
endfunction

// The same for an answer put together byte by byte, which no client would send.
function sell_raw_heard( clientcon, pkt )
  Clear_Event_Queue();
  clientcon.sendevent( struct{ todo := "raw_packet", arg := packet_bytes( pkt ), id := 0 } );
  return wait_for_report( EVT_PACKET_SENT );
endfunction

// The client's ack and the vendor's report race each other, and waitForClient() would throw the
// report away if it came first, so both are taken as they come.
function wait_for_report( ack, timeout := 10 )
  var acked := 0;
  var report := 0;
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ( !acked or !report ) and ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev )
      continue;
    elseif ( ev.type == "merchant_bought" )
      report := ev;
    elseif ( ev.type == ack and ev.id == 0 )
      acked := 1;
    endif
  endwhile
  if ( !acked )
    return ret_error( "client 0 did not send the answer" );
  elseif ( !report )
    return ret_error( "the vendor was never told how the sale went" );
  endif
  return report;
endfunction

// For an answer the core has to drop: the vendor must hear nothing, neither before the client's
// ack nor for as long after it as a report would take to follow.
function sell_unheard( clientcon, vendor_serial, items, quiet := QUIET_SECS )
  Clear_Event_Queue();
  clientcon.sendevent( struct{ todo := EVT_SELL_ITEMS,
                               arg := struct{ vendor_serial := vendor_serial, items := items },
                               id := 0 } );
  var acked := 0;
  var deadline := ReadMillisecondClock() + 10000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( POLL_INTERVAL );
    if ( !ev )
      continue;
    elseif ( ev.type == "merchant_bought" )
      return ret_error( $"the vendor was told of a sale: {ev}" );
    elseif ( !acked and ev.type == EVT_SELL_ITEMS and ev.id == 0 )
      acked := 1;
      deadline := ReadMillisecondClock() + quiet * 1000;
    endif
  endwhile
  if ( !acked )
    return ret_error( "client 0 did not send the answer" );
  endif
  return 1;
endfunction

// A sell answer whose count field says `count`, carrying `entries`: an array of structs with a
// serial and an amount. The two only disagree in an answer no client would send.
function sell_answer( vendor_serial, count, entries )
  var pkt := CreatePacket( 0x9F, MSGLEN_VARIABLE );
  pkt.SetInt32( 3, vendor_serial );
  pkt.SetInt16( 7, count );
  var offset := 9;
  foreach entry in entries
    pkt.SetInt32( offset, entry.serial );
    pkt.SetInt16( offset + 4, entry.amount );
    offset += 6;
  endforeach
  return pkt;
endfunction

// The handlers run on the server after the packet lands; an ack only says the client sent it.
// Waits for the container membership a handler is supposed to produce.
function wait_for_container( item, cont, what )
  var attempts := 50;
  while ( attempts-- )
    if ( item.container == cont )
      return 1;
    endif
    Sleepms( 100 );
  endwhile
  return ret_error( $"{what} (it is in {item.container})" );
endfunction

// The container slot client 0 was told an item has, once it is one other than `not_slot`: a listing
// can be older than the move that changed it. Scripts cannot read a slot, so this is where to look.
function client_slot( clientcon, serial, cont_serial, not_slot := -1, timeout := 5 )
  var seen := error;
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var entry := client_finds_obj( clientcon, 0, serial, cont_serial, 1 );
    if ( entry )
      seen := CInt( entry.slot );
      if ( seen != not_slot )
        return seen;
      endif
    endif
    Sleepms( POLL_INTERVAL * 1000 );
  endwhile
  return ret_error( $"client 0 never saw 0x{serial:x} out of slot {not_slot} (last saw {seen})" );
endfunction

// Gold the core pays out belongs to no resource manager, so a sale's gold would outlive the test
// and change what the next one finds in the pack. Registers putting the pack's gold back the way
// it is now: stacks that are new go, stacks that grew shrink back.
function keep_pack_gold( resmgr, cont )
  var was := dictionary{};
  foreach thing in ( EnumerateItemsInContainer( cont ) )
    if ( thing.objtype == GetObjtypeByName( "goldcoin" ) )
      was[thing.serial] := thing.amount;
    endif
  endforeach
  return resmgr.AddCleanup( struct{ cont := cont, was := was }, @restore_pack_gold );
endfunction

function restore_pack_gold( kept )
  foreach thing in ( EnumerateItemsInContainer( kept.cont ) )
    if ( thing.objtype != GetObjtypeByName( "goldcoin" ) )
      continue;
    elseif ( !kept.was.exists( thing.serial ) )
      DestroyItem( thing );
    elseif ( thing.amount > kept.was[thing.serial] )
      SubtractAmount( thing, thing.amount - kept.was[thing.serial] );
    endif
  endforeach
  return 1;
endfunction

// For an item the core made - what is left of a stack sold in part, gold paid onto the ground -
// which would otherwise outlive the test.
function destroy_later( resmgr, item )
  return resmgr.AddCleanup( item, @destroy_item );
endfunction

function destroy_item( item )
  return DestroyItem( item );
endfunction
