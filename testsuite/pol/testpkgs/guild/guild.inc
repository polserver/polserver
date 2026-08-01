// Shared helpers for the guild tests.
//
// Guilds are shard-global and nothing in the core ever removes one: a guild
// that still has a member, an ally or an enemy refuses to be destroyed, and
// testrestart adds three of its own. So the tests never count guilds, they only
// ask about the ones they made, and every guild made here is marked with a cprop
// so cleanup.src can dismantle whatever a failing test left behind.
use uo;
use os;
use guilds;

include "testutil";

// open flat land at z 0, the same area the npc package uses
const BASE_X := 120;
const BASE_Y := 120;

const TEST_GUILD_PROP := "testguild";

function make_guild( name )
  var guild := CreateGuild();
  if ( !guild )
    return ret_error( $"could not create a guild: {guild}" );
  endif

  var res := guild.setprop( TEST_GUILD_PROP, name );
  if ( !res )
    return ret_error( $"could not mark guild {name}: {res}" );
  endif
  return guild;
endfunction

// a member to put in a guild - membership is a property of Character, so an npc
// is the cheapest one to make and throw away
function make_member( resmgn, offset := 0 )
  var npc := resmgn.CreateNPCFromTemplate( ":testguild:guild_npc", BASE_X + offset, BASE_Y, 0 );
  if ( !npc )
    return ret_error( $"could not create an npc: {npc}" );
  endif
  return npc;
endfunction

// kill an npc and clear the corpse it leaves, the way the resource manager
// would. The object is gone from the object hash a moment later, which is what
// makes a guild prune its serial.
function kill_member( npc )
  var x := npc.x, y := npc.y, z := npc.z, serial := npc.serial;
  var res := npc.kill();
  if ( !res )
    return ret_error( $"could not kill 0x{serial:x}: {res}" );
  endif

  // 0x2006 is the corpse container
  foreach corpse in ( ListItemsNearLocationOfType( x, y, z, 0, 0x2006 ) )
    if ( corpse.ownerserial == serial )
      DestroyItem( corpse );
    endif
  endforeach
  return 1;
endfunction

// the member list is only pruned when it is read, and the killed object needs a
// moment to leave the object hash
function wait_for_members( guild, count, timeout := 2000 )
  var start := ReadMillisecondClock();
  var have := len( guild.members );
  while ( ReadMillisecondClock() - start < timeout )
    if ( have == count )
      return 1;
    endif
    sleepms( 10 );
    have := len( guild.members );
  endwhile
  return ret_error( $"{have} members after {timeout}ms, expected {count}" );
endfunction

// strip everything DestroyGuild refuses to destroy over, then destroy it.
// reading .members / .allyguilds / .enemyguilds yields a fresh array every time,
// so removing while iterating is safe
function drop_guild( guild )
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

// the ids of an array of guild refs, for comparing memberships
function guild_ids( guilds )
  var ids := array{};
  foreach guild in ( guilds )
    ids.append( guild.guildid );
  endforeach
  return ids;
endfunction

// the serials of an array of character refs
function serials( chars )
  var res := array{};
  foreach chr in ( chars )
    res.append( chr.serial );
  endforeach
  return res;
endfunction
