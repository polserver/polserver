/** @file
 *
 * @par History
 */

#include <set>
#include <stdio.h>

#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "uofilei.h"
#include "ustruct.h"


namespace Pol::Plib
{
std::set<unsigned int> water;

bool iswater( u16 objtype )
{
  return ( objtype >= 0x1796 && objtype <= 0x17b2 );
}

void readwater()
{
  USTRUCT_IDX idxrec;

  fseek( sidxfile, 0, SEEK_SET );
  for ( int xblock = 0; xblock < 6144 / 8; ++xblock )
  {
    INFO_PRINT( "{}..", xblock );
    for ( int yblock = 0; yblock < 4096 / 8; ++yblock )
    {
      if ( fread( &idxrec, sizeof idxrec, 1, sidxfile ) != 1 )
        throw std::runtime_error( "readwater: fread(idxrec) failed." );
      int xbase = xblock * 8;
      int ybase = yblock * 8;

      if ( idxrec.length != 0xFFffFFffLu )
      {
        fseek( statfile, idxrec.offset, SEEK_SET );

        for ( idxrec.length /= 7; idxrec.length > 0; --idxrec.length )
        {
          USTRUCT_STATIC srec;
          if ( fread( &srec, sizeof srec, 1, statfile ) != 1 )
            throw std::runtime_error( "readwater: fread(srec) failed." );

          if ( srec.z == -5 && iswater( srec.graphic ) )
          {
            int x = xbase + srec.x_offset;
            int y = ybase + srec.y_offset;
            unsigned int xy = x << 16 | y;
            water.insert( xy );
          }
        }
      }
    }
  }
}
}  // namespace Pol::Plib
