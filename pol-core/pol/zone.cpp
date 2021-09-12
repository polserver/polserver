/** @file
 *
 * @par History
 */


#include "zone.h"

namespace Pol
{
namespace Core
{
Pos2d XyToZone( const Pos2d& pos )
{
  return Pos2d( pos.x() >> ZONE_SHIFT, pos.y() >> ZONE_SHIFT );
}
}  // namespace Core
}  // namespace Pol
