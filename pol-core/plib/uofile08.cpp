/** @file
 *
 * @par History
 * - 2005/01/13 Shinigami: safe_getmapinfo - add missing checks for x and y
 * - 2005/01/23 Shinigami: rawmapinfo - used constant WORLD_X/Y... bad idea
 */


#include <cstdio>
#include <map>
#include <stdexcept>
#include <stdlib.h>

#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "clidata.h"
#include "poltype.h"
#include "uofile.h"
#include "uofilei.h"
#include "ustruct.h"

#include "RawMap.h"


namespace Pol::Plib
{
unsigned int num_map_patches = 0;
static RawMap rawmap;
static bool rawmap_ready = false;

void read_map_difs()
{
  num_map_patches = rawmap.load_map_difflist( mapdifl_file );
}

static signed char rawmapinfo( unsigned short x, unsigned short y, USTRUCT_MAPINFO* gi )
{
  // UoTool has a serious problem of not loading data before using this function...
  if ( !rawmap_ready )
    rawmapfullread();

  return rawmap.rawinfo( x, y, gi );
}

void rawmapfullread()
{
  rawmap.set_bounds( uo_map_width, uo_map_height );

  unsigned int blocks = 0;
  if ( mapfile == nullptr )
    blocks = rawmap.load_full_map( uo_mapid, uopmapfile );
  else
    blocks = rawmap.load_full_map( mapfile, mapdif_file );

  if ( blocks )
    rawmap_ready = true;
}

/*

    MAP(x,y)        MAP
    ^
    \--CHARACTER(x,y)

    MAP             MAP

    We use the diagonal with the smallest differential..

    Simple averaging doesn't seem to work near cliffs and such.
    poltest has a unit test for this.
    */
bool groundheight_read( unsigned short x, unsigned short y, short* z )
{
  USTRUCT_MAPINFO md, mi;
  short z1, z2, z3, z4;  // quadrants

  z1 = rawmapinfo( x + 1, y, &md );
  z2 = rawmapinfo( x, y, &mi );
  z3 = rawmapinfo( x, y + 1, &md );
  z4 = rawmapinfo( x + 1, y + 1, &md );

  if ( abs( z1 - z3 ) < abs( z2 - z4 ) )
    *z = ( z1 + z3 ) / 2;
  else
    *z = ( z2 + z4 ) / 2;

  if ( mi.landtile == 0x2 )  // it's a nodraw tile
    *z = Core::ZCOORD_MIN;

  return ( ( mi.landtile < 0x4000 ) &&
           ( ( landtile_uoflags_read( mi.landtile ) & USTRUCT_TILE::FLAG_BLOCKING ) == 0 ) );
}

void getmapinfo( unsigned short x, unsigned short y, short* z, USTRUCT_MAPINFO* mi )
{
  USTRUCT_MAPINFO md;
  short z1, z2, z3, z4;  // quadrants

  z1 = rawmapinfo( x + 1, y, &md );
  z2 = rawmapinfo( x, y, mi );
  z3 = rawmapinfo( x, y + 1, &md );
  z4 = rawmapinfo( x + 1, y + 1, &md );

  short zsum;

  if ( abs( z1 - z3 ) < abs( z2 - z4 ) )
  {
    zsum = ( z1 + z3 );
  }
  else
  {
    zsum = ( z2 + z4 );
  }
  if ( zsum >= 0 )
  {
    *z = zsum / 2;
  }
  else
  {
    *z = ( zsum - 1 ) / 2;
  }
}

void safe_getmapinfo( unsigned short x, unsigned short y, short* z, USTRUCT_MAPINFO* mi )
{
  USTRUCT_MAPINFO md;
  short z1, z2, z3, z4;  // quadrants

  if ( x >= uo_map_width )
    x = uo_map_width - 1;
  if ( y >= uo_map_height )
    y = uo_map_height - 1;
  unsigned short xp = x + 1, yp = y + 1;
  if ( xp >= uo_map_width )
    xp = uo_map_width - 1;
  if ( yp >= uo_map_height )
    yp = uo_map_height - 1;
  z1 = rawmapinfo( xp, y, &md );
  z2 = rawmapinfo( x, y, mi );
  z3 = rawmapinfo( x, yp, &md );
  z4 = rawmapinfo( xp, yp, &md );

  short zsum;

  if ( abs( z1 - z3 ) < abs( z2 - z4 ) )
  {
    zsum = ( z1 + z3 );
  }
  else
  {
    zsum = ( z2 + z4 );
  }
  if ( zsum >= 0 )
  {
    *z = zsum / 2;
  }
  else
  {
    *z = ( zsum - 1 ) / 2;
  }
}
}  // namespace Pol::Plib
