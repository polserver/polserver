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

    if ( ( srec.x_offset == x_offset ) && ( srec.y_offset == y_offset ) &&
         ( tile_uoflags_read( srec.graphic ) & flags ) )
    {
      vec.emplace_back( srec.graphic, srec.z, tile_uoflags_read( srec.graphic ),
                        tileheight_read( srec.graphic ) );
    }
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
