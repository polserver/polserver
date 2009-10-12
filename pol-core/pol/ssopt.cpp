/*
History
=======
2005/02/23 Shinigami: ServSpecOpt DecayItems to enable/disable item decay
2005/03/05 Shinigami: ServSpecOpt UseAAnTileFlags to enable/disable "a"/"an" via Tiles.cfg in formatted Item Names
2005/06/15 Shinigami: ServSpecOpt UseWinLFH to enable/disable Windows XP/2003 low-fragmentation Heap
2005/08/29 Shinigami: ServSpecOpt UseAAnTileFlags renamed to UseTileFlagPrefix
2005/12/05 MuadDib:   ServSpecOpt InvulTage using 0, 1, 2 for method of invul showing.
2009/07/31 Turley:    ServSpecOpt ResetSwingOnTurn=true/false Should SwingTimer be reset with projectile weapon on facing change
                      ServSpecOpt SendSwingPacket=true/false Should packet 0x2F be send on swing.
2009/09/03 MuadDib:   Moved combat related settings to Combat Config.
2009/09/09 Turley:    ServSpecOpt CarryingCapacityMod as * modifier for Character::carrying_capacity()
2009/10/12 Turley:    whisper/yell/say-range ssopt definition

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfile.h"
#include "ssopt.h"

#include "mobile/attribute.h"

ServSpecOpt ssopt;

void read_servspecopt()
{
    ConfigFile cf;
    ConfigElem elem;

    if (FileExists( "config/servspecopt.local.cfg" ))
    {
        cf.open( "config/servspecopt.local.cfg" );
        cf.readraw( elem );
    }
    else if (FileExists( "config/servspecopt.cfg" ))
    {
        cf.open( "config/servspecopt.cfg" );
        cf.readraw( elem );
    }

    ssopt.allow_secure_trading_in_warmode = elem.remove_bool( "AllowSecureTradingInWarMode", false );
	ssopt.dblclick_wait = elem.remove_ulong("DoubleClickWait", 0);
	ssopt.decay_items = elem.remove_bool( "DecayItems", true );
    ssopt.default_decay_time = elem.remove_ulong( "DefaultDecayTime", 10 );
    ssopt.default_doubleclick_range = elem.remove_ushort( "DefaultDoubleclickRange", 2 );
    ssopt.default_light_level = elem.remove_ushort( "DefaultLightLevel", 10 );
	ssopt.event_visibility_core_checks = elem.remove_bool("EventVisibilityCoreChecks", false);
	ssopt.max_pathfind_range = elem.remove_ulong( "MaxPathFindRange", 50 );
    ssopt.movement_uses_stamina = elem.remove_bool( "MovementUsesStamina", false );
	ssopt.use_tile_flag_prefix = elem.remove_bool( "UseTileFlagPrefix", true );
	ssopt.default_container_max_items = elem.remove_ushort( "DefaultContainerMaxItems", 125 );
	ssopt.default_container_max_weight = elem.remove_ushort( "DefaultContainerMaxWeight", 250 );
	ssopt.hidden_turns_count = elem.remove_bool("HiddenTurnsCount", true);
	ssopt.invul_tag = elem.remove_ushort("InvulTag", 1);
	ssopt.uo_feature_enable = elem.remove_ushort( "UOFeatureEnable", 0);
	ssopt.starting_gold = elem.remove_ushort("StartingGold", 100);
	ssopt.item_color_mask = elem.remove_ushort("ItemColorMask", 0xFFF);
	ssopt.use_win_lfh = elem.remove_bool( "UseWinLFH", false );
	ssopt.privacy_paperdoll = elem.remove_bool("PrivacyPaperdoll",false);
	ssopt.force_new_objcache_packets = elem.remove_bool("ForceNewObjCachePackets",false);
	ssopt.allow_moving_trade = elem.remove_bool("AllowMovingTrade",false);
	ssopt.core_handled_locks = elem.remove_bool("CoreHandledLocks",false);
	ssopt.default_attribute_cap = elem.remove_ushort("DefaultAttributeCap", 1000); // 100.0
	ssopt.default_max_slots = static_cast<unsigned char>(elem.remove_ushort("MaxContainerSlots", 125));
	ssopt.use_slot_index = elem.remove_bool("UseContainerSlots",false);
	ssopt.use_edit_server = elem.remove_bool("EditServer",false);
    ssopt.carrying_capacity_mod = elem.remove_double("CarryingCapacityMod",1.0);
	ssopt.core_sends_caps = elem.remove_bool("CoreSendsCaps",false);
	ssopt.send_stat_locks = elem.remove_bool("SendStatLocks",false);
    ssopt.speech_range = elem.remove_ushort("SpeechRange",12);
    ssopt.whisper_range = elem.remove_ushort("WhisperRange",2);
    ssopt.yell_range = elem.remove_ushort("YellRange",25);
	ssopt_parse_totalstats(elem);

	if ( ssopt.default_max_slots > 255 )
	{
		cerr << "Invalid MaxContainerSlots value '" 
             << ssopt.default_max_slots << "', using '255'" << endl;
        Log( "Invalid MaxContainerSlots value '%d', using '255'\n", 
              ssopt.default_max_slots );
		ssopt.default_max_slots = 255;
	}
}

void ssopt_parse_totalstats(ConfigElem& elem)
{
	static char tmp[100], tmpcopy[256];

	std::string total_stats = elem.remove_string( "TotalStatsAtCreation", "65,80" );
	strncpy(tmpcopy, total_stats.c_str(), 256);

	char *token, *valmax, *valend;
	unsigned long statmin, statmax;

	// Stat values must be comma-delimited.
	// Ranges can be specified using the syntax: <min>-<max>
	// <max> must be higher than <min>

	bool valok = true;
	ssopt.total_stats_at_creation.clear();
	token = strtok(tmpcopy, ",");
	while( token != NULL && valok )
	{
		valok = false;
		statmin = strtoul(token, &valmax, 0);
		if ( valmax == token )
			break; // invalid value
		if ( *valmax != '\0' )
		{
			// second value given?
			if ( *(valmax++) != '-' || !isdigit(*valmax) )
				break; // invalid second value
			statmax = strtoul(valmax, &valend, 0);
			if ( *valend != '\0' || valend == valmax || statmax < statmin )
				break; // invalid second value
			if ( statmax == statmin )
				sprintf(tmp, "%lu", statmin);
			else
				sprintf(tmp, "%lu-%lu", statmin, statmax);
		}
		else
			sprintf(tmp, "%lu", statmin);
		ssopt.total_stats_at_creation.push_back(tmp);
		valok = true;
		token = strtok(NULL, ",");
	}

	if ( !valok || ssopt.total_stats_at_creation.empty() )
	{
		ssopt.total_stats_at_creation.clear();
		ssopt.total_stats_at_creation.push_back("65");
		ssopt.total_stats_at_creation.push_back("80");
        cerr << "Invalid TotalStatsAtCreation value '" 
             << total_stats << "', using '65,80'" << endl;
        Log( "Invalid TotalStatsAtCreation value '%s', using '65,80'\n", 
              total_stats.c_str() );
	}
/*
	// DEBUG OUTPUT
	std::vector<std::string>::const_iterator itr = ssopt.total_stats_at_creation.begin();
	for( ; itr != ssopt.total_stats_at_creation.end() ; itr++ )
		cout << "* Stats-at-creation entry: " << *itr << endl;
*/
}
