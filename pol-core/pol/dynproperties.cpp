#include "dynproperties.h"

namespace Pol
{
namespace Core
{
const ValueModPack ValueModPack::DEFAULT = ValueModPack();

const MovementCostMod MovementCostMod::DEFAULT = MovementCostMod( 1.0, 1.0, 1.0, 1.0 );

const ExtStatBarFollowers ExtStatBarFollowers::DEFAULT = ExtStatBarFollowers( 0, 0 );

const SkillStatCap SkillStatCap::DEFAULT = SkillStatCap( 225, 700 );

}  // namespace Core
}  // namespace Pol
