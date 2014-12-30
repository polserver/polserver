/*
History
=======
2005/02/23 Shinigami: ServSpecOpt DecayItems to enable/disable item decay
2005/03/05 Shinigami: ServSpecOpt UseAAnTileFlags to enable/disable "a"/"an" via Tiles.cfg in formatted Item Names
2005/06/15 Shinigami: ServSpecOpt UseWinLFH to enable/disable Windows XP/2003 low-fragmentation Heap
2005/08/29 Shinigami: ServSpecOpt UseAAnTileFlags renamed to UseTileFlagPrefix
2005/12/05 MuadDib:   ServSpecOpt InvulTage using 0, 1, 2 for method of invul showing.
2009/07/31 Turley:	ServSpecOpt ResetSwingOnTurn=true/false Should SwingTimer be reset with projectile weapon on facing change
					  ServSpecOpt SendSwingPacket=true/false Should packet 0x2F be send on swing.
2009/09/03 MuadDib:   Moved combat related settings to Combat Config.
2009/09/09 Turley:	ServSpecOpt CarryingCapacityMod as * modifier for Character::carrying_capacity()
2009/10/12 Turley:	whisper/yell/say-range ssopt definition
2009/11/19 Turley:	ssopt.core_sends_season & .core_handled_tags - Tomi
2009/12/02 Turley:	added ssopt.support_faces
2010/01/22 Turley:	Speedhack Prevention System

Notes
=======

*/

#include "ssopt.h"
#include "pktdef.h"
#include "uvars.h"

#include "mobile/attribute.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/rawtypes.h"
#include "../clib/logfacility.h"

#include <cstring>

#ifdef _MSC_VER
#pragma warning(disable:4996) // deprecation warning for sprintf, strtok
#endif

namespace Pol {
  namespace Core {
	void ServSpecOpt::read_servspecopt()
	{
	  Clib::ConfigFile cf;
      Clib::ConfigElem elem;

      if ( Clib::FileExists( "config/servspecopt.local.cfg" ) )
	  {
		cf.open( "config/servspecopt.local.cfg" );
		cf.readraw( elem );
	  }
      else if ( Clib::FileExists( "config/servspecopt.cfg" ) )
	  {
		cf.open( "config/servspecopt.cfg" );
		cf.readraw( elem );
	  }

	  gamestate.ssopt.allow_secure_trading_in_warmode = elem.remove_bool( "AllowSecureTradingInWarMode", false );
	  gamestate.ssopt.dblclick_wait = elem.remove_ulong( "DoubleClickWait", 0 );
	  gamestate.ssopt.decay_items = elem.remove_bool( "DecayItems", true );
	  gamestate.ssopt.default_decay_time = elem.remove_ulong( "DefaultDecayTime", 10 );
	  gamestate.ssopt.default_doubleclick_range = elem.remove_ushort( "DefaultDoubleclickRange", 2 );
	  gamestate.ssopt.default_light_level = elem.remove_ushort( "DefaultLightLevel", 10 );
	  gamestate.ssopt.event_visibility_core_checks = elem.remove_bool( "EventVisibilityCoreChecks", false );
	  gamestate.ssopt.max_pathfind_range = elem.remove_ulong( "MaxPathFindRange", 50 );
	  gamestate.ssopt.movement_uses_stamina = elem.remove_bool( "MovementUsesStamina", false );
	  gamestate.ssopt.use_tile_flag_prefix = elem.remove_bool( "UseTileFlagPrefix", true );
	  gamestate.ssopt.default_container_max_items = elem.remove_ushort( "DefaultContainerMaxItems", 125 );
	  gamestate.ssopt.default_container_max_weight = elem.remove_ushort( "DefaultContainerMaxWeight", 250 );
	  gamestate.ssopt.hidden_turns_count = elem.remove_bool( "HiddenTurnsCount", true );
	  gamestate.ssopt.invul_tag = elem.remove_ushort( "InvulTag", 1 );
	  gamestate.ssopt.uo_feature_enable = elem.remove_ushort( "UOFeatureEnable", 0 );
	  gamestate.ssopt.starting_gold = elem.remove_ushort( "StartingGold", 100 );
	  gamestate.ssopt.item_color_mask = elem.remove_ushort( "ItemColorMask", 0xFFF );
	  gamestate.ssopt.use_win_lfh = elem.remove_bool( "UseWinLFH", false );
	  gamestate.ssopt.privacy_paperdoll = elem.remove_bool( "PrivacyPaperdoll", false );
	  gamestate.ssopt.force_new_objcache_packets = elem.remove_bool( "ForceNewObjCachePackets", false );
	  gamestate.ssopt.allow_moving_trade = elem.remove_bool( "AllowMovingTrade", false );
	  gamestate.ssopt.core_handled_locks = elem.remove_bool( "CoreHandledLocks", false );
	  gamestate.ssopt.default_attribute_cap = elem.remove_ushort( "DefaultAttributeCap", 1000 ); // 100.0
	  gamestate.ssopt.default_max_slots = static_cast<unsigned char>( elem.remove_ushort( "MaxContainerSlots", 125 ) ); // this is fine as is, yeah we cast it but 0-255 what we want.
	  gamestate.ssopt.use_slot_index = elem.remove_bool( "UseContainerSlots", false );
	  gamestate.ssopt.use_edit_server = elem.remove_bool( "EditServer", false );
	  gamestate.ssopt.carrying_capacity_mod = elem.remove_double( "CarryingCapacityMod", 1.0 );
	  gamestate.ssopt.core_sends_caps = elem.remove_bool( "CoreSendsCaps", false );
	  gamestate.ssopt.send_stat_locks = elem.remove_bool( "SendStatLocks", false );
	  gamestate.ssopt.speech_range = elem.remove_ushort( "SpeechRange", 12 );
	  gamestate.ssopt.whisper_range = elem.remove_ushort( "WhisperRange", 2 );
	  gamestate.ssopt.yell_range = elem.remove_ushort( "YellRange", 25 );
	  gamestate.ssopt.core_sends_season = elem.remove_bool( "CoreSendsSeason", true );
	  gamestate.ssopt.core_handled_tags = elem.remove_ushort( "CoreHandledTags", 0xFFFF );
	  gamestate.ssopt.support_faces = static_cast<unsigned char>( elem.remove_ushort( "SupportFaces", 0 ) );
	  gamestate.ssopt.newbie_starting_equipment = elem.remove_bool( "NewbieStartingEquipment", true );
	  gamestate.ssopt.speedhack_prevention = elem.remove_bool( "SpeedhackPrevention", false );
	  gamestate.ssopt.scripted_merchant_handlers = elem.remove_bool( "ScriptedMerchantHandlers", false );
	  gamestate.ssopt.honor_unequip_script_on_death = elem.remove_bool( "HonorUnequipScriptOnDeath", false );

	  gamestate.ssopt.speedhack_mountrundelay = elem.remove_ushort( "SpeedHack_MountRunDelay", PKTIN_02_MOUNT_RUN );
	  gamestate.ssopt.speedhack_mountwalkdelay = elem.remove_ushort( "SpeedHack_MountWalkDelay", PKTIN_02_MOUNT_WALK );
	  gamestate.ssopt.speedhack_footrundelay = elem.remove_ushort( "SpeedHack_FootRunDelay", PKTIN_02_FOOT_RUN );
	  gamestate.ssopt.speedhack_footwalkdelay = elem.remove_ushort( "SpeedHack_FootWalkDelay", PKTIN_02_FOOT_WALK );
	  gamestate.ssopt.seperate_speechtoken = elem.remove_bool( "SeperateSpeechTokens", false );
	  gamestate.ssopt.core_sends_guildmsgs = elem.remove_bool( "CoreGuildMessages", true );
	  gamestate.ssopt.mobiles_block_npc_movement = elem.remove_bool( "MobilesBlockNpcMovement", true );
	  ssopt_parse_totalstats( elem );

	  // Turley 2009/11/06 u8 range...
	  //	if ( ssopt.default_max_slots > 255 )
	  //	{
	  //		cerr << "Invalid MaxContainerSlots value '" 
	  //			 << ssopt.default_max_slots << "', using '255'" << endl;
	  //		Log( "Invalid MaxContainerSlots value '%d', using '255'\n", 
	  //			  ssopt.default_max_slots );
	  //		ssopt.default_max_slots = 255;
	  //	}
	}

