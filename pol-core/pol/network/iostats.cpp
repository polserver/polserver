/** @file
 *
 * @par History
 */


#include "iostats.h"

#include <cstring>


namespace Pol::Network
{
IOStats::IOStats()
{
  memset( &sent, 0, sizeof sent );
  memset( &received, 0, sizeof received );
}
}  // namespace Pol::Network
