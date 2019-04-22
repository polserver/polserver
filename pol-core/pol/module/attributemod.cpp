/** @file
 *
 * @par History
 * - 2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 */


#include "attributemod.h"
#include "../../bscript/berror.h"
#include "../../bscript/impstr.h"
#include "../globals/uvars.h"
#include "../mobile/attribute.h"
#include "../mobile/charactr.h"
#include "../network/cliface.h"
#include "../skilladv.h"
#include "../skillid.h"
#include "../uoexhelp.h"
#include "modules/attributes-tbl.h"


namespace Pol
{
namespace Module
{
using namespace Bscript;
using namespace Mobile;

AttributeExecutorModule::AttributeExecutorModule( Executor& exec )
    : TmplExecutorModule<AttributeExecutorModule>( exec )
{
}

Bscript::BObjectImp* AttributeExecutorModule::mf_CheckSkill()
{
  Mobile::Character* chr;
  Core::USKILLID skillid;
  int difficulty;
  unsigned short points;

  if ( Core::getCharacterParam( exec, 0, chr ) && getSkillIdParam( exec, 1, skillid ) &&
       getParam( 2, difficulty ) && getParam( 3, points ) )
  {
    return new Bscript::BLong( chr->check_skill( skillid, difficulty, points ) );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter" );
  }
}

Bscript::BObjectImp* AttributeExecutorModule::mf_GetAttributeName( /* alias_name */ )
{
  const Attribute* attr;

  if ( !Core::getAttributeParam( exec, 0, attr ) )
  {
    return new Bscript::BError( "Invalid parameter type." );
  }

  return new Bscript::String( attr->name );
}

Bscript::BObjectImp* AttributeExecutorModule::mf_GetAttributeDefaultCap( /* alias_name */ )
{
  const Attribute* attr;

  if ( !Core::getAttributeParam( exec, 0, attr ) )
  {
    return new Bscript::BError( "Invalid parameter type." );
  }

  return new Bscript::BLong( attr->default_cap );
}

Bscript::BObjectImp* AttributeExecutorModule::mf_GetAttribute( /* mob, attrname, precision */ )
{
  Character* chr;
  const Attribute* attr;
  short precision;

  if ( getCharacterParam( exec, 0, chr ) && Core::getAttributeParam( exec, 1, attr ) &&
       getParam( 2, precision ) )
  {
    const AttributeValue& av = chr->attribute( attr->attrid );
    return new Bscript::BLong( precision == 1 ? av.effective_tenths() : av.effective() );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* AttributeExecutorModule::mf_GetAttributeBaseValue( /* mob, attrname */ )
{
  Character* chr;
  const Attribute* attr;

  if ( getCharacterParam( exec, 0, chr ) && Core::getAttributeParam( exec, 1, attr ) )
  {
    const AttributeValue& av = chr->attribute( attr->attrid );
    return new Bscript::BLong( av.base() );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* AttributeExecutorModule::mf_GetAttributeTemporaryMod( /* mob, attrname */ )
{
  Character* chr;
  const Attribute* attr;

  if ( getCharacterParam( exec, 0, chr ) && Core::getAttributeParam( exec, 1, attr ) )
  {
    const AttributeValue& av = chr->attribute( attr->attrid );
    return new Bscript::BLong( av.temp_mod() );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* AttributeExecutorModule::mf_GetAttributeIntrinsicMod( /* mob, attrname */ )
{
  Character* chr;
  const Attribute* attr;

  if ( getCharacterParam( exec, 0, chr ) && Core::getAttributeParam( exec, 1, attr ) )
  {
    const AttributeValue& av = chr->attribute( attr->attrid );
    return new Bscript::BLong( av.intrinsic_mod() );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* AttributeExecutorModule::mf_GetAttributeLock( /* mob, attrname */ )
{
  Character* chr;
  const Attribute* attr;

  if ( getCharacterParam( exec, 0, chr ) && Core::getAttributeParam( exec, 1, attr ) )
  {
    const AttributeValue& av = chr->attribute( attr->attrid );
    return new Bscript::BLong( av.lock() );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}
Bscript::BObjectImp* AttributeExecutorModule::mf_GetAttributeCap( /* mob, attrname */ )
{
  Character* chr;
  const Attribute* attr;

  if ( getCharacterParam( exec, 0, chr ) && Core::getAttributeParam( exec, 1, attr ) )
  {
    const AttributeValue& av = chr->attribute( attr->attrid );
    return new Bscript::BLong( av.cap() );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* AttributeExecutorModule::mf_SetAttributeCap( /* mob, attributeid, capvalue */ )
{
  Character* chr;
  const Attribute* attr;
  unsigned short capvalue;

  // FIXME: SetAttributeCap(mob, attributeid) should reset cap to default value :)

  if ( getCharacterParam( exec, 0, chr ) && Core::getAttributeParam( exec, 1, attr ) &&
       getParam( 2, capvalue, ATTRIBUTE_MIN_BASE, ATTRIBUTE_MAX_BASE ) )
  {
    chr->set_dirty();
    AttributeValue& av = chr->attribute( attr->attrid );
    int old_cap = av.cap();
    av.cap( capvalue );

    if ( old_cap != capvalue )
    {
      Network::ClientInterface::tell_attribute_changed( chr, attr );
    }

    return new Bscript::BLong( 1 );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* AttributeExecutorModule::mf_SetAttributeLock(
    /* mob, attributeid, lockstate */ )
{
  Character* chr;
  const Attribute* attr;
  unsigned short lockstate;

  if ( getCharacterParam( exec, 0, chr ) && Core::getAttributeParam( exec, 1, attr ) &&
       getParam( 2, lockstate, 0,
                 2 ) )  // FIXME: hard-coded lock states min and max (0 and 2) -- Nando
  {
    chr->set_dirty();
    AttributeValue& av = chr->attribute( attr->attrid );

    unsigned char old_state = av.lock();
    av.lock( (unsigned char)lockstate );

    if ( old_state != lockstate )
    {
      Network::ClientInterface::tell_attribute_changed( chr, attr );
    }

    return new Bscript::BLong( 1 );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* AttributeExecutorModule::mf_SetAttributeBaseValue(
    /* mob, attributeid, basevalue */ )
{
  Character* chr;
  const Attribute* attr;
  unsigned short basevalue;

  if ( getCharacterParam( exec, 0, chr ) && Core::getAttributeParam( exec, 1, attr ) &&
       getParam( 2, basevalue, ATTRIBUTE_MIN_BASE, ATTRIBUTE_MAX_BASE ) )
  {
    chr->set_dirty();
    AttributeValue& av = chr->attribute( attr->attrid );
    int eff = av.effective_tenths();
    av.base( basevalue );

    if ( eff != av.effective_tenths() )
    {
      Network::ClientInterface::tell_attribute_changed( chr, attr );
      if ( attr->attrid == Core::gamestate.pAttrParry->attrid )
      {
        if ( chr->has_shield() )
          chr->refresh_ar();
      }
    }

    return new Bscript::BLong( 1 );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* AttributeExecutorModule::mf_SetAttributeTemporaryMod(
    /* mob, attributeid, temporary_mod */ )
{
  Character* chr;
  const Attribute* attr;
  int tempmod;

  if ( getCharacterParam( exec, 0, chr ) && Core::getAttributeParam( exec, 1, attr ) &&
       getParam( 2, tempmod, ATTRIBUTE_MIN_TEMP_MOD, ATTRIBUTE_MAX_TEMP_MOD ) )
  {
    AttributeValue& av = chr->attribute( attr->attrid );
    int eff = av.effective_tenths();
    av.temp_mod( static_cast<short>( tempmod ) );

    if ( eff != av.effective_tenths() )
    {
      Network::ClientInterface::tell_attribute_changed( chr, attr );
      if ( attr->attrid == Core::gamestate.pAttrParry->attrid )
      {
        if ( chr->has_shield() )
          chr->refresh_ar();
      }
    }

    return new Bscript::BLong( 1 );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* AttributeExecutorModule::mf_AlterAttributeTemporaryMod(
    /* mob, attributeid, temporary_mod */ )
{
  Character* chr;
  const Attribute* attr;
  int delta;

  if ( getCharacterParam( exec, 0, chr ) && Core::getAttributeParam( exec, 1, attr ) &&
       getParam( 2, delta ) )
  {
    AttributeValue& av = chr->attribute( attr->attrid );
    int eff = av.effective_tenths();
    int newmod = av.temp_mod() + delta;

    if ( newmod < ATTRIBUTE_MIN_TEMP_MOD || newmod > ATTRIBUTE_MAX_TEMP_MOD )
      return new BError( "New modifier value is out of range" );

    av.temp_mod( static_cast<short>( newmod ) );

    if ( eff != av.effective_tenths() )
    {
      Network::ClientInterface::tell_attribute_changed( chr, attr );
      if ( attr->attrid == Core::gamestate.pAttrParry->attrid )
      {
        if ( chr->has_shield() )
          chr->refresh_ar();
      }
    }

    return new Bscript::BLong( 1 );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}


Bscript::BObjectImp* AttributeExecutorModule::mf_RawSkillToBaseSkill()
{
  int rawskill;
  if ( getParam( 0, rawskill ) )
  {
    if ( rawskill < 0 )
      rawskill = 0;
    return new Bscript::BLong( Core::raw_to_base( rawskill ) );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* AttributeExecutorModule::mf_BaseSkillToRawSkill()
{
  unsigned short baseskill;
  if ( getParam( 0, baseskill ) )
    return new Bscript::BLong( Core::base_to_raw( baseskill ) );
  else
    return new Bscript::BError( "Invalid parameter type" );
}
}  // namespace Module
}  // namespace Pol
