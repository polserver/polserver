/** @file
 *
 * @par History
 * - 2005/02/23 Shinigami: ServSpecOpt DecayItems to enable/disable item decay
 * - 2005/03/05 Shinigami: ServSpecOpt UseAAnTileFlags to enable/disable "a"/"an" via Tiles.cfg in
 * formatted Item Names
 * - 2005/06/15 Shinigami: ServSpecOpt UseWinLFH to enable/disable Windows XP/2003 low-fragmentation
 * Heap
 * - 2005/08/29 Shinigami: ServSpecOpt UseAAnTileFlags renamed to UseTileFlagPrefix
 * - 2005/12/05 MuadDib:   ServSpecOpt InvulTage using 0, 1, 2 for method of invul showing.
 * - 2009/07/31 Turley:    ServSpecOpt ResetSwingOnTurn=true/false Should SwingTimer be reset with
 * projectile weapon on facing change
 *                         ServSpecOpt SendSwingPacket=true/false Should packet 0x2F be send on
 * swing.
 * - 2009/09/03 MuadDib:   Moved combat related settings to Combat Config.
 * - 2009/09/09 Turley:    ServSpecOpt CarryingCapacityMod as * modifier for
 * Character::carrying_capacity()
 * - 2009/10/12 Turley:    whisper/yell/say-range ssopt definition
 * - 2009/11/19 Turley:    ssopt.core_sends_season & .core_handled_tags - Tomi
 * - 2009/12/02 Turley:    added ssopt.support_faces
 * - 2010/01/22 Turley:    Speedhack Prevention System
 */


#include "ssopt.h"

#include <cstring>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/stlutil.h"
#include "globals/settings.h"
#include "globals/uvars.h"
#include "network/pktdef.h"


namespace Pol::Core
{
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

