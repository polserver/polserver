/** @file
 * Raw-map queries with neighbour z-averaging (uotool), over a loaded
 * UoClientFiles raw map. Relocated from plib -- uoconvert does not use these.
 *
 * @par History
 * - 2005/01/13 Shinigami: safe_getmapinfo - add missing checks for x and y
 * - 2005/01/23 Shinigami: rawmapinfo - used constant WORLD_X/Y... bad idea
 */


#include <stdlib.h>

#include "mapqueries.h"

#include "../../plib/clidata.h"
#include "../../plib/clientfiles/uoclientfiles.h"
#include "../../plib/poltype.h"
#include "../../plib/ustruct.h"
#include "../../clib/passert.h"


namespace Pol::UoTool
{
namespace
{
signed char rawmapinfo( const Pol::Plib::UoClientFiles& uof, unsigned short x, unsigned short y,
                        Pol::Plib::USTRUCT_MAPINFO* gi )
{
  passert_always( uof.rawmap_ready );  // caller must rawmapfullread() first
  return uof.rawmap.rawinfo( x, y, gi );
}
}  // namespace

/*

    MAP(x,y)        MAP
    ^
    \--CHARACTER(x,y)

    MAP             MAP

    We use the diagonal with the smallest differential..

    Simple averaging doesn't seem to work near cliffs and such.
    poltest has a unit test for this.
    */
bool groundheight_read( const Pol::Plib::UoClientFiles& uof, unsigned short x, unsigned short y,
                        short* z )
{
  using namespace Pol::Plib;
  USTRUCT_MAPINFO md, mi;
  short z1, z2, z3, z4;  // quadrants

  z1 = rawmapinfo( uof, x + 1, y, &md );
  z2 = rawmapinfo( uof, x, y, &mi );
  z3 = rawmapinfo( uof, x, y + 1, &md );
  z4 = rawmapinfo( uof, x + 1, y + 1, &md );

  if ( abs( z1 - z3 ) < abs( z2 - z4 ) )
    *z = ( z1 + z3 ) / 2;
  else
    *z = ( z2 + z4 ) / 2;

  if ( mi.landtile == 0x2 )  // it's a nodraw tile
    *z = Core::ZCOORD_MIN;

  return ( ( mi.landtile < 0x4000 ) &&
           ( ( uof.landtile_uoflags_read( mi.landtile ) & USTRUCT_TILE::FLAG_BLOCKING ) == 0 ) );
}

void getmapinfo( const Pol::Plib::UoClientFiles& uof, unsigned short x, unsigned short y, short* z,
                 Pol::Plib::USTRUCT_MAPINFO* mi )
{
  using namespace Pol::Plib;
  USTRUCT_MAPINFO md;
  short z1, z2, z3, z4;  // quadrants

  z1 = rawmapinfo( uof, x + 1, y, &md );
  z2 = rawmapinfo( uof, x, y, mi );
  z3 = rawmapinfo( uof, x, y + 1, &md );
  z4 = rawmapinfo( uof, x + 1, y + 1, &md );

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

void safe_getmapinfo( const Pol::Plib::UoClientFiles& uof, unsigned short x, unsigned short y,
                      short* z, Pol::Plib::USTRUCT_MAPINFO* mi )
{
  using namespace Pol::Plib;
  USTRUCT_MAPINFO md;
  short z1, z2, z3, z4;  // quadrants

  if ( x >= uof.uo_map_width )
    x = uof.uo_map_width - 1;
  if ( y >= uof.uo_map_height )
    y = uof.uo_map_height - 1;
  unsigned short xp = x + 1, yp = y + 1;
  if ( xp >= uof.uo_map_width )
    xp = uof.uo_map_width - 1;
  if ( yp >= uof.uo_map_height )
    yp = uof.uo_map_height - 1;
  z1 = rawmapinfo( uof, xp, y, &md );
  z2 = rawmapinfo( uof, x, y, mi );
  z3 = rawmapinfo( uof, x, yp, &md );
  z4 = rawmapinfo( uof, xp, yp, &md );

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
}  // namespace Pol::UoTool
