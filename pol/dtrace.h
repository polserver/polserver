/*
History
=======


Notes
=======

*/

#ifndef DTRACE_H
#define DTRACE_H

#include "polcfg.h"

#define dtrace(n) if (config.debug_level < n) ; else cout

#endif