  settingsManager.ssopt.allow_secure_trading_in_warmode =
      elem.remove_bool( "AllowSecureTradingInWarMode", false );
  settingsManager.ssopt.dblclick_wait = elem.remove_ulong( "DoubleClickWait", 0 );
  settingsManager.ssopt.decay_items = elem.remove_bool( "DecayItems", true );
  settingsManager.ssopt.default_decay_time = elem.remove_ulong( "DefaultDecayTime", 10 );
  settingsManager.ssopt.refresh_decay_after_boat_moves =
      elem.remove_bool( "RefreshDecayAfterBoatMoves", true );
  settingsManager.ssopt.default_doubleclick_range =
      elem.remove_ushort( "DefaultDoubleclickRange", 2 );
  settingsManager.ssopt.default_accessible_range = elem.remove_ushort(
      "DefaultAccessibleRange", settingsManager.ssopt.default_doubleclick_range );
  settingsManager.ssopt.default_light_level = elem.remove_ushort( "DefaultLightLevel", 10 );
  settingsManager.ssopt.event_visibility_core_checks =
      elem.remove_bool( "EventVisibilityCoreChecks", false );
  settingsManager.ssopt.max_pathfind_range = elem.remove_ushort( "MaxPathFindRange", 50 );
  settingsManager.ssopt.movement_uses_stamina = elem.remove_bool( "MovementUsesStamina", false );
  settingsManager.ssopt.use_tile_flag_prefix = elem.remove_bool( "UseTileFlagPrefix", true );
  settingsManager.ssopt.default_container_max_items =
      elem.remove_ushort( "DefaultContainerMaxItems", 125 );
  settingsManager.ssopt.default_container_max_weight =
      elem.remove_ushort( "DefaultContainerMaxWeight", 250 );
  settingsManager.ssopt.hidden_turns_count = elem.remove_bool( "HiddenTurnsCount", true );
  settingsManager.ssopt.invul_tag = elem.remove_ushort( "InvulTag", 1 );
  auto a9flag = static_cast<Plib::A9Feature>( elem.remove_ushort( "UOFeatureEnable", 0 ) );
  auto default_expansion = elem.remove_string(
      "DefaultExpansion", Plib::getExpansionName( Plib::ExpansionVersion::T2A ).c_str() );
  settingsManager.ssopt.features.updateFromSSOpt( a9flag, default_expansion,
                                                  elem.remove_ushort( "SupportFaces", 0 ) );
  settingsManager.ssopt.starting_gold = elem.remove_ushort( "StartingGold", 100 );
  settingsManager.ssopt.item_color_mask = elem.remove_ushort( "ItemColorMask", 0xFFF );
  settingsManager.ssopt.use_win_lfh = elem.remove_bool( "UseWinLFH", false );
  settingsManager.ssopt.privacy_paperdoll = elem.remove_bool( "PrivacyPaperdoll", false );
  settingsManager.ssopt.force_new_objcache_packets =
      elem.remove_bool( "ForceNewObjCachePackets", false );
  settingsManager.ssopt.allow_moving_trade = elem.remove_bool( "AllowMovingTrade", false );
  settingsManager.ssopt.core_handled_locks = elem.remove_bool( "CoreHandledLocks", false );
  settingsManager.ssopt.default_attribute_cap =
      elem.remove_ushort( "DefaultAttributeCap", 1000 );  // 100.0
  settingsManager.ssopt.default_max_slots = static_cast<unsigned char>( elem.remove_ushort(
      "MaxContainerSlots", 125 ) );  // this is fine as is, yeah we cast it but 0-255 what we want.
  settingsManager.ssopt.use_slot_index = elem.remove_bool( "UseContainerSlots", false );
  settingsManager.ssopt.use_edit_server = elem.remove_bool( "EditServer", false );
  settingsManager.ssopt.carrying_capacity_mod = elem.remove_double( "CarryingCapacityMod", 1.0 );
  settingsManager.ssopt.core_sends_caps = elem.remove_bool( "CoreSendsCaps", false );
  settingsManager.ssopt.core_ignores_defence_caps =
      elem.remove_bool( "CoreIgnoresDefenceCaps", false );
  settingsManager.ssopt.send_stat_locks = elem.remove_bool( "SendStatLocks", false );
  settingsManager.ssopt.allow_visual_range_modification =
      elem.remove_bool( "AllowVisualRangeModification", false );
  settingsManager.ssopt.default_visual_range =
      static_cast<u8>( elem.remove_int( "DefaultVisualRange", 18 ) );
  settingsManager.ssopt.min_visual_range =
      static_cast<u8>( elem.remove_int( "MinimumVisualRange", 5 ) );  // 5 is official minimum
  settingsManager.ssopt.max_visual_range =
      static_cast<u8>( elem.remove_int( "MaximumVisualRange", 24 ) );  // 24 is offical maximum
  settingsManager.ssopt.speech_range = elem.remove_ushort( "SpeechRange", 12 );
  settingsManager.ssopt.whisper_range = elem.remove_ushort( "WhisperRange", 2 );
  settingsManager.ssopt.yell_range = elem.remove_ushort( "YellRange", 25 );
  settingsManager.ssopt.core_sends_season = elem.remove_bool( "CoreSendsSeason", true );
  settingsManager.ssopt.core_handled_tags = elem.remove_ushort( "CoreHandledTags", 0xFFFF );
  settingsManager.ssopt.newbie_starting_equipment =
      elem.remove_bool( "NewbieStartingEquipment", true );
  settingsManager.ssopt.speedhack_prevention = elem.remove_bool( "SpeedhackPrevention", false );
  settingsManager.ssopt.scripted_merchant_handlers =
      elem.remove_bool( "ScriptedMerchantHandlers", false );
  settingsManager.ssopt.honor_unequip_script_on_death =
      elem.remove_bool( "HonorUnequipScriptOnDeath", false );

  settingsManager.ssopt.speedhack_mountrundelay =
      elem.remove_ushort( "SpeedHack_MountRunDelay", PKTIN_02_MOUNT_RUN );
  settingsManager.ssopt.speedhack_mountwalkdelay =
      elem.remove_ushort( "SpeedHack_MountWalkDelay", PKTIN_02_MOUNT_WALK );
  settingsManager.ssopt.speedhack_footrundelay =
      elem.remove_ushort( "SpeedHack_FootRunDelay", PKTIN_02_FOOT_RUN );
  settingsManager.ssopt.speedhack_footwalkdelay =
      elem.remove_ushort( "SpeedHack_FootWalkDelay", PKTIN_02_FOOT_WALK );
  settingsManager.ssopt.seperate_speechtoken = elem.remove_bool( "SeperateSpeechTokens", false );
  settingsManager.ssopt.core_sends_guildmsgs = elem.remove_bool( "CoreGuildMessages", true );
  settingsManager.ssopt.mobiles_block_npc_movement =
      static_cast<MOVEBLOCKMODE>( elem.remove_ushort( "MobilesBlockNpcMovement", 1 ) );

  unsigned char default_character_height =
      static_cast<unsigned char>( elem.remove_ushort( "DefaultCharacterHeight", 15 ) );
  if ( default_character_height < 1 )
    default_character_height = 1;
  else if ( default_character_height > 32 )
    default_character_height = 32;
  settingsManager.ssopt.default_character_height = default_character_height;

