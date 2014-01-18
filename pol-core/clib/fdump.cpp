/*
History
=======

Notes
=======

*/

#include <ctype.h>

#include "fdump.h"
#include "logfacility.h"

namespace Pol {
  namespace Clib {

    void dump16( fmt::Writer& writer, const unsigned char *s, int len )
    {
      int i;
      for ( i = 0; i < 16; i++ )
      {
        if ( i < len )
          writer.Format( "{:02x} " ) << (int)s[i];
        else
          writer << "   ";
        if ( i == 7 )
          writer << " ";
      }
      writer << "  ";

      for ( i = 0; i < 16; i++ )
      {
        if ( i >= len || !isprint( s[i] ) )
          writer << '.';
        else
          writer.Format("{}") << s[i];

        if ( i == 7 )
          writer << ' ';
      }
      writer << '\n';
    }

    void fdump( fmt::Writer& writer, const void *data, int len )
    {
      int i;
      const unsigned char *s = (const unsigned char *)data;

      for ( i = 0; i < len; i += 16 )
      {
        int nprint = len - i;
        if ( nprint > 16 ) nprint = 16;
        writer.Format( "{:04x}" ) << i;
        dump16( writer, &s[i], nprint );
      }
    }
  }
}
