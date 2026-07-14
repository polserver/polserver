#include "dynproperties.h"


namespace Pol::Core
{
const ValueModPack ValueModPack::DEFAULT = ValueModPack{};

const MovementCostMod MovementCostMod::DEFAULT = MovementCostMod{};

const ExtStatBarFollowers ExtStatBarFollowers::DEFAULT = ExtStatBarFollowers{};

const SkillStatCap SkillStatCap::DEFAULT = SkillStatCap{ .statcap = 225, .skillcap = 700 };

}  // namespace Pol::Core
