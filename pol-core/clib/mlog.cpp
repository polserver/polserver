/** @file
 *
 * @par History
 * - 2005/06/20 Shinigami: added llog (needs defined MEMORYLEAK)
 * - 2007/07/07 Shinigami: added memoryleak_debug to control specific log output (needs defined
 * MEMORYLEAK)
 */


#include "mlog.h"

namespace Pol::Clib
{
#ifdef MEMORYLEAK
bool memoryleak_debug = false;
#endif
}  // namespace Pol::Clib
