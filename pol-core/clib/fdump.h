/** @file
 *
 * @par History
 */


#ifndef CLIB_FDUMP_H
#define CLIB_FDUMP_H

#include "../../lib/format/format.h"

namespace Pol
{
namespace Clib
{
void fdump( fmt::Writer& writer, const void* data, int len );
}
}


#endif  // CLIB_FDUMP_H
