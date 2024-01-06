/** @file
 *
 * @par History
 */


#include "clidata.h"
#include "uconst.h"
#include "udatfile.h"
#include "uofile.h"
#include "ustruct.h"

namespace Pol
{
namespace Plib
{
inline bool flags_standable( unsigned int flags )
{
  return ( flags & ( USTRUCT_TILE::FLAG_PLATFORM | USTRUCT_TILE::FLAG_BLOCKING ) ) ==
         USTRUCT_TILE::FLAG_PLATFORM;
}
inline bool flags_swimmable( unsigned int flags )
{
  return ( flags & ( USTRUCT_TILE::FLAG_FLOOR | USTRUCT_TILE::FLAG_LIQUID ) ) ==
         ( USTRUCT_TILE::FLAG_FLOOR | USTRUCT_TILE::FLAG_LIQUID );
}
void standheight_read( MOVEMODE movemode, StaticList& statics, unsigned short x, unsigned short y,
                       short oldz, bool* result_out, short* newz_out )
{
  short lowest_blocking_z = 128;
  short highest_blocking_z = -127;


  short mapz;
  USTRUCT_MAPINFO mi;
  getmapinfo( x, y, &mapz, &mi );
  unsigned int mapflags = landtile_uoflags_read( mi.landtile );
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

  statics.push_back( StaticRec( 0, static_cast<signed char>( mapz - 1 ), mapflags, 1 ) );

  short newz = -127;
  bool result = false;

  for ( StaticList::iterator itr = statics.begin(); itr != statics.end(); ++itr )
  {
    StaticRec& srec = ( *itr );
    unsigned int flags = srec.flags;
    signed char ztemp;
#if ENABLE_POLTEST_OUTPUT
    if ( static_debug_on )
    {
      INFO_PRINT2( "static: graphic={:#x}, z={}, ht={}", srec.graphic, int( srec.z ),
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
          INFO_PRINT2( "Setting Z to {}", int( ztemp ) );
#endif
        newz = ztemp;
        result = true;
      }
    }
  }

  if ( result )
  {
    for ( StaticList::iterator itr = statics.begin(); itr != statics.end(); ++itr )
    {
      StaticRec& srec = ( *itr );
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
          INFO_PRINT2( "static: objtype={:#x}, z={}, ht={} blocks movement to z={}", srec.graphic,
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
}  // namespace Plib
}  // namespace Pol
