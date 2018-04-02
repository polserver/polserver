/** @file
 *
 * @par History
 */


#include "zone.h"

namespace Pol
{
namespace Core
{
void XyToZone( xcoord x, ycoord y, unsigned* zonex, unsigned* zoney )
{
  *zonex = x >> ZONE_SHIFT;
  *zoney = y >> ZONE_SHIFT;
}
}
}
