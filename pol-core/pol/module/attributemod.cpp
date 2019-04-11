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


namespace Pol
{
namespace Bscript
{
using namespace Module;
template <>
TmplExecutorModule<AttributeExecutorModule>::FunctionTable
    TmplExecutorModule<AttributeExecutorModule>::function_table = {
        {"CheckSkill", &AttributeExecutorModule::mf_CheckSkill, UINT_MAX},
        {"GetAttributeName", &AttributeExecutorModule::mf_GetAttributeName, UINT_MAX},
        {"GetAttributeDefaultCap", &AttributeExecutorModule::mf_GetAttributeDefaultCap, UINT_MAX},
        {"GetAttribute", &AttributeExecutorModule::mf_GetAttribute, UINT_MAX},
        {"GetAttributeBaseValue", &AttributeExecutorModule::mf_GetAttributeBaseValue, UINT_MAX},
        {"GetAttributeTemporaryMod", &AttributeExecutorModule::mf_GetAttributeTemporaryMod, UINT_MAX},
        {"GetAttributeIntrinsicMod", &AttributeExecutorModule::mf_GetAttributeIntrinsicMod, UINT_MAX},
        {"GetAttributeLock", &AttributeExecutorModule::mf_GetAttributeLock, UINT_MAX},
        {"GetAttributeCap", &AttributeExecutorModule::mf_GetAttributeCap, UINT_MAX},
        {"SetAttributeCap", &AttributeExecutorModule::mf_SetAttributeCap, UINT_MAX},
        {"SetAttributeLock", &AttributeExecutorModule::mf_SetAttributeLock, UINT_MAX},
        {"SetAttributeBaseValue", &AttributeExecutorModule::mf_SetAttributeBaseValue, UINT_MAX},
        {"SetAttributeTemporaryMod", &AttributeExecutorModule::mf_SetAttributeTemporaryMod, UINT_MAX},
        {"AlterAttributeTemporaryMod", &AttributeExecutorModule::mf_AlterAttributeTemporaryMod, UINT_MAX},
        {"RawSkillToBaseSkill", &AttributeExecutorModule::mf_RawSkillToBase, UINT_MAX},
        {"BaseSkillToRawSkill", &AttributeExecutorModule::mf_BaseSkillToRaw, UINT_MAX}};
template <>
const char* TmplExecutorModule<AttributeExecutorModule>::modname = "attributes";
}
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


Bscript::BObjectImp* AttributeExecutorModule::mf_RawSkillToBase()
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

Bscript::BObjectImp* AttributeExecutorModule::mf_BaseSkillToRaw()
{
  unsigned short baseskill;
  if ( getParam( 0, baseskill ) )
    return new Bscript::BLong( Core::base_to_raw( baseskill ) );
  else
    return new Bscript::BError( "Invalid parameter type" );
}
}  // namespace Module
}


