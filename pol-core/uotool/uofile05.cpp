/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../pol/polcfg.h"
#include "../pol/udatfile.h"
#include "../pol/ustruct.h"

#include "../pol/uofile.h"
#include "../pol/uofilei.h"

namespace Pol {
  namespace Core {
    void staticsmax()
    {
      unsigned int max = 0;
      USTRUCT_IDX idxrec;

      fseek( sidxfile, 0, SEEK_SET );
      for ( int xblock = 0; xblock < 6144 / 8; ++xblock )
      {
        for ( int yblock = 0; yblock < 4096 / 8; ++yblock )
        {
          fread( &idxrec, sizeof idxrec, 1, sidxfile );

          if ( idxrec.length != 0xFFffFFffLu )
          {
            if ( idxrec.length > max )
            {
              max = idxrec.length;
              cout << "Max: " << max << ", X=" << xblock << ", Y=" << yblock << endl;
            }
          }
        }
      }
    }
  }

}