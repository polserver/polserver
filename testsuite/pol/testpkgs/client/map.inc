// Shared helpers for the client-driven half of the Map tests.
//
// Using a map is the only thing that puts one on the wire: Map::builtin_on_use
// sends the gump packet - 0x90, or 0xF5 to a client from 7.0.13.0 on - and then
// a 0x56 stream that clears the client's pins and adds the ones the map
// carries, one packet each. Nothing else in the core sends either.
//
// Two things shape every case here:
//
// - A pin travels as gump pixels, not as world coordinates. The map's world
//   area and its gump size are two independent members, and the core scales
//   between them on the way out (worldToGump) and on the way back in
//   (gumpToWorld). Every case that asserts a coordinate has to say which space
//   it is in, so the helpers below name them apart.
// - The whole stream is unsolicited: nothing is acknowledged, and a use that
//   sends nothing is indistinguishable from one that has not arrived yet. So
//   the negative cases pay a quiet period, and the positive ones read the
//   packets in the order the core writes them.
//
// The map objtypes are testpkgs/item's, which are shard data - this package
// only drives them from a client.

use os;
use uo;

include "testutil";
include "communication";

// the Map entries of testpkgs/item/itemdesc.cfg, and one plain item from the
// same package for the cases that name something which is not a map at all
const TESTMAP := 0x200015;
const TESTMAP_LOCKED := 0x200016;
const MAP_NOT_A_MAP := 0x0EED;

// The types of the 0x56 packet, PKTBI_56 in the core. The client sends the
// first six; the server answers a toggle with the seventh and uses ADD and
// REMOVE_ALL to describe a map it is sending.
const MAP_ADD := 1;
const MAP_INSERT := 2;
const MAP_CHANGE := 3;
const MAP_REMOVE := 4;
const MAP_REMOVE_ALL := 5;
const MAP_TOGGLE_EDIT := 6;
const MAP_TOGGLE_RESPONSE := 7;

// The gump art the core names in every map packet, and the only value it ever
// writes there.
const MAP_GUMPART := 0x139D;

// The world area and the gump the test maps are built with. The area is 160
// tiles across an 80 pixel gump, so one pixel is two tiles and a coordinate
// that has been through the conversion cannot be mistaken for one that has not.
//
// A pin has to be a coordinate the map's realm holds, and britannia is 192 by
// 192 - so the area a map stands for has to fit inside that, even though the
// members that describe it would take any u16.
const MAP_WEST := 20;
const MAP_NORTH := 20;
const MAP_EAST := 180;
const MAP_SOUTH := 180;
const MAP_GUMPWIDTH := 80;
const MAP_GUMPHEIGHT := 80;

// two pins inside that area, at offsets that survive the halving distinctly
const PIN1_X := 60;
const PIN1_Y := 80;
const PIN2_X := 120;
const PIN2_Y := 160;

// The client version that decides which gump packet the core sends. Anything
// from CLIENT_VER_70130 on gets 0xF5, which carries the facet as well.
const VERSION_OLD := "7.0.9.1";
const VERSION_NEW := "7.0.13.0";

// Where the map tests stand. Clear of the plots the house cases build on and of
// the paths the movement cases walk.
const MAP_X := 55;
const MAP_Y := 75;

var clientcon := getClientConnection();
var char;

// Finds the character the map tests drive and stands it on clear ground. For
// the program of every map test file.
function map_setup()
  var acct := FindAccount( "testclient0" );
  char := acct.getcharacter( 1 );
  if ( !char )
    return ret_error( "Could not find char at slot 1" );
  endif
  return map_stand();
endfunction

// A move to where the character already is sends nothing, so there would be
// nothing to wait for - and this runs on every case.
function map_stand()
  if ( char.x == MAP_X and char.y == MAP_Y )
    return 1;
  endif
  MoveObjectToLocation( char, MAP_X, MAP_Y, 0, flags := MOVEOBJECT_FORCELOCATION );
  return wait_for_own_create( 0 );
endfunction

// Clears whatever the case before left behind. Every case opens with this.
function map_start()
  Clear_Event_Queue();
  return map_stand();
endfunction

// A map in the character's backpack, which is where find_legal_item looks and
// therefore the only place a client can reach one. The area and the gump size
// are members rather than itemdesc properties, so every map starts at zero and
// has to be told what it stands for - a map with no gump size is not sent at
// all, which is a case of its own.
function make_map( resmgn, objtype := TESTMAP, gumpwidth := MAP_GUMPWIDTH, gumpheight := MAP_GUMPHEIGHT )
  var mp := resmgn.CreateItemInContainer( char.backpack, objtype );
  if ( !mp )
    return ret_error( $"could not create map {objtype}: {mp}" );
  endif
  mp.xwest := MAP_WEST;
  mp.ynorth := MAP_NORTH;
  mp.xeast := MAP_EAST;
  mp.ysouth := MAP_SOUTH;
  mp.gumpwidth := gumpwidth;
  mp.gumpheight := gumpheight;
  return mp;
endfunction

