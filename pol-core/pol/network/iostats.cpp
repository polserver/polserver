/** @file
 *
 * @par History
 */


#include "iostats.h"

#include <cstring>

namespace Pol
{
namespace Network
{
IOStats::IOStats()
{
  memset( &sent, 0, sizeof sent );
  memset( &received, 0, sizeof received );
}
}
}
