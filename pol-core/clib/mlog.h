/** @file
 *
 * @par History
 * - 2005/06/20 Shinigami: added llog (needs defined MEMORYLEAK)
 * - 2007/07/07 Shinigami: added memoryleak_debug to control specific log output (needs defined MEMORYLEAK)
 */


#ifdef WINDOWS
#include "pol_global_config_win.h"
#else
#include "pol_global_config.h"
#endif

namespace Pol {
  namespace Clib {
#ifdef MEMORYLEAK
	extern bool memoryleak_debug;
#endif

  }
}
