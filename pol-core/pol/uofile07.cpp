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
namespace Core
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
        INFO_PRINT << "static: " << int( srec->x_offset ) << " " << int( srec->y_offset ) << " "
                   << int( srec->z ) << " 0x" << fmt::hexu( srec->graphic ) << "\n";
      }
#endif

      if ( ( srec->x_offset == x_offset ) && ( srec->y_offset == y_offset ) &&
           ( tile_uoflags( srec->graphic ) & USTRUCT_TILE::FLAG_WALKBLOCK ) )
      {
        vec.push_back( StaticRec( srec->graphic, srec->z ) );
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
        INFO_PRINT << "static: " << int( srec->x_offset ) << " " << int( srec->y_offset ) << " "
                   << int( srec->z ) << " 0x" << fmt::hexu( srec->graphic ) << "\n";
      }
#endif

      if ( ( srec->x_offset == x_offset ) && ( srec->y_offset == y_offset ) &&
           ( tile_uoflags( srec->graphic ) & flags ) )
      {
        vec.push_back( StaticRec( srec->graphic, srec->z ) );
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
        INFO_PRINT << "static: " << int( srec->x_offset ) << " " << int( srec->y_offset ) << " "
                   << int( srec->z ) << " 0x" << fmt::hexu( srec->graphic ) << "\n";
      }
#endif

      if ( ( srec->x_offset == x_offset ) && ( srec->y_offset == y_offset ) )
      {
        vec.push_back( StaticRec( srec->graphic, srec->z ) );
      }
    }
  }
}
}
}
