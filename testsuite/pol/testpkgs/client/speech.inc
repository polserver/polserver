// Shared state and helpers for the server-to-client speech tests.
//
// All the speech functions put their text into the same two packets, 0x1C and
// 0xAE, and what tells them apart is everything around it: which clients receive
// it, the serial and body it is attributed to, the name in the journal field, the
// font and the colour. The helpers here hand back the whole event so a case can
// assert all of that.

use os;
use uo;

include "testutil";
include "communication";

var char;
var char2;

var clientcon := getClientConnection();

// Finds the two client characters and puts them in sight of each other, so that
// anything sent to everyone in range reaches both clients. To be called from the
// program of every speech test file.
function speech_setup()
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

// Waits for a speech event of the given client carrying the given text. The
// clients hear each other and the shard talks at them, so the text is what tells
// the wanted event from the noise.
function wait_for_speech( id, text, timeout := 10 )
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

// Same, for text both clients are meant to hear: which of them is told first is
// not fixed, so they have to be collected in one loop.
function wait_for_speeches( text, timeout := 10 )
  var found := { 0, 0 };
  var deadline := ReadMillisecondClock() + timeout * 1000;

  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_SPEECH or ev.msg != text )
      continue;
    endif
    // client event numbers come out of UnpackJson as Doubles, and indexing an
    // array with one of those hands back the whole array
    var idx := CInt( ev.id ) + 1;
    if ( idx < 1 or idx > 2 or found[idx] )
      continue;
    endif
    found[idx] := ev;
    if ( found[1] and found[2] )
      return found;
    endif
  endwhile

  var missing := found[1] ? 1 : 0;
  return ret_error( $"client {missing} never heard \"{text}\" within {timeout}s" );
endfunction

// Same, matching a text by its opening instead of in full - a text that made the
// round trip through UTF-16 and back is only worth comparing on the part that
// survives it unchanged.
function wait_for_speech_like( id, prefix, timeout := 10 )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_SPEECH or CInt( ev.id ) != id or ev.msg.find( prefix ) != 1 )
      continue;
    endif
    return ev;
  endwhile
  return ret_error( $"client {id} never heard anything starting with \"{prefix}\" within {timeout}s" );
endfunction

// Same as wait_for_speeches, matching a text by its opening.
function wait_for_speeches_like( prefix, timeout := 10 )
  var found := { 0, 0 };
  var deadline := ReadMillisecondClock() + timeout * 1000;

  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_SPEECH or ev.msg.find( prefix ) != 1 )
      continue;
    endif
    var idx := CInt( ev.id ) + 1;
    if ( idx < 1 or idx > 2 or found[idx] )
      continue;
    endif
    found[idx] := ev;
    if ( found[1] and found[2] )
      return found;
    endif
  endwhile

  var missing := found[1] ? 1 : 0;
  return ret_error( $"client {missing} never heard anything starting with \"{prefix}\" within {timeout}s" );
endfunction

// Asserts a client is told nothing carrying the given text.
//
// The whole window is always paid, so it is kept to the round trip a wrongly
// sent message would need rather than to a comfortable-looking couple of
// seconds. Most callers have already waited out the same message on the other
// client, which proves the core has sent everything it was going to.
function no_speech_for( id, text, timeout := QUIET_SECS )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( POLL_INTERVAL );
    if ( !ev or ev.type != EVT_SPEECH or CInt( ev.id ) != id or ev.msg != text )
      continue;
    endif
    return ret_error( $"client {id} heard \"{text}\", which was not meant for it" );
  endwhile
  return 1;
endfunction

// The same three waits for cliloc messages, which carry no text of their own:
// what tells one from the next is the number the client would look up, so that
// is what they match on. The arguments come along in ev.msg.
function wait_for_cliloc( id, num, timeout := 10 )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_CLILOC or CInt( ev.id ) != id or CInt( ev.cliloc ) != num )
      continue;
    endif
    return ev;
  endwhile
  return ret_error( $"client {id} never got cliloc {num} within {timeout}s" );
endfunction

function wait_for_clilocs( num, timeout := 10 )
  var found := { 0, 0 };
  var deadline := ReadMillisecondClock() + timeout * 1000;

  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_CLILOC or CInt( ev.cliloc ) != num )
      continue;
    endif
    var idx := CInt( ev.id ) + 1;
    if ( idx < 1 or idx > 2 or found[idx] )
      continue;
    endif
    found[idx] := ev;
    if ( found[1] and found[2] )
      return found;
    endif
  endwhile

  var missing := found[1] ? 1 : 0;
  return ret_error( $"client {missing} never got cliloc {num} within {timeout}s" );
endfunction

function no_cliloc_for( id, num, timeout := QUIET_SECS )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( POLL_INTERVAL );
    if ( !ev or ev.type != EVT_CLILOC or CInt( ev.id ) != id or CInt( ev.cliloc ) != num )
      continue;
    endif
    return ret_error( $"client {id} got cliloc {num}, which was not meant for it" );
  endwhile
  return 1;
endfunction

function wait_for_own_create( id )
  var ev := waitForClient( id, { EVT_OWNCREATE } );
  if ( !ev )
    return ret_error( $"client {id} was not moved into place" );
  endif
  return 1;
endfunction
