/** @file
 *
 * @par History
 */


#include "zone.h"

namespace Pol
{
namespace Core
{
Pos2d XyToZone( const Pos2d& p );
{
  return Pos2d( p.x() >> ZONE_SHIFT, p.y() >> ZONE_SHIFT );
}
}  // namespace Core
}  // namespace Pol
