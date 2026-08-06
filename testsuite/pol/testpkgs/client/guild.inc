// Shared helpers for the client-side guild tests.
//
// A guild is built from a script - CreateGuild and addmember are all it takes -
// but everything the core does with one is only visible to a client: guild and
// alliance chat are routed to the guild instead of to whoever is in range, and
// membership decides the colour of a name and the notoriety byte of a mobile.
//
// speech.inc is included for the two characters, the client connection and the
// speech waits, which is what guild chat arrives as.
use os;
use uo;
use guilds;
use file;
use polsys;

include "testutil";
include "communication";
include "guildutil";
include "speech";

// How long the shard has to stay silent before settle() calls it quiet. A stale
// event that arrives after this leaks into the next case's answer, so it wants
// to stay comfortably above a client round trip. POLL_INTERVAL comes from
// communication.inc.
const SETTLE_QUIET := 500;

// where speech_setup() leaves the two characters
const CHAR_X := 100;
const CHAR_Y := 50;
const CHAR2_X := 101;
const CHAR2_Y := 50;

// Finds the two client characters, puts them in sight of each other and takes
// them out of any guild left over. To be called from the program of every guild
// test file.
function guild_setup()
  var res := speech_setup();
  if ( !res )
    return res;
  endif
  return drop_guilds();
endfunction

// Every case takes its guilds apart when it is done, whether it passed or not.
function guild_fail( message )
  drop_guilds();
  return ret_error( message );
endfunction

function guild_finish( result )
  var res := drop_guilds();
  if ( !res )
    return res;
  endif
  return result;
endfunction

function say_guild( id, text )
  clientcon.sendevent( struct{ todo := "speech",
                               arg := struct{ text := text, type := TEXTTYPE_GUILD },
                               id := id } );
endfunction

function say_alliance( id, text )
  clientcon.sendevent( struct{ todo := "speech",
                               arg := struct{ text := text, type := TEXTTYPE_ALLIANCE },
                               id := id } );
endfunction

// Says the text as guild or alliance chat and checks which of the two clients
// heard it. `want` is one flag per client, in client order.
//
// Both clients are served in the same breath, so which of them heard something
// cannot be settled by a wait followed by an assertion that nothing else came:
// the wait would already have thrown the other client's event away. This
// listens once and sorts the result afterwards, and once everything expected
// has arrived it listens a moment longer - anything wrongly sent would have
// gone out with the rest.
//
// The poll is a tenth of a second rather than a whole one: a wait ends the
// moment an event arrives, so the interval only decides how far past the
// deadline the last idle wait runs. A whole second of that on every call is
// most of what the guild tests spend their time on.
function check_heard( text, want, timeout := 10, grace := 500 )
  var found := { 0, 0 };
  var wanted := want[1] + want[2];
  // with nothing expected there is nothing to wait for, so the whole window is
  // spent listening for what should not come
  var deadline := ReadMillisecondClock() + ( wanted ? timeout * 1000 : grace );
  var settled := 0;

  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( POLL_INTERVAL );
    if ( ev and ev.type == EVT_SPEECH and ev.msg == text )
      var idx := CInt( ev.id ) + 1;
      if ( idx >= 1 and idx <= 2 )
        found[idx] := ev;
      endif
    endif
    if ( !settled and wanted and ( !want[1] or found[1] ) and ( !want[2] or found[2] ) )
      settled := 1;
      deadline := ReadMillisecondClock() + grace;
    endif
  endwhile

  var i := 1;
  while ( i <= 2 )
    if ( want[i] and !found[i] )
      return ret_error( $"client {i - 1} never heard \"{text}\"" );
    elseif ( !want[i] and found[i] )
      return ret_error( $"client {i - 1} heard \"{text}\", which was not meant for it" );
    endif
    i := i + 1;
  endwhile
  return found;
endfunction

function single_click( id, serial )
  clientcon.sendevent( struct{ todo := "single_click", arg := serial, id := id } );
endfunction

