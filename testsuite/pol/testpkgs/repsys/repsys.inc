// Shared fixture for the reputation system tests.
//
// The four rep characters are offline player characters, one per account, made
// by setup.src. They are player characters and not NPCs on purpose: the PC-vs-PC
// rules in repsys.cpp only run when the defender's class is CLASS_CHARACTER, and
// UObject::isa is exact class equality - an NPC is CLASS_NPC even when it stands
// in for a player. An NPC reaches the rules only through its master.
//
// They stay where createchar2 puts them, at 1,1,1 in britannia, which is clear of
// every other package's fixtures. That matters for restart_criminal_timer, which
// walks the NPCs within 32 tiles to propagate the flag.
use guilds;
use os;
use party;
use uo;
use vitals;

include "testutil";

// setup.src makes testrepsys0..testrepsys3; index 3 is the one cases are free to
// kill, so the three below can be relied on to be alive.
const REP_AMY := 0;
const REP_BOB := 1;
const REP_CARL := 2;
const REP_VICTIM := 3;

function rep_char( idx )
  var acct := FindAccount( $"testrepsys{idx}" );
  if ( !acct )
    return ret_error( $"no account testrepsys{idx}" );
  endif
  var chr := acct.getcharacter( 1 );
  if ( !chr )
    return ret_error( $"no character in account testrepsys{idx}" );
  endif
  return chr;
endfunction

// Put a character back to a clean reputation. Every list member is read into an
// array first, so clearing while walking it is safe - aggressorto and friends
// each build a fresh array on every read. The `ref` is what gets passed back in:
// a bare serial only resolves for a mobile that is logged in.
function rep_reset( chr )
  chr.setcriminal( 0 );
  chr.setmurderer( 0 );
  foreach entry in ( chr.aggressorto )
    chr.clearaggressorto( entry.ref );
  endforeach
  foreach entry in ( chr.lawfullydamaged )
    chr.clearlawfullydamagedto( entry.ref );
  endforeach
  foreach entry in ( chr.reportables )
    chr.removereportable( entry.serial, entry.gameclock );
  endforeach
  return 1;
endfunction

// Bring a character back from a kill and take its corpse with it. Resurrect is
// forced to the current location because a rep character stands at 1,1,1, which
// walkheight has no reason to accept.
function rep_revive( chr )
  if ( chr.dead )
    var res := Resurrect( chr, RESURRECT_FORCELOCATION );
    if ( !res )
      return res;
    endif
  endif
  rep_unpoison( chr );
  var corpse := chr.getcorpse();
  if ( corpse )
    DestroyItem( corpse );
  endif
  return 1;
endfunction

// The entry of `list` naming `other`, or 0. Used for aggressorto and
// lawfullydamaged, which are unordered.
function rep_entry_for( list, other )
  foreach entry in ( list )
    if ( entry.serial == other.serial )
      return entry;
    endif
  endforeach
  return 0;
endfunction

// The reputation rules are reached by acting on a character with a script
// controller set: the controller is who the core reads as the attacker, the
// damager or the healer.
//
// Damage of 0 is the one clean way to reach the attack rules on their own -
// apply_raw_damage_hundredths calls repsys_on_attack before it looks at the
// amount and returns as soon as it sees a zero, so the damage rules never run.
function rep_attack( amy, bob )
  SetScriptController( amy );
  var res := ApplyRawDamage( bob, 0, DAMAGE_USE_REPSYS, 0 );
  SetScriptController( 0 );
  return res;
endfunction

// The damage rules, reached without spending a hit point.
//
// A rep character has a hit point maximum of zero: the shard works it out from
// Strength through a MaximumFunction hook, and RecalcVitals is the only thing
// that runs it - which refuses an offline mobile with "Mobile must be online."
// So one point of raw damage takes such a character all the way to dead, and the
// death commits the to-be-reportable list the case is about to look at.
//
// Poisoning is the way in that costs nothing: MTH_SETPOISONED applies the damage
// rules for the script's controller and moves no vital at all.
function rep_damage( amy, bob )
  SetScriptController( amy );
  var res := bob.setpoisoned( 1 );
  SetScriptController( 0 );
  return res;
endfunction

// Clearing the poison is the *help* rules, so this drops the controller first -
// with none set the core skips the reputation block entirely. It also runs
// check_undamaged, which empties the to-be-reportable list, so anything a case
// wants committed has to have died before this is called.
function rep_unpoison( bob )
  SetScriptController( 0 );
  return bob.setpoisoned( 0 );
endfunction

function rep_help( amy, bob )
  SetScriptController( amy );
  var res := HealDamage( bob, 1 );
  SetScriptController( 0 );
  return res;
endfunction

// Open flat land at z 0, clear of the other packages' fixtures. The rep
// characters themselves never move - only the npcs of test_repsys_npc.src and
// test_repsys_timers.src stand here.
const REP_X := 60;
const REP_Y := 150;

function rep_make_npc( resmgn, template, offset := 0, override := 0 )
  var npc := resmgn.CreateNPCFromTemplate( $":testrepsys:{template}", REP_X + offset, REP_Y, 0,
                                           override );
  if ( !npc )
    return ret_error( $"could not create a {template}: {npc}" );
  endif
  return npc;
endfunction

const REP_GUILD_PROP := "testrepsysguild";

// Guilds are shard-global and nothing in the core removes one that still holds a
// member, an ally or an enemy, so each one made here is marked and dismantled
// rather than counted. cleanup.src drops whatever a failing case left standing.
function rep_make_guild()
  var guild := CreateGuild();
  if ( !guild )
    return ret_error( $"could not create a guild: {guild}" );
  endif
  var res := guild.setprop( REP_GUILD_PROP, 1 );
  if ( !res )
    return ret_error( $"could not mark the guild: {res}" );
  endif
  return guild;
endfunction

function rep_drop_guild( guild )
  if ( !guild )
    return 1;
  endif
  foreach member in ( guild.members )
    guild.removemember( member );
  endforeach
  foreach ally in ( guild.allyguilds )
    guild.removeallyguild( ally );
  endforeach
  foreach enemy in ( guild.enemyguilds )
    guild.removeenemyguild( enemy );
  endforeach
  return DestroyGuild( guild );
endfunction

// Removing the last member disbands a party on its own, so this is a no-op as
// often as not.
function rep_drop_party( chr )
  var party := chr.party;
  if ( !party )
    return 1;
  endif
  return DisbandParty( party );
endfunction
