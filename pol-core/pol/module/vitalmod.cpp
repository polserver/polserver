/** @file
 *
 * @par History
 * - 2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 * - 2009/09/22 Turley:    repsys param to applydamage
 * - 2009/11/20 Turley:    RecalcVitals can update single Attributes/Vitals - based on Tomi
 * - 2010/01/15 Turley:    (Tomi) send damage param ApplyDamage/ApplyRawDamage
 */


#include "vitalmod.h"
#include "modules/vitals-tbl.h"

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/impstr.h"
#include "../cmbtcfg.h"
#include "../globals/settings.h"
#include "../mobile/attribute.h"
#include "../mobile/charactr.h"
#include "../spells.h"
#include "../ufunc.h"
#include "../uoexhelp.h"
#include "../vital.h"

namespace Pol
{
namespace Bscript
{
using namespace Module;
template <>
TmplExecutorModule<VitalExecutorModule>::FunctionTable
    TmplExecutorModule<VitalExecutorModule>::function_table = {
        {"ApplyRawDamage", &VitalExecutorModule::mf_ApplyRawDamage},
        {"ApplyDamage", &VitalExecutorModule::mf_ApplyDamage},
        {"HealDamage", &VitalExecutorModule::mf_HealDamage},
        {"ConsumeMana", &VitalExecutorModule::mf_ConsumeMana},
        {"ConsumeVital", &VitalExecutorModule::mf_ConsumeVital},
        {"RecalcVitals", &VitalExecutorModule::mf_RecalcVitals},
        {"GetVitalName", &VitalExecutorModule::mf_GetVitalName},
        {"GetVital", &VitalExecutorModule::mf_GetVital},
        {"SetVital", &VitalExecutorModule::mf_SetVital},
        {"GetVitalRegenRate", &VitalExecutorModule::mf_GetVitalRegenRate},
        {"GetVitalMaximumValue", &VitalExecutorModule::mf_GetVitalMaximumValue}};
}  // namespace Bscript
namespace Module
{
using namespace Bscript;

VitalExecutorModule::VitalExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<VitalExecutorModule>( "vitals", exec )
{
}

BObjectImp* VitalExecutorModule::mf_ApplyRawDamage()
{
  Mobile::Character* chr;
  int damage;
  int userepsys;
  int send_damage_packet;
  if ( getCharacterParam( exec, 0, chr ) && getParam( 1, damage ) && getParam( 2, userepsys ) &&
       getParam( 3, send_damage_packet ) && damage >= 0 && damage <= USHRT_MAX )
  {
    bool send_dmg = send_damage_packet == 2 ? Core::settingsManager.combat_config.send_damage_packet
                                            : ( send_damage_packet > 0 ? true : false );
    chr->apply_raw_damage_hundredths( static_cast<unsigned int>( damage * 100 ), GetUOController(),
                                      userepsys > 0 ? true : false, send_dmg );
    return new BLong( 1 );
  }
  else
    return new BLong( 0 );
}

BObjectImp* VitalExecutorModule::mf_ApplyDamage()
{
  Mobile::Character* chr;
  double damage;
  int userepsys;
  int send_damage_packet;
  if ( getCharacterParam( exec, 0, chr ) && getRealParam( 1, damage ) && getParam( 2, userepsys ) &&
       getParam( 3, send_damage_packet ) )
  {
    if ( damage >= 0.0 && damage <= 30000.0 )
    {
      bool send_dmg = send_damage_packet == 2
                          ? Core::settingsManager.combat_config.send_damage_packet
                          : ( send_damage_packet > 0 ? true : false );
      damage = chr->apply_damage( static_cast<unsigned short>( damage ), GetUOController(),
                                  userepsys > 0 ? true : false, send_dmg );
      return new BLong( static_cast<int>( damage ) );
    }
    else
      return new BError( "Damage is out of range" );
  }
  else
    return new BError( "Invalid parameter type" );
}

BObjectImp* VitalExecutorModule::mf_HealDamage()
{
  Mobile::Character* chr;
  int amount;
  if ( getCharacterParam( exec, 0, chr ) && getParam( 1, amount ) && amount >= 0 &&
       amount <= USHRT_MAX )
  {
    Mobile::Character* controller = GetUOController();
    if ( controller )
      controller->repsys_on_help( chr );

    chr->heal_damage_hundredths( static_cast<unsigned short>( amount ) * 100L );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* VitalExecutorModule::mf_ConsumeMana()
{
  Mobile::Character* chr;
  int spellid;
  if ( getCharacterParam( exec, 0, chr ) && getParam( 1, spellid ) )
  {
    if ( !Core::VALID_SPELL_ID( spellid ) )
      return new BError( "Spell ID out of range" );

    Core::USpell* spell = Core::gamestate.spells[spellid];
    if ( spell == nullptr )
      return new BError( "No such spell" );
    else if ( spell->check_mana( chr ) == false )
      return new BLong( 0 );

    spell->consume_mana( chr );
    if ( chr->has_active_client() )
      Core::send_mana_level( chr->client );

    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* VitalExecutorModule::mf_GetVitalName( /* alias_name */ )
{
  const Core::Vital* vital;

  if ( !getVitalParam( exec, 0, vital ) )
  {
    return new BError( "Invalid parameter type." );
  }

  return new String( vital->name );
}

BObjectImp* VitalExecutorModule::mf_GetVital( /* mob, vitalid */ )
{
  Mobile::Character* chr;
  const Core::Vital* vital;

  if ( getCharacterParam( exec, 0, chr ) && getVitalParam( exec, 1, vital ) )
  {
    const Mobile::VitalValue& vv = chr->vital( vital->vitalid );
    return new BLong( vv.current() );
  }
  else
    return new BError( "Invalid parameter type" );
}

BObjectImp* VitalExecutorModule::mf_GetVitalMaximumValue( /* mob, vitalid */ )
{
  Mobile::Character* chr;
  const Core::Vital* vital;

  if ( getCharacterParam( exec, 0, chr ) && getVitalParam( exec, 1, vital ) )
  {
    const Mobile::VitalValue& vv = chr->vital( vital->vitalid );
    return new BLong( vv.maximum() );
  }
  else
    return new BError( "Invalid parameter type" );
}

BObjectImp* VitalExecutorModule::mf_GetVitalRegenRate( /* mob, vitalid */ )
{
  Mobile::Character* chr;
  const Core::Vital* vital;

  if ( getCharacterParam( exec, 0, chr ) && getVitalParam( exec, 1, vital ) )
  {
    const Mobile::VitalValue& vv = chr->vital( vital->vitalid );
    return new BLong( vv.regenrate() );
  }
  else
    return new BError( "Invalid parameter type" );
}

BObjectImp* VitalExecutorModule::mf_SetVital( /* mob, vitalid, hundredths */ )
{
  Mobile::Character* chr;
  const Core::Vital* vital;
  int value;

  if ( getCharacterParam( exec, 0, chr ) && getVitalParam( exec, 1, vital ) &&
       getParam( 2, value, Core::VITAL_MAX_HUNDREDTHS ) )
  {
    Mobile::VitalValue& vv = chr->vital( vital->vitalid );
    chr->set_current( vital, vv, value, Mobile::Character::VitalDepletedReason::SCRIPT );
    return new BLong( 1 );
  }
  else
    return new BError( "Invalid parameter type" );
}

BObjectImp* VitalExecutorModule::mf_ConsumeVital( /* mob, vital, hundredths */ )
{
  Mobile::Character* chr;
  const Core::Vital* vital;
  int hundredths;

  if ( getCharacterParam( exec, 0, chr ) && getVitalParam( exec, 1, vital ) &&
       getParam( 2, hundredths, Core::VITAL_MAX_HUNDREDTHS ) )
  {
    Mobile::VitalValue& vv = chr->vital( vital->vitalid );
    bool res =
        chr->consume( vital, vv, hundredths, Mobile::Character::VitalDepletedReason::SCRIPT );
    return new BLong( res ? 1 : 0 );
  }
  else
    return new BError( "Invalid parameter type" );
}

BObjectImp* VitalExecutorModule::mf_RecalcVitals( /* mob, attributes, vitals */ )
{
  Mobile::Character* chr;
  BObjectImp* param1 = getParamImp( 1 );
  BObjectImp* param2 = getParamImp( 2 );

  if ( getCharacterParam( exec, 0, chr ) && param1 != nullptr && param2 != nullptr )
  {
    if ( chr->logged_in() )
    {
      bool calc_attr = false;
      bool calc_vital = false;

      if ( param1->isa( BObjectImp::OTLong ) )
        calc_attr = static_cast<BLong*>( param1 )->isTrue();
      else if ( param1->isa( BObjectImp::OTString ) )
      {
        String* attrname = static_cast<String*>( param1 );
        Mobile::Attribute* attr = Mobile::Attribute::FindAttribute( attrname->value() );
        if ( attr == nullptr )
          return new BError( "Attribute not defined: " + attrname->value() );
        chr->calc_single_attribute( attr );
      }
      else
        return new BError( "Invalid parameter type" );

      if ( param2->isa( BObjectImp::OTLong ) )
        calc_vital = static_cast<BLong*>( param2 )->isTrue();
      else if ( param2->isa( BObjectImp::OTString ) )
      {
        String* vitalname = static_cast<String*>( param2 );
        Core::Vital* vital = Core::FindVital( vitalname->value() );
        if ( vital == nullptr )
          return new BError( "Vital not defined: " + vitalname->value() );
        chr->calc_single_vital( vital );
      }
      else
        return new BError( "Invalid parameter type" );

      chr->calc_vital_stuff( calc_attr, calc_vital );
      return new BLong( 1 );
    }
    else
      return new BError( "Mobile must be online." );
  }
  else
    return new BError( "Invalid parameter type" );
}
}  // namespace Module
}  // namespace Pol
