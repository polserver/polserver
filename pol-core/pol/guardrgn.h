/*
History
=======
2007/05/04 Shinigami: added region_name() to support UO::GetRegionName*

Notes
=======

*/

#ifndef GUARDRGN_H
#define GUARDRGN_H

#include "region.h"

#include <string>

namespace Pol {
    namespace Network {
        class Client;
    }
	namespace Mobile {
	  class Character;
	}

  namespace Core {

	void read_justice_zones();

	class JusticeRegion : public Region
	{
	  typedef Region base;
	public:
	  JusticeRegion( Clib::ConfigElem& elem, RegionId id );

	  const std::string& region_name() const;
	  const std::string& entertext() const;
	  const std::string& leavetext() const;

	  bool RunEnterScript( Mobile::Character* chr );
	  bool RunLeaveScript( Mobile::Character* chr );
	  static bool RunNoCombatCheck( Network::Client* client );

      virtual size_t estimateSize() const POL_OVERRIDE;

	private:
	  bool guarded_;
	  bool nocombat_;
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
  }
}
#endif
