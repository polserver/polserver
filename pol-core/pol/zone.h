/** @file
 *
 * @par History
 */


#ifndef ZONE_H
#define ZONE_H

#include <utility>

#include "base/position.h"

namespace Pol
{
namespace Core
{
const unsigned ZONE_SIZE = 4;
const unsigned ZONE_SHIFT = 2;

Pos2d XyToZone( const Pos2d& p );

typedef unsigned short RegionId;
}  // namespace Core
}  // namespace Pol
#endif
