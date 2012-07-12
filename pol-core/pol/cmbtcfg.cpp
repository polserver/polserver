/*
History
=======
2005/23/11 MuadDib:   Added warmode_wait timer for changing war mode.
                      Delay default is 1.
2009/09/03 MuadDib:   Moved combat related settings to Combat Config from SSOPT
2009/09/22 Turley:    Added DamagePacket support
2010/01/14 Turley:    Added AttackWhileFrozen

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"

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
	combat_config.core_hit_sounds = elem.remove_bool("CoreHitSounds", false);
	combat_config.scripted_attack_checks = elem.remove_bool("ScriptedAttackChecks", false);
	combat_config.reset_swing_onturn = elem.remove_bool("ResetSwingOnTurn",false);
	combat_config.send_swing_packet = elem.remove_bool("SendSwingPacket",true);
    combat_config.send_damage_packet = elem.remove_bool("SendDamagePacket",false);
	combat_config.attack_while_frozen = elem.remove_bool("AttackWhileFrozen", true);
	combat_config.send_attack_msg = elem.remove_bool("SendAttackmsg", true);
}
