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


#ifndef SSOPT_H
#define SSOPT_H

#include <atomic>
#include <string>
#include <vector>

#include "../clib/rawtypes.h"
#include "../plib/uoexpansion.h"

namespace Pol
{
namespace Clib
{
class ConfigElem;
}

namespace Core
{

enum MOVEBLOCKMODE : u8
{
  MOVEBLOCKMODE_SAME_MASTER = 0,
  MOVEBLOCKMODE_ALL = 1,
  MOVEBLOCKMODE_NONE = 2
};

struct ServSpecOpt
{
  bool allow_secure_trading_in_warmode;
  unsigned int dblclick_wait;
  std::atomic<bool> decay_items;
  std::atomic<unsigned int> default_decay_time;
  std::atomic<bool> refresh_decay_after_boat_moves;
  unsigned short default_doubleclick_range;
  unsigned short default_accessible_range;
  unsigned short default_light_level;
  bool event_visibility_core_checks;
  unsigned short max_pathfind_range;
  bool movement_uses_stamina;
  bool use_tile_flag_prefix;
  unsigned short default_container_max_items;
  unsigned short default_container_max_weight;
  bool hidden_turns_count;
  unsigned short invul_tag;
  Plib::ServerFeatures features;
  unsigned short starting_gold;
  unsigned short item_color_mask;
  unsigned char default_max_slots;
  bool use_win_lfh;
  bool privacy_paperdoll;
  bool force_new_objcache_packets;
  bool allow_moving_trade;
  bool core_handled_locks;
  bool use_slot_index;
  double carrying_capacity_mod;
  bool use_edit_server;

  unsigned short default_attribute_cap;
  bool core_sends_caps;
  bool core_ignores_defence_caps;
  bool send_stat_locks;

  /* how to validate these values (and how range checking works on the client):
   tell the client about a character.  Take your character to the spot that
   character is standing on.  Walk straight, 18 steps in any direction.  Run back
   to the spot.  The character will still be there.  If you walk 19 steps, the
   client automatically removes the object - the server doesn't need to tell it to.
   Simple rectangular distance is used - if the x's differ by <= 18, and the y's differ
   by <= 18, you're in range.  If either is out of range, you're out.
   */
  u8 default_visual_range;
  u8 min_visual_range;
  u8 max_visual_range;
  bool allow_visual_range_modification;

  unsigned short speech_range;
  unsigned short whisper_range;
  unsigned short yell_range;

  bool core_sends_season;
  unsigned short core_handled_tags;
  bool newbie_starting_equipment;
  bool speedhack_prevention;

  bool scripted_merchant_handlers;
  bool honor_unequip_script_on_death;

  unsigned short speedhack_mountrundelay;
  unsigned short speedhack_mountwalkdelay;
  unsigned short speedhack_footrundelay;
  unsigned short speedhack_footwalkdelay;

  std::vector<std::string> total_stats_at_creation;
  bool seperate_speechtoken;
  bool core_sends_guildmsgs;
  MOVEBLOCKMODE mobiles_block_npc_movement;

  unsigned char default_character_height;
  bool enable_worldmap_packets;

  bool undo_get_item_enable_range_check;
  bool undo_get_item_drop_here;

  bool boat_sails_collide;

  unsigned short npc_minimum_movement_delay;

  static void read_servspecopt();
  static void ssopt_parse_totalstats( Clib::ConfigElem& elem );
};
}  // namespace Core
}  // namespace Pol
#endif