  settingsManager.ssopt.enable_worldmap_packets =
      elem.remove_bool( "EnableWorldMapPackets", false );

  settingsManager.ssopt.undo_get_item_enable_range_check =
      elem.remove_ushort( "UndoGetItemEnableRangeCheck", false );
  settingsManager.ssopt.undo_get_item_drop_here =
      elem.remove_ushort( "UndoGetItemDropHere", false );
  settingsManager.ssopt.boat_sails_collide = elem.remove_bool( "BoatSailsCollide", false );
  settingsManager.ssopt.npc_minimum_movement_delay =
      elem.remove_ushort( "NpcMinimumMovementDelay", 250 );

  settingsManager.ssopt.master_can_clothe_npcs = elem.remove_bool( "MasterCanClotheNPCs", false );
  settingsManager.ssopt.master_can_access_npcs_backpack =
      elem.remove_bool( "MasterCanAccessNPCsBackpack", false );

  ssopt_parse_totalstats( elem );

  // default value needs to be reset as the object is created before config is loaded
  gamestate.initialize_range_from_config( settingsManager.ssopt.default_visual_range );

  // Turley 2009/11/06 u8 range...
  // if ( ssopt.default_max_slots > 255 )
  // {
  //   cerr << "Invalid MaxContainerSlots value '"
  //        << ssopt.default_max_slots << "', using '255'" << endl;
  //   Log( "Invalid MaxContainerSlots value '%d', using '255'\n",
  //        ssopt.default_max_slots );
  //   ssopt.default_max_slots = 255;
  // }
}

void ServSpecOpt::ssopt_parse_totalstats( Clib::ConfigElem& elem )
{
  static char tmp[100], tmpcopy[256];

  std::string total_stats = elem.remove_string( "TotalStatsAtCreation", "65,80" );
  strncpy( tmpcopy, total_stats.c_str(), sizeof( tmpcopy ) - 1 );  // strncpy only sets last byte
                                                                   // null if there is less bytes at
                                                                   // src than the size of tmpcopy
                                                                   // (Nando)
  tmpcopy[sizeof( tmpcopy ) - 1] = '\0';

  char *token, *valmax, *valend;
  unsigned int statmin, statmax;

  // Stat values must be comma-delimited.
  // Ranges can be specified using the syntax: <min>-<max>
  // <max> must be higher than <min>

  bool valok = true;
  settingsManager.ssopt.total_stats_at_creation.clear();
  token = strtok( tmpcopy, "," );
  while ( token != nullptr && valok )
  {
    valok = false;
    statmin = strtoul( token, &valmax, 0 );
    if ( valmax == token )
      break;  // invalid value
    if ( *valmax != '\0' )
    {
      // second value given?
      if ( *( valmax++ ) != '-' || !isdigit( *valmax ) )
        break;  // invalid second value
      statmax = strtoul( valmax, &valend, 0 );
      if ( *valend != '\0' || valend == valmax || statmax < statmin )
        break;  // invalid second value
      if ( statmax == statmin )
        snprintf( tmp, Clib::arsize( tmp ), "%lu", static_cast<unsigned long>( statmin ) );
      else
        snprintf( tmp, Clib::arsize( tmp ), "%lu-%lu", static_cast<unsigned long>( statmin ),
                  static_cast<unsigned long>( statmax ) );
    }
    else
      snprintf( tmp, Clib::arsize( tmp ), "%lu", static_cast<unsigned long>( statmin ) );
    settingsManager.ssopt.total_stats_at_creation.emplace_back( tmp );
    valok = true;
    token = strtok( nullptr, "," );
  }

  if ( !valok || settingsManager.ssopt.total_stats_at_creation.empty() )
  {
    settingsManager.ssopt.total_stats_at_creation.clear();
    settingsManager.ssopt.total_stats_at_creation.emplace_back( "65" );
    settingsManager.ssopt.total_stats_at_creation.emplace_back( "80" );
    POLLOG_ERRORLN( "Invalid TotalStatsAtCreation value '{}', using '65,80'", total_stats );
  }
  /*
      // DEBUG OUTPUT
      std::vector<std::string>::const_iterator itr = ssopt.total_stats_at_creation.begin();
      for( ; itr != ssopt.total_stats_at_creation.end() ; itr++ )
      cout << "* Stats-at-creation entry: " << *itr << endl;
      */
}
}  // namespace Pol::Core
