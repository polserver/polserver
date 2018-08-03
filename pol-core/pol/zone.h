/** @file
 *
 * @par History
 */


#ifndef ZONE_H
#define ZONE_H

#include <utility>

#include "poltype.h"

namespace Pol
{
namespace Core
{
const unsigned ZONE_SIZE = 4;
const unsigned ZONE_SHIFT = 2;

std::pair<unsigned, unsigned> XyToZone( xcoord x, ycoord y );

typedef unsigned short RegionId;
}  // namespace Core
}  // namespace Pol
#endif
