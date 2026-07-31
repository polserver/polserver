// Shared state and helpers for the secure trade window tests.
//
// The window only exists while both characters have an active client, so every
// one of those tests drives the two test clients. Both characters are parked next
// to each other: AllowMovingTrade is off in the test shard, which cancels a trade
// as soon as the two walk more than 3 tiles apart.

use os;
use uo;

include "testutil";
include "communication";

var char;
var char2;

var clientcon := getClientConnection();

// Finds the two client characters and parks them next to each other. To be
// called from the program of every trade test file.
function trade_setup()
  var a := FindAccount( "testclient0" );
  char := a.getcharacter( 1 );
  if ( !char )
    return ret_error( "Could not find char at slot 1" );
  endif
  a := FindAccount( "testclient1" );
  char2 := a.getcharacter( 1 );
  if ( !char2 )
    return ret_error( "Could not find char2 at slot 1" );
  endif

  // one at a time: waitForClient() throws away the events of the other client
  MoveObjectToLocation( char2, 101, 50, 0, flags := MOVEOBJECT_FORCELOCATION );
  var res := wait_for_own_create( 1 );
  if ( !res )
    return res;
  endif
  MoveObjectToLocation( char, 100, 50, 0, flags := MOVEOBJECT_FORCELOCATION );
  return wait_for_own_create( 0 );
endfunction

function wait_for_own_create( id )
  var ev := waitForClient( id, { EVT_OWNCREATE } );
  if ( !ev )
    return ret_error( $"client {id} was not moved into place" );
  endif
  return 1;
endfunction

// Opens a trade window and returns the init event of both clients.
function open_trade()
  close_trades();

  Clear_Event_Queue();
  var res := SecureTradeWin( char, char2 );
  if ( !res )
    return ret_error( $"Could not open the trade window: {res}" );
  endif

  return wait_for_trades( TRADE_ACTION_INIT );
endfunction

// Closes whatever window is open and waits for both clients to hear about it. A
// case that failed halfway through leaves one behind, and the core refuses to
// open a second one for the same character. Cancelling one side cancels the
// other, so one call is enough.
//
// The waiting matters: the cancel travels through the test client, and a case
// that asserts the clients are told nothing would otherwise catch the tail of
// the one before it.
function close_trades()
  var open := char.tradewindow or char2.tradewindow;
  if ( char.tradewindow )
    CloseTradeWindow( char );
  elseif ( char2.tradewindow )
    CloseTradeWindow( char2 );
  endif
  if ( open )
    wait_for_trades( TRADE_ACTION_CANCEL );
  endif
  Clear_Event_Queue();
endfunction

// Waits for a trade window event of the given action on both clients and returns
// the two of them, client 0 first. Every action of the window is sent to both
// sides, and which of the two arrives first is not fixed, so they have to be
// collected in one loop - waiting for them one after the other would throw the
// event of the client not waited for away.
//
// Offering an item sends three status updates of its own, and those are still on
// their way through the test client when the next step clears the event queue,
// so the accept indicators are waited for by value, not just by action. Pass a
// struct{ cont1, cont2 } per client to do so, 0 to take any.
function wait_for_trades( action, expected := 0, expected2 := 0, timeout := 10 )
  var wanted := { expected, expected2 };
  var found := { 0, 0 };
  var deadline := ReadMillisecondClock() + timeout * 1000;

  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_TRADE or ev.action != action )
      continue;
    endif
    // the client events come out of UnpackJson, so every number in them is a
    // Double - and indexing an array with one of those hands back the whole
    // array instead of the element
    var idx := CInt( ev.id ) + 1;
    if ( idx < 1 or idx > 2 or found[idx] )
      continue;
    endif
    var expect := wanted[idx];
    if ( expect and ( ev.cont1 != expect.cont1 or ev.cont2 != expect.cont2 ) )
      continue;
    endif
    found[idx] := ev;
    if ( found[1] and found[2] )
      return found;
    endif
  endwhile

  var missing := found[1] ? 1 : 0;
  var expect := wanted[missing + 1];
  if ( expect )
    return ret_error( $"client {missing} never saw the trade indicators {expect.cont1}/{expect.cont2} within {timeout}s" );
  endif
  return ret_error( $"client {missing} received no trade action {action} within {timeout}s" );
endfunction

// Asserts that no client hears about the trade window at all.
function wait_for_no_trade( timeout := 2 )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_TRADE )
      continue;
    endif
    return ret_error( $"client {ev.id} was told about the trade window: action {ev.action}" );
  endwhile
  return 1;
endfunction

// Accepts or un-accepts the trade for the given client. Only a client can do
// this, there is no module function for it.
function send_accept( id, flag )
  clientcon.sendevent( struct{ todo := "trade",
                               arg := struct{ action := TRADE_ACTION_STATUS, flag := flag },
                               id := id } );
endfunction

function send_trade_cancel( id )
  clientcon.sendevent( struct{ todo := "trade", arg := struct{
                                                action := TRADE_ACTION_CANCEL }, id := id } );
endfunction

// Picks an item up with client 0. There is no success answer for a lift, only a
// rejection, so this waits a moment for one not to arrive.
function lift_item( serial )
  Clear_Event_Queue();
  clientcon.sendevent( struct{ todo := "lift_item", arg := serial, id := 0 } );
  var ev := waitForClient( 0, { EVT_LIFT_ITEM } );
  if ( !ev )
    return ret_error( $"client 0 did not lift 0x{serial:x}: {ev}" );
  endif
  ev := waitForClient( 0, { EVT_MOVE_ITEM_REJECTED }, 1 );
  if ( ev )
    return ret_error( $"lifting 0x{serial:x} was rejected: reason {ev.reason}" );
  endif
  return 1;
endfunction

// Drops the item client 0 is holding onto a mobile. Does not wait for the drop
// to be approved: opening a trade window sends the init to both clients first,
// and waiting for the approval would throw those away.
function drop_on_mobile( serial, target_serial )
  Clear_Event_Queue();
  clientcon.sendevent( struct{ todo := "drop_item",
                               arg := struct{ serial := serial, x := 0xFFFF,
                                              y := 0xFFFF, z := 0, dropped_on_serial := target_serial },
                               id := 0 } );
  var ev := waitForClient( 0, { EVT_DROP_ITEM } );
  if ( !ev )
    return ret_error( $"client 0 did not drop 0x{serial:x}: {ev}" );
  endif
  return 1;
endfunction

// Drops the held item onto a mobile and expects the core to refuse it.
function drop_on_mobile_rejected( serial, target_serial )
  var res := drop_on_mobile( serial, target_serial );
  if ( !res )
    return res;
  endif
  var ev := waitForClient( 0, { EVT_MOVE_ITEM_REJECTED } );
  if ( !ev )
    return ret_error( $"dropping 0x{serial:x} on 0x{target_serial:x} was not rejected" );
  endif
  return 1;
endfunction
