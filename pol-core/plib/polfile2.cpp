/** @file
 *
 * @par History
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */

// TODO: consider joining this file with landtile.cpp, tiles.cpp and clidata.h and renaming it all
// to something consistent.
// TODO: encapsulate the tile[] vector here

#include <string>

#include "../clib/rawtypes.h"
#include "mapcell.h"
#include "systemstate.h"
#include "tiles.h"

namespace Pol
{
namespace Plib
{
unsigned char tilelayer( unsigned short tilenum )
{
  if ( tilenum <= systemstate.config.max_tile_id )
  {
    return systemstate.tile[tilenum].layer;
  }
  else
  {
    return 0;
  }
}

char tileheight( unsigned short tilenum )
{
  if ( tilenum <= systemstate.config.max_tile_id )
  {
    char height = systemstate.tile[tilenum].height;
    if ( systemstate.tile[tilenum].flags & FLAG::GRADUAL )
      height /= 2;
    return height;
  }
  else
  {
    return 0;
  }
}

u32 tile_flags( unsigned short tilenum )
{
  if ( tilenum <= systemstate.config.max_tile_id )
  {
    return systemstate.tile[tilenum].flags;
  }
  else
  {
    return 0;
  }
}

u32 tile_uoflags( unsigned short tilenum )
{
  if ( tilenum <= systemstate.config.max_tile_id )
  {
    return systemstate.tile[tilenum].uoflags;
  }
  else
  {
    return 0;
  }
}

std::string tile_desc( unsigned short tilenum )
{
  if ( tilenum <= systemstate.config.max_tile_id )
  {
    return systemstate.tile[tilenum].desc;
  }
  else  // a multi, probably
  {
    return "multi";
  }
}

unsigned short tileweight( unsigned short tilenum )
{
  if ( tilenum <= systemstate.config.max_tile_id )
  {
    return systemstate.tile[tilenum].weight;
  }
  else
  {
    return 0;
  }
}
}  // namespace Plib
}  // namespace Pol