// Waits for the shard to go quiet, so that what comes next can be told apart
// from what came before.
//
// The two things asked about below are answered with an event carrying nothing
// that distinguishes one answer from the next: a name text is the same string
// every time, and a mobile packet says only where the mobile is. Worse, a guild
// change makes the core send everyone involved out to every client again, and
// pyuo answers every mobile it is shown with a single click of its own - so a
// case that changes a guild and then asks a question finds several stale
// answers waiting for it. Clear_Event_Queue() is not enough either: it empties
// the queue, not the connection, and what the test client has already sent
// arrives afterwards. Draining until the quiet window has passed with nothing
// arriving leaves only the events a case caused itself.
function settle( quiet := SETTLE_QUIET )
  var deadline := ReadMillisecondClock() + quiet;
  while ( ReadMillisecondClock() < deadline )
    if ( Wait_For_Event( POLL_INTERVAL ) )
      deadline := ReadMillisecondClock() + quiet;
    endif
  endwhile
  return 1;
endfunction

// The colour the server drew a character's name in, asked the way a player
// asks: a single click, answered with the name as a "you see" text. Returns the
// colour, which is never 0, so a caller can tell it from an error.
function click_name_color( id, chr )
  settle();
  single_click( id, chr.serial );

  var ev := wait_for_speech( id, chr.name );
  if ( !ev )
    return ev;
  endif
  if ( CInt( ev.texttype ) != TEXTTYPE_YOU_SEE )
    return ret_error( $"the name text of {chr.name} came as texttype {ev.texttype}" );
  endif
  if ( CInt( ev.serial ) != chr.serial )
    return ret_error( $"the name text of {chr.name} was attributed to 0x{CInt( ev.serial ):x}" );
  endif
  return CInt( ev.color );
endfunction

// The notoriety the server told a client to draw a character with.
//
// It travels in the packet that puts a mobile on screen and nowhere else, so
// there is no way to be told that the server changed its mind: the character
// has to leave the client's sight and come back. Returns the notoriety, which
// is never 0.
function notoriety_of( id, chr, x, y )
  settle();
  MoveObjectToLocation( chr, 1, 1, 1, flags := MOVEOBJECT_FORCELOCATION );
  var ev := waitForClient( id, { EVT_REMOVED_OBJ } );
  if ( !ev )
    return ret_error( $"client {id} never lost sight of {chr.name}" );
  endif

  settle();
  MoveObjectToLocation( chr, x, y, 0, flags := MOVEOBJECT_FORCELOCATION );

  var deadline := ReadMillisecondClock() + 10000;
  while ( ReadMillisecondClock() < deadline )
    ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_NEW_MOBILE or CInt( ev.id ) != id or CInt( ev.serial ) != chr.serial )
      continue;
    endif
    if ( !( ev.?notoriety ) )
      return ret_error( $"the mobile packet of {chr.name} carried no notoriety" );
    endif
    return CInt( ev.notoriety );
  endwhile
  return ret_error( $"client {id} never saw {chr.name} again" );
endfunction

// Rewrites one setting of config/servspecopt.cfg and reloads it. Unlike
// party.cfg, servspecopt.cfg is re-read by ReloadConfiguration, so a case can
// look at both sides of a flag - it just has to put the file back, whatever
// happens. Returns the original contents, to hand to restore_ssopt().
function set_ssopt( key, value )
  var original := ReadFile( "::config/servspecopt.cfg" );
  if ( original == error )
    return ret_error( $"could not read config/servspecopt.cfg: {original}" );
  endif

  var rewritten := array{};
  foreach line in ( original )
    if ( Find( line, key, 1 ) != 1 )
      rewritten.append( line );
    endif
  endforeach
  rewritten.append( $"{key}={value}" );

  var res := WriteFile( "::config/servspecopt.cfg", rewritten );
  if ( res == error )
    return ret_error( $"could not write config/servspecopt.cfg: {res}" );
  endif

  ReloadConfiguration();
  return original;
endfunction

function restore_ssopt( original )
  WriteFile( "::config/servspecopt.cfg", original );
  ReloadConfiguration();
  return 1;
endfunction
