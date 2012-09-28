/*
History
=======
2007/05/04 Shinigami: added region_name() to support UO::GetRegionName*

Notes
=======

*/

#ifndef GUARDRGN_H
#define GUARDRGN_H

#include <string>

#include "region.h"

void read_justice_zones();

class JusticeRegion : public Region
{
    typedef Region base;
public:
    JusticeRegion( ConfigElem& elem );

    const std::string& region_name() const;
    const std::string& entertext() const;
    const std::string& leavetext() const;

	bool RunEnterScript(Character* chr);
	bool RunLeaveScript(Character* chr);
	static bool RunNoCombatCheck(Client* client);

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

typedef RegionGroup<JusticeRegion> JusticeDef;

extern JusticeDef* justicedef;

#endif
