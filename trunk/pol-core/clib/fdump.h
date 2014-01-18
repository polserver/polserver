/*
History
=======

Notes
=======

*/

#ifndef __FDUMP_H
#define __FDUMP_H

#include "../../lib/format/format.h"

namespace Pol {
  namespace Clib {

    void fdump( fmt::Writer& writer, const void *data, int len );
  }
}


#endif
