/** @file
 *
 * @par History
 * - 2007/05/04 Shinigami: added region_name() to support UO::GetRegionName*
 */


#pragma once

#include <string>

#include "regions/region.h"
#include "zone.h"

namespace Pol::Clib
{
class ConfigElem;
}
namespace Pol::Network
{
class Client;
}
namespace Pol::Mobile
{
class Character;
}

namespace Pol::Core
{
void read_justice_zones();

class JusticeRegion : public Region
{
  typedef Region base;

public:
  JusticeRegion( Clib::ConfigElem& elem, RegionId id );

  const std::string& region_name() const;
  const std::string& entertext() const;
  const std::string& leavetext() const;

  bool itemsdecay() const;

  bool RunEnterScript( Mobile::Character* chr );
  bool RunLeaveScript( Mobile::Character* chr );
  static bool RunNoCombatCheck( Network::Client* client );

  size_t estimateSize() const override;

private:
  bool guarded_;
  bool nocombat_;
  bool itemsdecay_;
  std::string region_name_;
  std::string entertext_;
  std::string leavetext_;
  std::string enter_script_;
  std::string leave_script_;
};

inline const std::string& JusticeRegion::region_name() const
{
  return region_name_;
}

inline const std::string& JusticeRegion::entertext() const
{
  return entertext_;
}

inline const std::string& JusticeRegion::leavetext() const
{
  return leavetext_;
}
inline bool JusticeRegion::itemsdecay() const
{
  return itemsdecay_;
}
}  // namespace Pol::Core
