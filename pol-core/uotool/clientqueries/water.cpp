/** @file
 * Water-tile scan + range predicate (uotool). Relocated from plib; the water set
 * readwater used to fill had no readers, so it was deleted with the plib member.
 *
 * @par History
 */


#include <stdexcept>
#include <stdio.h>

#include "water.h"

#include "../../clib/logfacility.h"
#include "../../clib/rawtypes.h"
#include "../../plib/clientfiles/uoclientfiles.h"
#include "../../plib/ustruct.h"


namespace Pol::UoTool
{
bool iswater( u16 objtype )
{
  return ( objtype >= 0x1796 && objtype <= 0x17b2 );
}

void readwater( const Pol::Plib::UoClientFiles& uof )
{
  using namespace Pol::Plib;
  USTRUCT_IDX idxrec;

  fseek( uof.staidx_file(), 0, SEEK_SET );
  for ( int xblock = 0; xblock < 6144 / 8; ++xblock )
  {
    INFO_PRINT( "{}..", xblock );
    for ( int yblock = 0; yblock < 4096 / 8; ++yblock )
    {
      if ( fread( &idxrec, sizeof idxrec, 1, uof.staidx_file() ) != 1 )
        throw std::runtime_error( "readwater: fread(idxrec) failed." );

      if ( idxrec.length != 0xFFffFFffLu )
      {
        fseek( uof.statics_file(), idxrec.offset, SEEK_SET );

        for ( idxrec.length /= 7; idxrec.length > 0; --idxrec.length )
        {
          USTRUCT_STATIC srec;
          if ( fread( &srec, sizeof srec, 1, uof.statics_file() ) != 1 )
            throw std::runtime_error( "readwater: fread(srec) failed." );
          // The set this scan used to populate (z == -5 && iswater) had no
          // consumers, so it was removed; the traversal is kept for parity.
        }
      }
    }
  }
}
}  // namespace Pol::UoTool
