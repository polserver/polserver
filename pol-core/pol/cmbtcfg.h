/** @file
 *
 * @par History
 * - 2005/23/11 MuadDib:   Added warmode_wait timer for changing war mode.
 * - 2009/09/03 MuadDib:   Moved combat related settings to Combat Config from SSOPT
 * - 2009/09/22 Turley:    Added DamagePacket support
 */

#ifndef __CMBTCFG_H
#define __CMBTCFG_H
namespace Pol
{
namespace Core
{
enum class WarmodeInhibitsRegenStrategy : unsigned short
{
  None = 0,
  Both = 1,
  PlayerOnly = 2,
  NonplayerOnly = 3,

  MAX = NonplayerOnly,
};

struct CombatConfig
{
  bool display_parry_success_messages;
  WarmodeInhibitsRegenStrategy warmode_inhibits_regen;
  bool attack_self;
  unsigned int warmode_delay;

  bool core_hit_sounds;
  bool scripted_attack_checks;
  bool reset_swing_onturn;
  bool send_swing_packet;
  bool send_damage_packet;
  bool attack_while_frozen;
  bool send_attack_msg;

  static void read_combat_config();
  static WarmodeInhibitsRegenStrategy to_warmode_inhibits_regen_strategy( unsigned short value );
};
}  // namespace Core
}  // namespace Pol
#endif
