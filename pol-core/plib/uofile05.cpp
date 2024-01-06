/** @file
 *
 * @par History
 */


#include <stdio.h>

#include "../clib/logfacility.h"
#include "uofilei.h"
#include "ustruct.h"

namespace Pol
{
namespace Plib
{
void staticsmax()
{
  unsigned int max = 0;
  USTRUCT_IDX idxrec;

  fseek( sidxfile, 0, SEEK_SET );
  for ( int xblock = 0; xblock < 6144 / 8; ++xblock )
  {
    for ( int yblock = 0; yblock < 4096 / 8; ++yblock )
    {
      if ( fread( &idxrec, sizeof idxrec, 1, sidxfile ) != 1 )
        throw std::runtime_error( "staticsmax: fread(idxrec) failed." );

      if ( idxrec.length != 0xFFffFFffLu )
      {
        if ( idxrec.length > max )
        {
          max = idxrec.length;
          INFO_PRINT2( "Max: {}, X={}, Y={}", max, xblock, yblock );
        }
      }
    }
  }
}
}  // namespace Plib
}  // namespace Pol
