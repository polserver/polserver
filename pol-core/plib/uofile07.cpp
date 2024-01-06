/** @file
 *
 * @par History
 */


#include "clidata.h"
#include "udatfile.h"
#include "uofile.h"
#include "ustruct.h"

namespace Pol
{
namespace Plib
{
extern bool static_debug_on;

// I'd put these in an anonymous namespace, but the debugger can't see 'em...at least not easily.

void readstatics( StaticList& vec, unsigned short x, unsigned short y )
{
  std::vector<USTRUCT_STATIC> srecarr;
  int nrec;

  readstaticblock( &srecarr, &nrec, x, y );
  if ( nrec )
  {
    unsigned short x_offset, y_offset;
    x_offset = x & 0x7;
    y_offset = y & 0x7;

    for ( int i = 0; i < nrec; ++i )
    {
      USTRUCT_STATIC* srec = &srecarr[i];

#if ENABLE_POLTEST_OUTPUT
      if ( static_debug_on )
      {
        INFO_PRINT2( "static: {} {} {} {}", int( srec->x_offset ), int( srec->y_offset ),
                     int( srec->z ), srec->graphic );
      }
#endif

      if ( ( srec->x_offset == x_offset ) && ( srec->y_offset == y_offset ) &&
           ( tile_uoflags_read( srec->graphic ) & USTRUCT_TILE::FLAG_WALKBLOCK ) )
      {
        vec.push_back( StaticRec( srec->graphic, srec->z, tile_uoflags_read( srec->graphic ),
                                  tileheight_read( srec->graphic ) ) );
      }
    }
  }
}
void readstatics( StaticList& vec, unsigned short x, unsigned short y, unsigned int flags )
{
  std::vector<USTRUCT_STATIC> srecarr;
  int nrec;

  readstaticblock( &srecarr, &nrec, x, y );
  if ( nrec )
  {
    unsigned short x_offset, y_offset;
    x_offset = x & 0x7;
    y_offset = y & 0x7;

    for ( int i = 0; i < nrec; ++i )
    {
      USTRUCT_STATIC* srec = &srecarr[i];

#if ENABLE_POLTEST_OUTPUT
      if ( static_debug_on )
      {
        INFO_PRINT2( "static: {} {} {} {:#x}", int( srec->x_offset ), int( srec->y_offset ),
                     int( srec->z ), srec->graphic );
      }
#endif

      if ( ( srec->x_offset == x_offset ) && ( srec->y_offset == y_offset ) &&
           ( tile_uoflags_read( srec->graphic ) & flags ) )
      {
        vec.push_back( StaticRec( srec->graphic, srec->z, tile_uoflags_read( srec->graphic ),
                                  tileheight_read( srec->graphic ) ) );
      }
    }
  }
}
void readallstatics( StaticList& vec, unsigned short x, unsigned short y )
{
  std::vector<USTRUCT_STATIC> srecarr;
  int nrec;

  readstaticblock( &srecarr, &nrec, x, y );
  if ( nrec )
  {
    unsigned short x_offset, y_offset;
    x_offset = x & 0x7;
    y_offset = y & 0x7;

    for ( int i = 0; i < nrec; ++i )
    {
      USTRUCT_STATIC* srec = &srecarr[i];

#if ENABLE_POLTEST_OUTPUT
      if ( static_debug_on )
      {
        INFO_PRINT2( "static: {} {} {} {:#x}", int( srec->x_offset ), int( srec->y_offset ),
                     int( srec->z ), srec->graphic );
      }
#endif

      if ( ( srec->x_offset == x_offset ) && ( srec->y_offset == y_offset ) )
      {
        vec.push_back( StaticRec( srec->graphic, srec->z, tile_uoflags_read( srec->graphic ),
                                  tileheight_read( srec->graphic ) ) );
      }
    }
  }
}
}  // namespace Plib
}  // namespace Pol
