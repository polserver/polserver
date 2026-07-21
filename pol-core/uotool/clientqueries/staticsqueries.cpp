/** @file
 * Per-tile statics queries + the staidx staticsmax diagnostic (uotool).
 * Relocated from plib -- uoconvert uses UoClientFiles::readstatics_block instead.
 *
 * @par History
 * - 2005/01/17 Shinigami: readstaticblock - modified passert reason
 * - 2005/07/05 Shinigami: added uoconvert.cfg option *StaticsPerBlock
 */


#include <cstdio>
#include <stdexcept>
#include <vector>

#include "staticsqueries.h"

#include "../../clib/logfacility.h"
#include "../../clib/strutil.h"
#include "../../plib/clientfiles/uoclientfiles.h"
#include "../../plib/udatfile.h"
#include "../../plib/ustruct.h"


namespace Pol::UoTool
{
bool static_debug_on = false;

void readstatics( const Pol::Plib::UoClientFiles& uof, Pol::Plib::StaticList& vec, unsigned short x,
                  unsigned short y )
{
  using namespace Pol::Plib;
  const std::vector<USTRUCT_STATIC>& srecarr = uof.getstaticblock( x, y );

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
         ( uof.tile_uoflags_read( srec.graphic ) & USTRUCT_TILE::FLAG_WALKBLOCK ) )
    {
      vec.emplace_back( srec.graphic, srec.z, uof.tile_uoflags_read( srec.graphic ),
                        uof.tileheight_read( srec.graphic ) );
    }
  }
}
void readstatics( const Pol::Plib::UoClientFiles& uof, Pol::Plib::StaticList& vec, unsigned short x,
                  unsigned short y, unsigned int flags )
{
  using namespace Pol::Plib;
  const std::vector<USTRUCT_STATIC>& srecarr = uof.getstaticblock( x, y );

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
      const unsigned int uoflags = uof.tile_uoflags_read( srec.graphic );
      if ( uoflags & flags )
        vec.emplace_back( srec.graphic, srec.z, uoflags, uof.tileheight_read( srec.graphic ) );
    }
  }
}

void readallstatics( const Pol::Plib::UoClientFiles& uof, Pol::Plib::StaticList& vec,
                     unsigned short x, unsigned short y )
{
  using namespace Pol::Plib;
  const std::vector<USTRUCT_STATIC>& srecarr = uof.getstaticblock( x, y );

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
      vec.emplace_back( srec.graphic, srec.z, uof.tile_uoflags_read( srec.graphic ),
                        uof.tileheight_read( srec.graphic ) );
    }
  }
}

void staticsmax( const Pol::Plib::UoClientFiles& uof )
{
  using namespace Pol::Plib;
  unsigned int max = 0;
  USTRUCT_IDX idxrec;

  fseek( uof.sidxfile, 0, SEEK_SET );
  for ( int xblock = 0; xblock < 6144 / 8; ++xblock )
  {
    for ( int yblock = 0; yblock < 4096 / 8; ++yblock )
    {
      if ( fread( &idxrec, sizeof idxrec, 1, uof.sidxfile ) != 1 )
        throw std::runtime_error( "staticsmax: fread(idxrec) failed." );

      if ( idxrec.length != 0xFFffFFffLu )
      {
        if ( idxrec.length > max )
        {
          max = idxrec.length;
          INFO_PRINTLN( "Max: {}, X={}, Y={}", max, xblock, yblock );
        }
      }
    }
  }
}
}  // namespace Pol::UoTool
