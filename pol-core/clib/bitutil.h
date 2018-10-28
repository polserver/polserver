/** @file
 *
 * @par History
 */


#ifndef CLIB_BITUTIL_H
#define CLIB_BITUTIL_H

#include "rawtypes.h"

namespace Pol
{
namespace Clib
{
inline bool flags_set( u32 value, u32 flags )
{
  return ( value & flags ) == flags;
}

inline bool flags_clear( u32 value, u32 flags )
{
  return ( ~value & flags ) == flags;
}
}  // namespace Clib
}  // namespace Pol
#endif
