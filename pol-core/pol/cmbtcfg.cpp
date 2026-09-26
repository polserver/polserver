/** @file
 *
 * @par History
 * - 2005/23/11 MuadDib:   Added warmode_wait timer for changing war mode.
 *                         Delay default is 1.
 * - 2009/09/03 MuadDib:   Moved combat related settings to Combat Config from SSOPT
 * - 2009/09/22 Turley:    Added DamagePacket support
 * - 2010/01/14 Turley:    Added AttackWhileFrozen
 */


#include "pol/cmbtcfg.h"

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "plib/systemstate.h"

#include "pol/globals/settings.h"


namespace Pol::Core
{
WarmodeInhibitsRegenStrategy CombatConfig::to_warmode_inhibits_regen_strategy(
    unsigned short value )
{
  if ( value > static_cast<unsigned short>( WarmodeInhibitsRegenStrategy::MAX ) )
  {
    return WarmodeInhibitsRegenStrategy::None;
  }
  return static_cast<WarmodeInhibitsRegenStrategy>( value );
}

void CombatConfig::read_combat_config()
{
  Clib::ConfigFile cf;
  Clib::ConfigElem elem;
  if ( Clib::FileExists( "config/combat.cfg" ) )
  {
    cf.open( "config/combat.cfg" );
    cf.readraw( elem );
  }
  else if ( Plib::systemstate.config.loglevel > 1 )
    INFO_PRINTLN( "File config/combat.cfg not found, skipping." );

  // Parse into a copy: a bad value throws, and must leave the live settings untouched.
  CombatConfig cfg;
  cfg.display_parry_success_messages = elem.remove_bool( "DisplayParrySuccessMessages", false );
  cfg.warmode_inhibits_regen =
      to_warmode_inhibits_regen_strategy( elem.remove_ushort( "WarmodeInhibitsRegen", 0 ) );
  cfg.attack_self = elem.remove_bool( "SingleCombat", false );
  cfg.warmode_delay = elem.remove_ulong( "WarModeDelay", 1 );
  cfg.core_hit_sounds = elem.remove_bool( "CoreHitSounds", false );
  cfg.scripted_attack_checks = elem.remove_bool( "ScriptedAttackChecks", false );
  cfg.reset_swing_onturn = elem.remove_bool( "ResetSwingOnTurn", false );
  cfg.send_swing_packet = elem.remove_bool( "SendSwingPacket", true );
  cfg.send_damage_packet = elem.remove_bool( "SendDamagePacket", false );
  cfg.attack_while_frozen = elem.remove_bool( "AttackWhileFrozen", true );
  cfg.send_attack_msg = elem.remove_bool( "SendAttackMsg", true );

  settingsManager.combat_config = cfg;
}
}  // namespace Pol::Core
