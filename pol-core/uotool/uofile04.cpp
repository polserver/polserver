/** @file
 *
 * @par History
 */

#include <set>
#include <stdio.h>

#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../pol/uofilei.h"
#include "../pol/ustruct.h"

namespace Pol
{
namespace Core
{
std::set<unsigned int> water;

bool iswater( u16 objtype )
{
  return ( objtype >= 0x1796 && objtype <= 0x17b2 );
}

void readwater()
{
  Core::USTRUCT_IDX idxrec;

  fseek( Core::sidxfile, 0, SEEK_SET );
  for ( int xblock = 0; xblock < 6144 / 8; ++xblock )
  {
    INFO_PRINT << xblock << "..";
    for ( int yblock = 0; yblock < 4096 / 8; ++yblock )
    {
      fread( &idxrec, sizeof idxrec, 1, Core::sidxfile );
      int xbase = xblock * 8;
      int ybase = yblock * 8;

      if ( idxrec.length != 0xFFffFFffLu )
      {
        fseek( Core::statfile, idxrec.offset, SEEK_SET );

        for ( idxrec.length /= 7; idxrec.length > 0; --idxrec.length )
        {
          Core::USTRUCT_STATIC srec;
          fread( &srec, sizeof srec, 1, Core::statfile );

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
}
}
