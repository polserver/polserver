// Shared helpers for the party tests.
//
// A party is made of Characters, not of clients: CreateParty and the PartyRef
// methods take any Character at all, and npcs are the cheapest ones to make and
// throw away. Every method that actually moves a member through the party
// refuses a character with no client, so the npcs also give the refusals a
// subject.
//
// Parties are shard-global and are written to parties.txt at shutdown, so each
// case disbands what it made - a party whose members are killed off by the
// resource manager afterwards would otherwise be saved with dangling serials.
use uo;
use os;
use party;

include "testutil";

// open flat land at z 0, the same area the npc package uses
const BASE_X := 120;
const BASE_Y := 120;

function make_member( resmgn, offset := 0 )
  var npc := resmgn.CreateNPCFromTemplate( ":testparty:party_npc", BASE_X + offset, BASE_Y, 0 );
  if ( !npc )
    return ret_error( $"could not create an npc: {npc}" );
  endif
  return npc;
endfunction

// a party of two npcs, returned as { leader, member }
function make_party( resmgn )
  var leader := make_member( resmgn );
  var member := make_member( resmgn, 2 );
  if ( !leader || !member )
    return !leader ? leader : member;
  endif

  var res := CreateParty( leader, member );
  if ( !res )
    return ret_error( $"CreateParty failed: {res}" );
  endif
  return { leader, member };
endfunction

// the serials of an array of character refs
function serials( chars )
  var res := array{};
  foreach chr in ( chars )
    res.append( chr.serial );
  endforeach
  return res;
endfunction

// disband whatever the character is still in. Removing the last member disbands
// a party on its own, so this is a no-op as often as not
function drop_party( chr )
  var party := chr.party;
  if ( !party )
    return 1;
  endif
  return DisbandParty( party );
endfunction

function drop_and_fail( chr, message )
  drop_party( chr );
  return ret_error( message );
endfunction

function drop_and_finish( chr, result )
  var res := drop_party( chr );
  if ( !res )
    return ret_error( $"could not disband the party: {res}" );
  endif
  return result;
endfunction

// 201 characters, one over what the party chat functions accept
function overlong_text()
  var text := "";
  while ( text.length() < 201 )
    text := text + "x";
  endwhile
  return text;
endfunction
