// Shared helpers for the party tests.
//
// The party system is almost entirely a client protocol: every method that moves
// a character through a party refuses one without a client, so the flows can only
// be driven from here. Everything travels in the 0xBF 0x0006 packet, in both
// directions, with a command byte telling the two apart - see PARTYCMD_* in
// communication.inc.
//
// speech.inc is included for the two characters, the client connection and the
// cliloc helpers: the party system says most of what it has to say in clilocs.
use os;
use uo;
use party;

include "testutil";
include "communication";
include "speech";

// Finds the two client characters, puts them in sight of each other and makes
// sure neither is left in a party. To be called from the program of every party
// test file.
function party_setup()
  var res := speech_setup();
  if ( !res )
    return res;
  endif
  return disband_parties();
endfunction

// Drops whatever party the two characters are in and forgets what the hooks
// recorded. Disbanding also clears the candidates, so a half-finished
// invitation goes with it.
function disband_parties()
  forget_hooks();
  foreach chr in ( { char, char2 } )
    var party := chr.party ?: chr.candidate_of_party;
    if ( !party )
      continue;
    endif
    var res := DisbandParty( party );
    if ( !res )
      return ret_error( $"could not disband a leftover party: {res}" );
    endif
  endforeach
  return 1;
endfunction

function send_party_invite( id, serial )
  clientcon.sendevent( struct{ todo := "party",
                               arg := struct{ partycmd := PARTYCMD_ADD, serial := serial },
                               id := id } );
endfunction

function send_party_remove( id, serial )
  clientcon.sendevent( struct{ todo := "party",
                               arg := struct{ partycmd := PARTYCMD_REMOVE, serial := serial },
                               id := id } );
endfunction

// Dismisses a cursor the core raised, the way pressing Escape does.
function cancel_target( id )
  clientcon.sendevent( struct{ todo := "cancel_target", id := id } );
  var ev := waitForClient( id, { EVT_CANCEL_TARGET } );
  if ( !ev )
    return ret_error( $"client {id} never answered the cancel" );
  endif
  if ( !ev.res )
    return ret_error( $"client {id} had no cursor to cancel" );
  endif
  return 1;
endfunction

function send_party_accept( id, leaderserial )
  clientcon.sendevent( struct{ todo := "party",
                               arg := struct{ partycmd := PARTYCMD_ACCEPT_INVITE,
                                              serial := leaderserial },
                               id := id } );
endfunction

function send_party_decline( id, leaderserial )
  clientcon.sendevent( struct{ todo := "party",
                               arg := struct{ partycmd := PARTYCMD_DECLINE_INVITE,
                                              serial := leaderserial },
                               id := id } );
endfunction

function send_party_msg( id, text )
  clientcon.sendevent( struct{ todo := "party",
                               arg := struct{ partycmd := PARTYCMD_MSG, text := text },
                               id := id } );
endfunction

function send_party_private_msg( id, serial, text )
  clientcon.sendevent( struct{ todo := "party",
                               arg := struct{ partycmd := PARTYCMD_MEMBER_MSG,
                                              serial := serial, text := text },
                               id := id } );
endfunction

function send_party_loot( id, canloot )
  clientcon.sendevent( struct{ todo := "party",
                               arg := struct{ partycmd := PARTYCMD_LOOT_PERMISSION,
                                              canloot := canloot },
                               id := id } );
endfunction

// Waits for a party packet of the given command for the given client. Party
// traffic is quiet enough that the command is all it takes to tell one from
// another.
function wait_for_party( id, partycmd, timeout := 10 )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_PARTY or CInt( ev.id ) != id or CInt( ev.partycmd ) != partycmd )
      continue;
    endif
    return ev;
  endwhile
  return ret_error( $"client {id} never got party command {partycmd} within {timeout}s" );
endfunction

// Same, for a packet both clients get: which of them is served first is not
// fixed, so collect them in one loop
function wait_for_parties( partycmd, timeout := 10 )
  var found := { 0, 0 };
  var deadline := ReadMillisecondClock() + timeout * 1000;

  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_PARTY or CInt( ev.partycmd ) != partycmd )
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
  return ret_error( $"client {missing} never got party command {partycmd} within {timeout}s" );
endfunction

// As with no_speech_for(), the whole window is always paid, so it is kept to a
// round trip rather than to a couple of seconds.
function no_party_for( id, partycmd, timeout := QUIET_SECS )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( POLL_INTERVAL );
    if ( !ev or ev.type != EVT_PARTY or CInt( ev.id ) != id or CInt( ev.partycmd ) != partycmd )
      continue;
    endif
    return ret_error( $"client {id} got party command {partycmd}, which was not meant for it" );
  endwhile
  return 1;
endfunction

