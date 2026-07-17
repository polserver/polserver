/** @file
 *
 * @par History
 */


#include "clidata.h"
#include "udatfile.h"
#include "uofile.h"
#include "ustruct.h"


namespace Pol::Plib
{
extern bool static_debug_on;

// I'd put these in an anonymous namespace, but the debugger can't see 'em...at least not easily.

void readstatics( StaticList& vec, unsigned short x, unsigned short y )
{
  const std::vector<USTRUCT_STATIC>& srecarr = getstaticblock( x, y );

  unsigned short x_offset, y_offset;
  x_offset = x & 0x7;
  y_offset = y & 0x7;

  for ( const auto& srec : srecarr )
  {
#if ENABLE_POLTEST_OUTPUT
    if ( static_debug_on )
    {
      INFO_PRINTLN( "static: {} {} {} {}", int( srec.x_offset ), int( srec.y_offset ),
                    int( srec.z ), srec.graphic );
    }
#endif

    if ( ( srec.x_offset == x_offset ) && ( srec.y_offset == y_offset ) &&
         ( tile_uoflags_read( srec.graphic ) & USTRUCT_TILE::FLAG_WALKBLOCK ) )
    {
      vec.emplace_back( srec.graphic, srec.z, tile_uoflags_read( srec.graphic ),
                        tileheight_read( srec.graphic ) );
    }
  }
}
void readstatics( StaticList& vec, unsigned short x, unsigned short y, unsigned int flags )
{
  const std::vector<USTRUCT_STATIC>& srecarr = getstaticblock( x, y );

  unsigned short x_offset, y_offset;
  x_offset = x & 0x7;
  y_offset = y & 0x7;

  for ( const auto& srec : srecarr )
  {
#if ENABLE_POLTEST_OUTPUT
    if ( static_debug_on )
    {
      INFO_PRINTLN( "static: {} {} {} {:#x}", int( srec.x_offset ), int( srec.y_offset ),
                    int( srec.z ), srec.graphic );
    }
#endif

    if ( ( srec.x_offset == x_offset ) && ( srec.y_offset == y_offset ) )
    {
      const unsigned int uoflags = tile_uoflags_read( srec.graphic );
      if ( uoflags & flags )
        vec.emplace_back( srec.graphic, srec.z, uoflags, tileheight_read( srec.graphic ) );
    }
  }
}
void readstatics_block( StaticBuckets& buckets, unsigned short x, unsigned short y,
                        unsigned int flags )
{
  for ( auto& bucket : buckets )
    bucket.clear();

  const std::vector<USTRUCT_STATIC>& srecarr = getstaticblock( x, y );

  for ( const auto& srec : srecarr )
  {
    // Records with offsets outside the 8x8 block never match any tile in the
    // per-tile readstatics() scan; drop them here the same way.
    if ( srec.x_offset < 0 || srec.x_offset >= static_cast<int>( STATICBLOCK_CHUNK ) ||
         srec.y_offset < 0 || srec.y_offset >= static_cast<int>( STATICBLOCK_CHUNK ) )
      continue;

    const unsigned int uoflags = tile_uoflags_read( srec.graphic );
    if ( uoflags & flags )
      buckets[srec.x_offset * STATICBLOCK_CHUNK + srec.y_offset].emplace_back(
          srec.graphic, srec.z, uoflags, tileheight_read( srec.graphic ) );
  }
}

void readallstatics( StaticList& vec, unsigned short x, unsigned short y )
{
  const std::vector<USTRUCT_STATIC>& srecarr = getstaticblock( x, y );

  unsigned short x_offset, y_offset;
  x_offset = x & 0x7;
  y_offset = y & 0x7;

  for ( const auto& srec : srecarr )
  {
#if ENABLE_POLTEST_OUTPUT
    if ( static_debug_on )
    {
      INFO_PRINTLN( "static: {} {} {} {:#x}", int( srec.x_offset ), int( srec.y_offset ),
                    int( srec.z ), srec.graphic );
    }
#endif

    if ( ( srec.x_offset == x_offset ) && ( srec.y_offset == y_offset ) )
    {
      vec.emplace_back( srec.graphic, srec.z, tile_uoflags_read( srec.graphic ),
                        tileheight_read( srec.graphic ) );
    }
  }
}
}  // namespace Pol::Plib
