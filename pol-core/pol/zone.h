/** @file
 *
 * @par History
 */


#ifndef ZONE_H
#define ZONE_H
#include <vector>

#include "base/position.h"

namespace Pol
{
namespace Mobile
{
class Character;
}
namespace Multi
{
class UMulti;
}
namespace Items
{
class Item;
}
namespace Core
{
const unsigned ZONE_SIZE = 4;
const unsigned ZONE_SHIFT = 2;

Pos2d XyToZone( const Pos2d& pos );

using RegionId = unsigned short;

// world
using ZoneCharacters = std::vector<Mobile::Character*>;
using ZoneMultis = std::vector<Multi::UMulti*>;
using ZoneItems = std::vector<Items::Item*>;

struct Zone
{
  ZoneCharacters characters;
  ZoneCharacters npcs;
  ZoneItems items;
  ZoneMultis multis;
};

}  // namespace Core
}  // namespace Pol
#endif
