/*
History
=======


Notes
=======

*/

#ifndef DTRACE_H
#define DTRACE_H

#include "polcfg.h"
namespace Pol {
  namespace Core {

  }
#define dtrace(n) if (Core::config.debug_level < n) ; else cout
}
#endif