// FIXME I... Have no idea...
#include "uomod.h"
namespace Pol
{
namespace Bscript
{
using namespace Module;
template <>
TmplExecutorModule<UOExecutorModule>::FunctionTable
    TmplExecutorModule<UOExecutorModule>::function_table = {
        {"SendStatus", &UOExecutorModule::mf_SendStatus, UINT_MAX},

        {"SendCharacterRaceChanger", &UOExecutorModule::mf_SendCharacterRaceChanger, UINT_MAX},
        {"SendHousingTool", &UOExecutorModule::mf_SendHousingTool, UINT_MAX},
        {"MoveObjectToLocation", &UOExecutorModule::mf_MoveObjectToLocation, UINT_MAX},
        {"SendOpenBook", &UOExecutorModule::mf_SendOpenBook, UINT_MAX},
        {"SelectColor", &UOExecutorModule::mf_SelectColor, UINT_MAX},
        {"AddAmount", &UOExecutorModule::mf_AddAmount, UINT_MAX},
        {"SendViewContainer", &UOExecutorModule::mf_SendViewContainer, UINT_MAX},
        {"SendInstaResDialog", &UOExecutorModule::mf_SendInstaResDialog, UINT_MAX},
        {"SendStringAsTipWindow", &UOExecutorModule::mf_SendStringAsTipWindow, UINT_MAX},
        {"GetCommandHelp", &UOExecutorModule::mf_GetCommandHelp, UINT_MAX},
        {"PlaySoundEffectPrivate", &UOExecutorModule::mf_PlaySoundEffectPrivate, UINT_MAX},
        {"ConsumeSubstance", &UOExecutorModule::mf_ConsumeSubstance, UINT_MAX},
        {"FindSubstance", &UOExecutorModule::mf_FindSubstance, UINT_MAX},
        {"Shutdown", &UOExecutorModule::mf_Shutdown, UINT_MAX},
        {"OpenPaperdoll", &UOExecutorModule::mf_OpenPaperdoll, UINT_MAX},
        {"SendSkillWindow", &UOExecutorModule::mf_SendSkillWindow, UINT_MAX},
        {"ReserveItem", &UOExecutorModule::mf_ReserveItem, UINT_MAX},
        {"ReleaseItem", &UOExecutorModule::mf_ReleaseItem, UINT_MAX},
        {"GetStandingHeight", &UOExecutorModule::mf_GetStandingHeight, UINT_MAX},
        {"GetStandingLayers", &UOExecutorModule::mf_GetStandingLayers, UINT_MAX},
        {"AssignRectToWeatherRegion", &UOExecutorModule::mf_AssignRectToWeatherRegion, UINT_MAX},
        {"CreateAccount", &UOExecutorModule::mf_CreateAccount, UINT_MAX},
        {"FindAccount", &UOExecutorModule::mf_FindAccount, UINT_MAX},
        {"ListAccounts", &UOExecutorModule::mf_ListAccounts, UINT_MAX},

        // { "AssignMultiComponent",   &UOExecutorModule::mf_AssignMultiComponent , UINT_MAX},
        {"SetScriptController", &UOExecutorModule::mf_SetScriptController, UINT_MAX},
        {"PolCore", &UOExecutorModule::mf_PolCore, UINT_MAX},
        {"GetWorldHeight", &UOExecutorModule::mf_GetWorldHeight, UINT_MAX},
        {"StartSpellEffect", &UOExecutorModule::mf_StartSpellEffect, UINT_MAX},
        {"GetSpellDifficulty", &UOExecutorModule::mf_GetSpellDifficulty, UINT_MAX},
        {"SpeakPowerWords", &UOExecutorModule::mf_SpeakPowerWords, UINT_MAX},
        {"GetMultiDimensions", &UOExecutorModule::mf_GetMultiDimensions, UINT_MAX},
        {"DestroyMulti", &UOExecutorModule::mf_DestroyMulti, UINT_MAX},
        {"SendTextEntryGump", &UOExecutorModule::mf_SendTextEntryGump, UINT_MAX},
        {"SendDialogGump", &UOExecutorModule::mf_SendGumpMenu, UINT_MAX},
        {"CloseGump", &UOExecutorModule::mf_CloseGump, UINT_MAX},
        {"CloseWindow", &UOExecutorModule::mf_CloseWindow, UINT_MAX},
        {"SendEvent", &UOExecutorModule::mf_SendEvent, UINT_MAX},
        {"PlayMovingEffectXyz", &UOExecutorModule::mf_PlayMovingEffectXyz, UINT_MAX},
        {"GetEquipmentByLayer", &UOExecutorModule::mf_GetEquipmentByLayer, UINT_MAX},
        {"GetObjtypeByName", &UOExecutorModule::mf_GetObjtypeByName, UINT_MAX},
        {"ListHostiles", &UOExecutorModule::mf_ListHostiles, UINT_MAX},
        {"DisconnectClient", &UOExecutorModule::mf_DisconnectClient, UINT_MAX},
        {"GetRegionName", &UOExecutorModule::mf_GetRegionName, UINT_MAX},
        {"GetRegionNameAtLocation", &UOExecutorModule::mf_GetRegionNameAtLocation, UINT_MAX},
        {"GetRegionLightLevelAtLocation", &UOExecutorModule::mf_GetRegionLightLevelAtLocation,
         UINT_MAX},
        {"GetRegionString", &UOExecutorModule::mf_GetRegionString, UINT_MAX},
        {"PlayStationaryEffect", &UOExecutorModule::mf_PlayStationaryEffect, UINT_MAX},
        {"GetMapInfo", &UOExecutorModule::mf_GetMapInfo, UINT_MAX},
        {"ListObjectsInBox", &UOExecutorModule::mf_ListObjectsInBox, UINT_MAX},
        {"ListItemsInBoxOfObjType", &UOExecutorModule::mf_ListItemsInBoxOfObjType, UINT_MAX},
        {"ListObjectsInBoxOfClass", &UOExecutorModule::mf_ListObjectsInBoxOfClass, UINT_MAX},
        {"ListMultisInBox", &UOExecutorModule::mf_ListMultisInBox, UINT_MAX},
        {"ListStaticsInBox", &UOExecutorModule::mf_ListStaticsInBox, UINT_MAX},
        {"ListEquippedItems", &UOExecutorModule::mf_ListEquippedItems, UINT_MAX},
        {"ConsumeReagents", &UOExecutorModule::mf_ConsumeReagents, UINT_MAX},
        {"SendPacket", &UOExecutorModule::mf_SendPacket, UINT_MAX},
        {"SendQuestArrow", &UOExecutorModule::mf_SendQuestArrow, UINT_MAX},
        {"RequestInput", &UOExecutorModule::mf_PromptInput, UINT_MAX},
        {"ReadGameClock", &UOExecutorModule::mf_ReadGameClock, UINT_MAX},
        {"GrantPrivilege", &UOExecutorModule::mf_GrantPrivilege, UINT_MAX},
        {"RevokePrivilege", &UOExecutorModule::mf_RevokePrivilege, UINT_MAX},
        {"EquipFromTemplate", &UOExecutorModule::mf_EquipFromTemplate, UINT_MAX},
        {"GetHarvestDifficulty", &UOExecutorModule::mf_GetHarvestDifficulty, UINT_MAX},
        {"HarvestResource", &UOExecutorModule::mf_HarvestResource, UINT_MAX},
        {"RestartScript", &UOExecutorModule::mf_RestartScript, UINT_MAX},
        {"EnableEvents", &UOExecutorModule::mf_EnableEvents, UINT_MAX},
        {"DisableEvents", &UOExecutorModule::mf_DisableEvents, UINT_MAX},
        {"EquipItem", &UOExecutorModule::mf_EquipItem, UINT_MAX},
        {"MoveItemToContainer", &UOExecutorModule::mf_MoveItemToContainer, UINT_MAX},
        {"MoveItemToSecureTradeWin", &UOExecutorModule::mf_MoveItemToSecureTradeWin, UINT_MAX},
        {"FindObjtypeInContainer", &UOExecutorModule::mf_FindObjtypeInContainer, UINT_MAX},
        {"SendOpenSpecialContainer", &UOExecutorModule::mf_SendOpenSpecialContainer, UINT_MAX},
        {"SecureTradeWin", &UOExecutorModule::mf_SecureTradeWin, UINT_MAX},
        {"CloseTradeWindow", &UOExecutorModule::mf_CloseTradeWindow, UINT_MAX},
        {"SendBuyWindow", &UOExecutorModule::mf_SendBuyWindow, UINT_MAX},
        {"SendSellWindow", &UOExecutorModule::mf_SendSellWindow, UINT_MAX},
        {"CreateItemInContainer", &UOExecutorModule::mf_CreateItemInContainer, UINT_MAX},
        {"CreateItemInInventory", &UOExecutorModule::mf_CreateItemInInventory, UINT_MAX},
        {"ListMobilesNearLocationEx", &UOExecutorModule::mf_ListMobilesNearLocationEx, UINT_MAX},
        {"SystemFindObjectBySerial", &UOExecutorModule::mf_SystemFindObjectBySerial, UINT_MAX},
        {"ListItemsNearLocationOfType", &UOExecutorModule::mf_ListItemsNearLocationOfType,
         UINT_MAX},
        {"ListItemsNearLocationWithFlag", &UOExecutorModule::mf_ListItemsNearLocationWithFlag,
         UINT_MAX},
        {"ListStaticsAtLocation", &UOExecutorModule::mf_ListStaticsAtLocation, UINT_MAX},
        {"ListStaticsNearLocation", &UOExecutorModule::mf_ListStaticsNearLocation, UINT_MAX},
        {"ListGhostsNearLocation", &UOExecutorModule::mf_ListGhostsNearLocation, UINT_MAX},
        {"ListMobilesInLineOfSight", &UOExecutorModule::mf_ListMobilesInLineOfSight, UINT_MAX},
        {"Distance", &UOExecutorModule::mf_Distance, UINT_MAX},
        {"CoordinateDistance", &UOExecutorModule::mf_CoordinateDistance, UINT_MAX},
        {"DistanceEuclidean", &UOExecutorModule::mf_DistanceEuclidean, UINT_MAX},
        {"CoordinateDistanceEuclidean", &UOExecutorModule::mf_CoordinateDistanceEuclidean,
         UINT_MAX},
        {"GetCoordsInLine", &UOExecutorModule::mf_GetCoordsInLine, UINT_MAX},
        {"GetFacing", &UOExecutorModule::mf_GetFacing, UINT_MAX},
        {"SetRegionLightLevel", &UOExecutorModule::mf_SetRegionLightLevel, UINT_MAX},
        {"SetRegionWeatherLevel", &UOExecutorModule::mf_SetRegionWeatherLevel, UINT_MAX},
        {"EraseObjProperty", &UOExecutorModule::mf_EraseObjProperty, UINT_MAX},
        {"GetGlobalProperty", &UOExecutorModule::mf_GetGlobalProperty, UINT_MAX},
        {"SetGlobalProperty", &UOExecutorModule::mf_SetGlobalProperty, UINT_MAX},
        {"EraseGlobalProperty", &UOExecutorModule::mf_EraseGlobalProperty, UINT_MAX},
        {"GetGlobalPropertyNames", &UOExecutorModule::mf_GetGlobalPropertyNames, UINT_MAX},
        {"SaveWorldState", &UOExecutorModule::mf_SaveWorldState, UINT_MAX},
        {"CreateMultiAtLocation", &UOExecutorModule::mf_CreateMultiAtLocation, UINT_MAX},
        {"TargetMultiPlacement", &UOExecutorModule::mf_TargetMultiPlacement, UINT_MAX},
        {"Resurrect", &UOExecutorModule::mf_Resurrect, UINT_MAX},
        {"CreateNpcFromTemplate", &UOExecutorModule::mf_CreateNpcFromTemplate, UINT_MAX},
        {"RegisterForSpeechEvents", &UOExecutorModule::mf_RegisterForSpeechEvents, UINT_MAX},
        {"EnumerateOnlineCharacters", &UOExecutorModule::mf_EnumerateOnlineCharacters, UINT_MAX},
        {"PrintTextAbove", &UOExecutorModule::mf_PrintTextAbove, UINT_MAX},
        {"PrintTextAbovePrivate", &UOExecutorModule::mf_PrivateTextAbove, UINT_MAX},

        {"Accessible", &UOExecutorModule::mf_Accessible, UINT_MAX},
        {"ApplyConstraint", &UOExecutorModule::mf_ApplyConstraint, UINT_MAX},
        {"Attach", &UOExecutorModule::mf_Attach, UINT_MAX},
        {"broadcast", &UOExecutorModule::broadcast, UINT_MAX},
        {"CheckLineOfSight", &UOExecutorModule::mf_CheckLineOfSight, UINT_MAX},
        {"CheckLosAt", &UOExecutorModule::mf_CheckLosAt, UINT_MAX},

        {"CreateItemInBackpack", &UOExecutorModule::mf_CreateItemInBackpack, UINT_MAX},
        {"CreateItemAtLocation", &UOExecutorModule::mf_CreateItemAtLocation, UINT_MAX},

        {"CreateItemCopyAtLocation", &UOExecutorModule::mf_CreateItemCopyAtLocation, UINT_MAX},

        {"DestroyItem", &UOExecutorModule::mf_DestroyItem, UINT_MAX},
        {"Detach", &UOExecutorModule::mf_Detach, UINT_MAX},
        {"EnumerateItemsInContainer", &UOExecutorModule::mf_EnumerateItemsInContainer, 2},
        {"FindPath", &UOExecutorModule::mf_FindPath, UINT_MAX},
        {"GetAmount", &UOExecutorModule::mf_GetAmount, UINT_MAX},
        {"GetMenuObjTypes", &UOExecutorModule::mf_GetMenuObjTypes, UINT_MAX},
        {"GetObjProperty", &UOExecutorModule::mf_GetObjProperty, UINT_MAX},
        {"GetObjPropertyNames", &UOExecutorModule::mf_GetObjPropertyNames, UINT_MAX},
        {"GetObjType", &UOExecutorModule::mf_GetObjType, UINT_MAX},
        {"GetPosition", &UOExecutorModule::mf_GetPosition, UINT_MAX},
        {"IsStackable", &UOExecutorModule::mf_IsStackable, UINT_MAX},
        {"ListItemsAtLocation", &UOExecutorModule::mf_ListItemsAtLocation, UINT_MAX},
        {"ListItemsNearLocation", &UOExecutorModule::mf_ListItemsNearLocation, UINT_MAX},
        {"ListMobilesNearLocation", &UOExecutorModule::mf_ListMobilesNearLocation, UINT_MAX},
        {"PerformAction", &UOExecutorModule::mf_PerformAction, UINT_MAX},
        {"PlayLightningBoltEffect", &UOExecutorModule::mf_PlayLightningBoltEffect, UINT_MAX},
        {"PlayMovingEffect", &UOExecutorModule::mf_PlayMovingEffect, UINT_MAX},
        {"PlayObjectCenteredEffect", &UOExecutorModule::mf_PlayObjectCenteredEffect, UINT_MAX},
        {"PlaySoundEffect", &UOExecutorModule::mf_PlaySoundEffect, UINT_MAX},
        {"PlaySoundEffectXYZ", &UOExecutorModule::mf_PlaySoundEffectXYZ, UINT_MAX},
        {"PlayMusic", &UOExecutorModule::mf_PlayMusic, UINT_MAX},
        {"SelectMenuItem2", &UOExecutorModule::mf_SelectMenuItem, UINT_MAX},
        {"SendSysMessage", &UOExecutorModule::mf_SendSysMessage, 4},
        {"SetObjProperty", &UOExecutorModule::mf_SetObjProperty, UINT_MAX},
        {"SetName", &UOExecutorModule::mf_SetName, UINT_MAX},
        {"SubtractAmount", &UOExecutorModule::mf_SubtractAmount, UINT_MAX},
        {"Target", &UOExecutorModule::mf_Target, UINT_MAX},
        {"TargetCoordinates", &UOExecutorModule::mf_TargetCoordinates, UINT_MAX},
        {"CancelTarget", &UOExecutorModule::mf_TargetCancel, UINT_MAX},
        {"UseItem", &UOExecutorModule::mf_UseItem, UINT_MAX},

        {"CreateMenu", &UOExecutorModule::mf_CreateMenu, UINT_MAX},
        {"AddMenuItem", &UOExecutorModule::mf_AddMenuItem, UINT_MAX},

        {"PlayStationaryEffectEx", &UOExecutorModule::mf_PlayStationaryEffect_Ex, UINT_MAX},
        {"PlayObjectCenteredEffectEx", &UOExecutorModule::mf_PlayObjectCenteredEffect_Ex, UINT_MAX},
        {"PlayMovingEffectEx", &UOExecutorModule::mf_PlayMovingEffect_Ex, UINT_MAX},
        {"PlayMovingEffectXyzEx", &UOExecutorModule::mf_PlayMovingEffectXyz_Ex, UINT_MAX},

        {"UpdateItem", &UOExecutorModule::mf_UpdateItem, UINT_MAX},
        {"UpdateMobile", &UOExecutorModule::mf_UpdateMobile, UINT_MAX},
        {"CheckLosBetween", &UOExecutorModule::mf_CheckLosBetween, UINT_MAX},
        {"CanWalk", &UOExecutorModule::mf_CanWalk, UINT_MAX},
        {"SendCharProfile", &UOExecutorModule::mf_SendCharProfile, UINT_MAX},
        {"SendOverallSeason", &UOExecutorModule::mf_SendOverallSeason, UINT_MAX},
        {"ListOfflineMobilesInRealm", &UOExecutorModule::mf_ListOfflineMobilesInRealm, UINT_MAX},
        {"ListMobilesInBox", &UOExecutorModule::mf_ListMobilesInBox, UINT_MAX},
        {"GetMidpointCircleCoords", &UOExecutorModule::mf_GetMidpointCircleCoords, UINT_MAX},

        {"SendPopUpMenu", &UOExecutorModule::mf_SendPopUpMenu, UINT_MAX},
        {"SingleClick", &UOExecutorModule::mf_SingleClick, UINT_MAX},
        {"ListStaticsNearLocationOfType", &UOExecutorModule::mf_ListStaticsNearLocationOfType,
         UINT_MAX},
        {"ListStaticsNearLocationWithFlag", &UOExecutorModule::mf_ListStaticsNearLocationWithFlag,
         UINT_MAX}};
template <>
const char* Bscript::TmplExecutorModule<Module::UOExecutorModule>::modname = "UO";
}  // namespace Bscript
}  // namespace Pol
