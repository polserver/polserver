/** @file
 *
 * @par History
 */


#ifndef POLTYPE_H
#define POLTYPE_H

#include "../clib/rawtypes.h"

namespace Pol::Core
{
const short ZCOORD_MIN = -128;
const short ZCOORD_MAX = +127;

// ?coord: validated to be within range.
using xcoord = unsigned short;
using ycoord = unsigned short;
using zcoord = short;

using pol_serial_t = u32;
using pol_objtype_t = u16;
}  // namespace Pol::Core

#endif
