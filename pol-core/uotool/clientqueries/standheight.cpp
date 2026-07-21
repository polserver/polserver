/** @file
 * standheight_read: walk/stand height simulation over raw client data (uotool).
 * Relocated from plib -- uoconvert does not use this.
 *
 * @par History
 */


#include "standheight.h"

#include "mapqueries.h"
#include "staticsqueries.h"  // static_debug_on

#include "../../plib/clidata.h"
#include "../../plib/clientfiles/uoclientfiles.h"
#include "../../plib/uconst.h"
#include "../../plib/udatfile.h"
#include "../../plib/ustruct.h"


namespace Pol::UoTool
{
inline bool flags_standable( unsigned int flags )
{
  using namespace Pol::Plib;
  return ( flags & ( USTRUCT_TILE::FLAG_PLATFORM | USTRUCT_TILE::FLAG_BLOCKING ) ) ==
         USTRUCT_TILE::FLAG_PLATFORM;
}
inline bool flags_swimmable( unsigned int flags )
{
  using namespace Pol::Plib;
  return ( flags & ( USTRUCT_TILE::FLAG_FLOOR | USTRUCT_TILE::FLAG_LIQUID ) ) ==
         ( USTRUCT_TILE::FLAG_FLOOR | USTRUCT_TILE::FLAG_LIQUID );
}
void standheight_read( const Pol::Plib::UoClientFiles& uof, Pol::Plib::MOVEMODE movemode,
                       Pol::Plib::StaticList& statics, unsigned short x, unsigned short y,
                       short oldz, bool* result_out, short* newz_out )
{
  using namespace Pol::Plib;
  short lowest_blocking_z = 128;
  short highest_blocking_z = -127;


  short mapz;
  USTRUCT_MAPINFO mi;
  getmapinfo( uof, x, y, &mapz, &mi );
  unsigned int mapflags = uof.landtile_uoflags_read( mi.landtile );
  if ( !mapflags )
    mapflags = /*USTRUCT_TILE::FLAG_BLOCKING|*/ USTRUCT_TILE::FLAG_PLATFORM;

  bool land_ok = ( movemode & MOVEMODE_LAND ) ? true : false;
  bool sea_ok = ( movemode & MOVEMODE_SEA ) ? true : false;

  if ( statics.empty() )
  {
    if ( land_ok && ( mapflags & ( USTRUCT_TILE::FLAG_PLATFORM ) ) )
    {
      *result_out = true;
      *newz_out = mapz;
      return;
    }
  }

  mapflags |= USTRUCT_TILE::FLAG_PLATFORM | USTRUCT_TILE::FLAG_FLOOR;

  statics.emplace_back( 0, static_cast<signed char>( mapz - 1 ), mapflags, 1 );

  short newz = -127;
  bool result = false;

  for ( auto& srec : statics )
  {
    unsigned int flags = srec.flags;
    signed char ztemp;
#if ENABLE_POLTEST_OUTPUT
    if ( static_debug_on )
    {
      INFO_PRINTLN( "static: graphic={:#x}, z={}, ht={}", srec.graphic, int( srec.z ),
                    int( srec.height ) );
    }
#endif

    ztemp = srec.z + srec.height;

    if ( srec.z < lowest_blocking_z )
      lowest_blocking_z = srec.z;
    if ( ztemp > highest_blocking_z )
      highest_blocking_z = ztemp;

    if ( ( land_ok && flags_standable( flags ) ) || ( sea_ok && flags_swimmable( flags ) ) )
    {
      if ( ( ztemp <= oldz + 7 ||
             ( ztemp <= oldz + 10 &&
               ( ( srec.flags &
                   USTRUCT_TILE::FLAG_HALF_HEIGHT ) ) ) ) &&  // not too high to step onto
           ( ztemp >= newz ) &&  // but above or same as the highest yet seen
           ( newz == -127 || srec.graphic != 0 || ztemp > newz + 18 ) )
      {  // NOTE, the >= here is so statics at ground height
         // will override a blocking map tile.
#if ENABLE_POLTEST_OUTPUT
        if ( static_debug_on )
          INFO_PRINTLN( "Setting Z to {}", int( ztemp ) );
#endif
        newz = ztemp;
        result = true;
      }
    }
  }

  if ( result )
  {
    for ( auto& srec : statics )
    {
      signed char ztemp;
      ztemp = srec.z + srec.height;

      if ( srec.z <= newz + 10 &&  // bottom of static is below top of mobile
           ztemp > newz &&         // top of static is above mobile's feet
           ( ( srec.flags & USTRUCT_TILE::FLAG_BLOCKING ) ||
             ( !land_ok && flags_standable( srec.flags ) ) ) )
      {
#if ENABLE_POLTEST_OUTPUT
        if ( static_debug_on )
        {
          INFO_PRINTLN( "static: objtype={:#x}, z={}, ht={} blocks movement to z={}", srec.graphic,
                        int( srec.z ), int( srec.height ), int( newz ) );
        }
#endif
        result = false;
        break;
      }
    }
  }

  *result_out = result;
  *newz_out = newz;
}
}  // namespace Pol::UoTool
