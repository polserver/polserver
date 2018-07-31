/** @file
 *
 * @par History
 */


#include "zone.h"

namespace Pol
{
namespace Core
{
std::pair<unsigned, unsigned> XyToZone( xcoord x, ycoord y )
{
  return std::make_pair<unsigned, unsigned>( x >> ZONE_SHIFT, y >> ZONE_SHIFT );
}
}  // namespace Core
}  // namespace Pol
