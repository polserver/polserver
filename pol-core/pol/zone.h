/** @file
 *
 * @par History
 */


#ifndef ZONE_H
#define ZONE_H

#include <utility>
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

Pos2d XyToZone( const Pos2d& p );

typedef unsigned short RegionId;

typedef std::vector<Mobile::Character*> ZoneCharacters;
typedef std::vector<Multi::UMulti*> ZoneMultis;
typedef std::vector<Items::Item*> ZoneItems;

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
