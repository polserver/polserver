#ifndef POL_WORLDCHANGEREASON_H
#define POL_WORLDCHANGEREASON_H

namespace Pol
{
namespace Realms
{
enum class WorldChangeReason
{
  NpcLoad,
  NpcCreate,
  NpcDeath,

  PlayerLoad,
  PlayerEnter,
  PlayerExit,
  PlayerDeleted,

  Moved
};
}
}
#endif
