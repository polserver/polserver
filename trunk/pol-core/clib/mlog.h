/*
History
=======
2005/06/20 Shinigami: added llog (needs defined MEMORYLEAK)
2007/07/07 Shinigami: added memoryleak_debug to control specific log output (needs defined MEMORYLEAK)

Notes
=======

*/

namespace Pol {
  namespace Clib {
#ifdef MEMORYLEAK
	extern bool memoryleak_debug;
#endif

  }
}