    void ServSpecOpt::ssopt_parse_totalstats( Clib::ConfigElem& elem )
	{
	  static char tmp[100], tmpcopy[256];

	  std::string total_stats = elem.remove_string( "TotalStatsAtCreation", "65,80" );
	  strncpy( tmpcopy, total_stats.c_str(), sizeof(tmpcopy)-1 ); // strncpy only sets last byte null if there is less bytes at src than the size of tmpcopy (Nando)
	  tmpcopy[sizeof(tmpcopy)-1] = '\0';

	  char *token, *valmax, *valend;
	  unsigned int statmin, statmax;

	  // Stat values must be comma-delimited.
	  // Ranges can be specified using the syntax: <min>-<max>
	  // <max> must be higher than <min>

	  bool valok = true;
	  gamestate.ssopt.total_stats_at_creation.clear();
	  token = strtok( tmpcopy, "," );
	  while ( token != NULL && valok )
	  {
		valok = false;
		statmin = strtoul( token, &valmax, 0 );
		if ( valmax == token )
		  break; // invalid value
		if ( *valmax != '\0' )
		{
		  // second value given?
		  if ( *( valmax++ ) != '-' || !isdigit( *valmax ) )
			break; // invalid second value
		  statmax = strtoul( valmax, &valend, 0 );
		  if ( *valend != '\0' || valend == valmax || statmax < statmin )
			break; // invalid second value
		  if ( statmax == statmin )
			sprintf( tmp, "%lu", static_cast<unsigned long>( statmin ) );
		  else
			sprintf( tmp, "%lu-%lu", static_cast<unsigned long>( statmin ), static_cast<unsigned long>( statmax ) );
		}
		else
		  sprintf( tmp, "%lu", static_cast<unsigned long>( statmin ) );
		gamestate.ssopt.total_stats_at_creation.push_back( tmp );
		valok = true;
		token = strtok( NULL, "," );
	  }

	  if ( !valok || gamestate.ssopt.total_stats_at_creation.empty() )
	  {
		gamestate.ssopt.total_stats_at_creation.clear();
		gamestate.ssopt.total_stats_at_creation.push_back( "65" );
		gamestate.ssopt.total_stats_at_creation.push_back( "80" );
        POLLOG_ERROR << "Invalid TotalStatsAtCreation value '"
		  << total_stats << "', using '65,80'\n";
	  }
	  /*
		  // DEBUG OUTPUT
		  std::vector<std::string>::const_iterator itr = ssopt.total_stats_at_creation.begin();
		  for( ; itr != ssopt.total_stats_at_creation.end() ; itr++ )
		  cout << "* Stats-at-creation entry: " << *itr << endl;
		  */
	}
  }
}