// What a world coordinate inside the map's area arrives at the client as, and
// back. Both are the core's own arithmetic (worldToGump / gumpToWorld) rather
// than a fixed table, so a case can name whichever space suits it.
// The tiles per pixel are a fraction whenever the gump has more pixels than the
// area has tiles, and a division of two integers truncates - so the ratio has
// to be taken as a double here, the way the core takes it as a float.
function world_to_gump( world, west := MAP_WEST, east := MAP_EAST, gumpsize := MAP_GUMPWIDTH )
  return CInt( CDbl( world - west ) / ( CDbl( east - west ) / gumpsize ) );
endfunction

function gump_to_world( gump, west := MAP_WEST, east := MAP_EAST, gumpsize := MAP_GUMPWIDTH )
  return CInt( west + ( CDbl( east - west ) / gumpsize ) * gump );
endfunction

// Asks a client to double-click a map, which is the route a player takes.
function client_use_map( mp, id := 0 )
  clientcon.sendevent( struct{ todo := "double_click", arg := mp.serial, id := id } );
  return 1;
endfunction

// Sends one 0x56 command. x and y are gump pixels; pinidx is what insert,
// change and remove address.
function send_pin( action, mp, pinidx := 0, x := 0, y := 0, id := 0 )
  var arg := struct{};
  arg.+serial := mp.serial;
  arg.+action := action;
  arg.+pinidx := pinidx;
  arg.+x := x;
  arg.+y := y;
  clientcon.sendevent( struct{ todo := "map_pin", arg := arg, id := id } );
  return 1;
endfunction

// Waits for the gump packet of a given map.
function wait_for_map( mp, id := 0, timeout := 5 )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( POLL_INTERVAL );
    if ( !ev or ev.type != EVT_MAP or CInt( ev.id ) != id )
      continue;
    endif
    if ( CInt( ev.serial ) != mp.serial )
      continue;
    endif
    return ev;
  endwhile
  return ret_error( $"client {id} was never sent map 0x{mp.serial:x} within {timeout}s" );
endfunction

// Waits for one 0x56 packet of a given map, whatever it says.
function wait_for_pin( mp, id := 0, timeout := 5 )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( POLL_INTERVAL );
    if ( !ev or ev.type != EVT_MAP_PIN or CInt( ev.id ) != id )
      continue;
    endif
    if ( CInt( ev.serial ) != mp.serial )
      continue;
    endif
    return ev;
  endwhile
  return ret_error( $"client {id} was sent no pin packet for 0x{mp.serial:x} within {timeout}s" );
endfunction

// The whole pin stream that follows a gump packet, as one "action/pinidx:x,y;"
// string. It ends when nothing more has come for a quiet period, because the
// core does not say how many packets are coming - so this costs that period
// every time.
function pin_stream( mp, id := 0, quiet := QUIET_SECS )
  var text := "";
  var deadline := ReadMillisecondClock() + quiet * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( POLL_INTERVAL );
    if ( !ev or ev.type != EVT_MAP_PIN or CInt( ev.id ) != id )
      continue;
    endif
    if ( CInt( ev.serial ) != mp.serial )
      continue;
    endif
    var action := CInt( ev.action );
    var pinidx := CInt( ev.pinidx );
    text := text + $"{action}/{pinidx}:{CInt( ev.x )},{CInt( ev.y )};";
    deadline := ReadMillisecondClock() + quiet * 1000;
  endwhile
  return text;
endfunction

// Asserts a client is sent no gump packet at all. The whole window is paid, so
// it is kept to what a packet that was really sent would need to arrive.
function no_map_for( id := 0, timeout := QUIET_SECS )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( POLL_INTERVAL );
    if ( !ev or ev.type != EVT_MAP or CInt( ev.id ) != id )
      continue;
    endif
    return ret_error( $"client {id} was sent a map it should not have been" );
  endwhile
  return 1;
endfunction

// Asks a map to start plotting and checks it says it has. A toggle is the only
// 0x56 the core answers, and it answers in the byte that carries the pin index
// everywhere else - so the new plotting state arrives as ev.pinidx.
function start_plotting( mp, id := 0 )
  send_pin( MAP_TOGGLE_EDIT, mp, 0, 0, 0, id );
  var ev := wait_for_pin( mp, id );
  if ( !ev )
    return ev;
  endif
  var action := CInt( ev.action );
  if ( action != MAP_TOGGLE_RESPONSE )
    return ret_error( $"the toggle was answered with action {action}" );
  endif
  if ( CInt( ev.pinidx ) != 1 )
    return ret_error( $"map 0x{mp.serial:x} did not start plotting" );
  endif
  return 1;
endfunction

// Waits until every command sent before it has been read. Nothing the client
// sends about a map is acknowledged except a toggle, so a pair of them - which
// leaves the map plotting exactly as it was - is what proves an add or an erase
// ahead of it has already been applied or refused.
function pin_settle( mp, id := 0 )
  for i := 1 to 2
    send_pin( MAP_TOGGLE_EDIT, mp, 0, 0, 0, id );
    var ev := wait_for_pin( mp, id );
    if ( !ev )
      return ev;
    endif
  endfor
  return 1;
endfunction

// The pin list of a map as one "x,y;" string of world coordinates, the same
// shape testpkgs/item asserts in. This is what says whether a command the
// client sent was applied - the core acknowledges none of them.
function map_pin_text( mp )
  var text := "";
  foreach pin in ( mp.getpins() )
    text := text + $"{pin.x},{pin.y};";
  endforeach
  return text;
endfunction
