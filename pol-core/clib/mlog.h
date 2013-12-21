/*
History
=======
2005/06/20 Shinigami: added llog (needs defined MEMORYLEAK)
2007/07/07 Shinigami: added memoryleak_debug to control specific log output (needs defined MEMORYLEAK)

Notes
=======

*/

#include "stl_inc.h"
namespace Pol {
  namespace Clib {
	extern ofstream mlog;
#ifdef MEMORYLEAK
	extern ofstream llog;
	extern bool memoryleak_debug;
#endif

  }
}
