/*
History
=======
2005/23/11 MuadDib: Added warmode_wait timer for changing war mode.
                    Delay default is 1.

Notes
=======

*/

#include "clib/stl_inc.h"
#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"

#include "cmbtcfg.h"
#include "polcfg.h"

CombatConfig combat_config;

void read_combat_config()
{
	ConfigFile cf;
	ConfigElem elem;
	if ( FileExists("config/combat.cfg") )
	{
		cf.open("config/combat.cfg");
		cf.readraw( elem );
	
		if ( config.loglevel > 1 )
			cout << "Loading combat.cfg options\n";
	}
	else if ( config.loglevel > 1 )
			cout << "File config/combat.cfg not found, skipping.\n";

	combat_config.display_parry_success_messages = elem.remove_bool( "DisplayParrySuccessMessages", false );
	combat_config.warmode_inhibits_regen = elem.remove_bool( "WarmodeInhibitsRegen", false );
	combat_config.attack_self = elem.remove_bool( "SingleCombat", false );
	combat_config.warmode_delay = elem.remove_ulong( "WarModeDelay", 1 );
}
