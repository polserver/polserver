// Making and unmaking the guilds the client tests need.
//
// Kept apart from guild.inc so cleanup.src can have it without dragging in the
// client connection: guild.inc pulls in speech.inc, which opens a connection of
// its own the moment it is included.
use os;
use uo;
use guilds;

include "testutil";

// Nothing in the core ever removes a guild, so every guild made here is marked
// with a cprop and cleanup.src takes apart whatever a failing case left behind.
// The mark is the client package's own: the guild package cleans up after
// itself under a different one.
const TEST_GUILD_PROP := "testclientguild";

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

// A guild holding the given characters. Its members have to be put in before a
// case starts talking: a message is routed by who is in the guild at the moment
// it is spoken.
function guild_of( name, members )
  var guild := make_guild( name );
  if ( !guild )
    return guild;
  endif

  foreach chr in ( members )
    var res := guild.addmember( chr );
    if ( !res )
      // a half-built guild is still a marked guild, and the next case would
      // find its characters in it
      drop_guild( guild );
      return ret_error( $"could not add {chr.name} to guild {name}: {res}" );
    endif
  endforeach
  return guild;
endfunction

// Strips everything DestroyGuild refuses to destroy over, then destroys it.
// Reading .members / .allyguilds / .enemyguilds hands back a fresh array every
// time, so removing while iterating is safe.
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

// Destroys every guild this package made. A guild outlives the case that made
// it and the next one would find its characters already guilded, so this runs
// before each case as well as after it.
function drop_guilds()
  foreach guild in ( ListGuilds() )
    if ( !guild.getprop( TEST_GUILD_PROP ) )
      continue;
    endif
    var res := drop_guild( guild );
    if ( !res )
      return ret_error( $"could not destroy a leftover guild: {res}" );
    endif
  endforeach
  return 1;
endfunction