// The hooks of config/party.cfg, which record every call in a global property
// of their own. See partyhook.src.
// (a const array has to be empty in eScript, so this is a var)
var PARTY_HOOKS := { "CanAddToParty", "CanRemoveMember", "CanLeaveParty", "OnPartyCreate", "OnAddToParty",
                     "OnLeaveParty", "OnDecline", "OnDisband", "OnLootPermChange", "OnPublicChat",
                     "OnPrivateChat", "ChangePublicChat", "ChangePrivateChat" };

// What a hook was called with the last time it ran, or an empty result if it
// has not run since the last forget_hooks()
function hook_call( name )
  return GetGlobalProperty( "#PartyHook" + name );
endfunction

function forget_hooks()
  foreach name in ( PARTY_HOOKS )
    EraseGlobalProperty( "#PartyHook" + name );
  endforeach
  EraseGlobalProperty( "#PartyRefuse" );
  EraseGlobalProperty( "#PartyChatChange" );
  return 1;
endfunction

// Makes one of the three asking hooks answer no. Only one at a time is ever
// needed, and an unset property cannot be told from a refusing 0.
function refuse_hook( name )
  SetGlobalProperty( "#PartyRefuse", name );
endfunction

// What the Change*Chat hooks answer with: a text to put in place of the
// message, or the string "block" to swallow it.
function change_chat( text )
  SetGlobalProperty( "#PartyChatChange", text );
endfunction

// What to look for, for collect_events(): a party packet or a cliloc message,
// for one of the two clients.
function want_party( id, partycmd )
  return struct{ id := id, type := EVT_PARTY, code := partycmd };
endfunction

function want_cliloc( id, num )
  return struct{ id := id, type := EVT_CLILOC, code := num };
endfunction

// Collects several awaited events in one pass, and returns them in the order
// they were asked for.
//
// One call for everything a step is meant to produce, rather than a wait per
// event: a single step of the party protocol sends packets and clilocs
// interleaved, and every wait helper here throws away what it is not looking
// for - so a second wait would find its event already gone.
function collect_events( wanted, timeout := 10 )
  var found := array{};
  var missing := len( wanted );
  var i := 1;
  while ( i <= missing )
    found.append( 0 );
    i := i + 1;
  endwhile

  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( missing and ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev )
      continue;
    endif
    i := 1;
    while ( i <= len( wanted ) )
      if ( !found[i] and event_matches( ev, wanted[i] ) )
        found[i] := ev;
        missing := missing - 1;
        break;
      endif
      i := i + 1;
    endwhile
  endwhile

  if ( missing )
    i := 1;
    while ( i <= len( wanted ) )
      if ( !found[i] )
        var want := wanted[i];
        return ret_error( $"client {want.id} never got {want.type} {want.code} within {timeout}s" );
      endif
      i := i + 1;
    endwhile
  endif
  return found;
endfunction

function event_matches( ev, want )
  if ( ev.type != want.type or CInt( ev.id ) != want.id )
    return 0;
  endif
  if ( want.type == EVT_PARTY )
    return CInt( ev.partycmd ) == want.code;
  endif
  return CInt( ev.cliloc ) == want.code;
endfunction

// Sends an invitation from client 0 to client 1 and waits for it to arrive.
// Anything left over from the case before is disbanded first.
function send_invitation()
  var res := disband_parties();
  if ( !res )
    return res;
  endif

  Clear_Event_Queue();
  send_party_invite( 0, char2.serial );
  res := wait_for_party( 1, PARTYCMD_INVITE_MEMBER );
  if ( !res )
    return party_fail( "no invitation arrived" );
  endif
  return 1;
endfunction

// Accepts the invitation that is out and waits for both member lists.
function accept_invitation()
  Clear_Event_Queue();
  send_party_accept( 1, char.serial );

  var res := wait_for_parties( PARTYCMD_ADD );
  if ( !res )
    return party_fail( res.errortext );
  endif
  return 1;
endfunction

// A party of the two clients, built the only way there is: an invitation and an
// acceptance.
function build_party()
  var res := send_invitation();
  if ( !res )
    return res;
  endif
  return accept_invitation();
endfunction

// Every case takes its party apart when it is done, whether it passed or not: a
// party outlives the case that made it, and the next one would find the two
// characters already in one.
function party_fail( message )
  disband_parties();
  return ret_error( message );
endfunction

function party_finish( result )
  var res := disband_parties();
  if ( !res )
    return res;
  endif
  return result;
endfunction

// The member list of a party packet. Numbers arrive from the client as doubles,
// so they need converting before they can be compared to a serial.
function party_members( ev )
  var res := array{};
  foreach serial in ( ev.members )
    res.append( CInt( serial ) );
  endforeach
  return res;
endfunction
