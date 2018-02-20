/** @file
 *
 * @par History
 * - 2006/04/09 Shinigami: added uoconvert.cfg flag ShowRoofAndPlatformWarning
 */


#ifndef PLIB_MAPFUNC_H
#define PLIB_MAPFUNC_H

#include <string>

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Clib
{
class ConfigElem;
}
namespace Plib
{
u32 polflags_from_tileflags( unsigned short tile, u32 uoflags, bool use_no_shoot,
                             bool LOS_through_windows );
u32 polflags_from_landtileflags( unsigned short tile, u32 lt_flags );

unsigned int readflags( Clib::ConfigElem& elem );
std::string flagstr( unsigned int flags );
std::string flagdescs();
}
}
#endif
