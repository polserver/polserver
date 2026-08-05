// Shared state and helpers for the packet hook tests.
//
// The hooks themselves live in packethook.src and are installed by uopacket.cfg.
// They are inert until a test parks an arm struct in the matching global property,
// so every helper here comes in an arm/disarm pair and every case has to disarm -
// a hook left armed changes what every later test in the shard sees.
use os;
use uo;
use polsys;

include "testutil";
include "communication";

const PKTHOOK_0x77 := "#PktHook0x77";
const PKTHOOK_0x77_LOG := "#PktHook0x77Log";
const PKTHOOK_0x1C := "#PktHook0x1C";
const PKTHOOK_0x1C_LOG := "#PktHook0x1CLog";
const PKTHOOK_0x34 := "#PktHook0x34";
const PKTHOOK_0x34_LOG := "#PktHook0x34Log";
const PKTHOOK_0x5D_LOG := "#PktHook0x5DLog";

// the 0x1C header is 44 bytes, everything after it is the text and its terminator
const SPEECH_TEXT_OFFSET := 44;

// the body an npc created from the packet_probe template really has
const PROBE_GRAPHIC := 0x190;

var char;
var char2;

var clientcon := getClientConnection();

// Puts both client characters in a known place, client 0 with room around it for
// the probe npc to move in. To be called from the program of every hook test file.
function pkthook_setup()
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

// arms a hook and clears whatever an earlier case left in its log
function arm_hook( key, log_key, arm )
  EraseGlobalProperty( log_key );
  SetGlobalProperty( key, arm );
  return 1;
endfunction

function disarm_hook( key )
  EraseGlobalProperty( key );
  return 1;
endfunction

// what the hook recorded, as an array - empty when it never fired
function hook_log( log_key )
  var log := GetGlobalProperty( log_key );
  if ( !log )
    return array{};
  endif
  return log;
endfunction

// An outgoing hook is called once for every client the packet is going to, so a
// mobile both test clients can see produces two entries per move and the order
// between them is not fixed. This picks the one addressed to a given character.
function hook_entry_for( log, recipient )
  foreach entry in ( log )
    if ( entry.recipient == recipient )
      return entry;
    endif
  endforeach
  return ret_error( $"the hook never fired for 0x{recipient:x}, log holds {len( log )} entries" );
endfunction

// Polls a client's object list until the given serial carries the given graphic.
// A listing is answered from the world the client knew one iteration ago, so a
// single shot right after the core sent something is a race that cannot be won.
function client_sees_graphic( id, serial, graphic, timeout := 10 )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  var last := 0;
  while ( ReadMillisecondClock() < deadline )
    var objs := pkthook_client_objs( id );
    if ( !objs )
      return objs;
    endif
    foreach obj in ( objs )
      if ( CInt( obj.serial ) != serial )
        continue;
      endif
      last := CInt( obj.graphic );
      if ( last == graphic )
        return obj;
      endif
    endforeach
    SleepMS( 100 );
  endwhile
  return ret_error( $"client {id} shows 0x{serial:x} as body 0x{last:x}, expected 0x{graphic:x}" );
endfunction

// Polls until the client reports the given serial at the given position.
function client_sees_position( id, serial, x, y, timeout := 10 )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  var last := "nowhere";
  while ( ReadMillisecondClock() < deadline )
    var objs := pkthook_client_objs( id );
    if ( !objs )
      return objs;
    endif
    foreach obj in ( objs )
      if ( CInt( obj.serial ) != serial )
        continue;
      endif
      var pos := obj.pos;
      if ( CInt( pos[1] ) == x and CInt( pos[2] ) == y )
        return obj;
      endif
      last := $"{CInt( pos[1] )},{CInt( pos[2] )}";
    endforeach
    SleepMS( 100 );
  endwhile
  return ret_error( $"client {id} shows 0x{serial:x} at {last}, expected {x},{y}" );
endfunction

// Waits for a client to be told the exact given text. The shard talks at the
// clients on its own account, so the text is what tells the wanted event from the
// noise.
function wait_for_text( id, text, timeout := 10 )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_SPEECH or CInt( ev.id ) != id or ev.msg != text )
      continue;
    endif
    return ev;
  endwhile
  return ret_error( $"client {id} never heard \"{text}\" within {timeout}s" );
endfunction

// Asserts a client is *not* told the given text. This one always burns its whole
// window, so keep it short - but above a client round trip.
function no_text_for( id, text, secs := 0.75 )
  var deadline := ReadMillisecondClock() + secs * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 0.1 );
    if ( !ev or ev.type != EVT_SPEECH or CInt( ev.id ) != id )
      continue;
    endif
    if ( ev.msg == text )
      return ret_error( $"client {id} heard \"{text}\" after all" );
    endif
  endwhile
  return 1;
endfunction

// one listing of everything a client can see, on the connection these tests share
function pkthook_client_objs( id, timeout := 10 )
  var ev := list_client_objs( clientcon, id, 0, timeout );
  if ( !ev )
    return ev;
  endif
  return ev.objs;
endfunction
