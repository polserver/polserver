/** @file
 *
 * @par History
 * - 2005/04/28 Shinigami: added mf_SecureTradeWin - to init secure trade via script over long
 * distances
 *                         added mf_MoveItemToSecureTradeWin - to move item to secure trade window
 * via script
 * - 2005/06/01 Shinigami: added mf_Attach - to attach a Script to a Character
 *                         added mf_ListStaticsAtLocation - list static Items at location
 *                         added mf_ListStaticsNearLocation - list static Items around location
 *                         added mf_GetStandingLayers - get layer a mobile can stand
 * - 2005/06/06 Shinigami: flags added to mf_ListStatics*
 * - 2005/07/04 Shinigami: added mf_ListStaticsInBox - list static items in box
 * - 2005/09/23 Shinigami: added mf_SendStatus - to send full status packet to support extensions
 * - 2006/01/18 Shinigami: added attached_npc_ - to get attached NPC from AI-Script-Process Obj
 * - 2006/05/07 Shinigami: mf_SendBuyWindow & mf_SendSellWindow - added Flags to send Item
 * Description using AoS Tooltips
 * - 2006/05/24 Shinigami: added mf_SendCharacterRaceChanger - change Hair, Beard and Color
 * - 2007/05/03 Turley:    added mf_GetRegionNameAtLocation - get name of justice region
 * - 2007/05/04 Shinigami: added mf_GetRegionName - get name of justice region by objref
 * - 2008/07/08 Turley:    Added mf_IsStackable - Is item stackable?
 * - 2009/08/08 MuadDib:   mf_SetRawSkill(),  mf_GetRawSkill(),  mf_ApplyRawDamage(), mf_GameStat(),
 *                         mf_AwardRawPoints(), old replace_properties(), mf_GetSkill() cleaned out.
 * - 2009/09/10 Turley:    CompressedGump support (Grin)
 * - 2009/10/22 Turley:    added CanWalk()
 * - 2009/12/17 Turley:    CloseWindow( character, type, object ) - Tomi
 */


#ifndef H_UOMOD_H
#define H_UOMOD_H

#include "../../bscript/execmodl.h"
#include "../../clib/rawtypes.h"
#include "../../plib/poltype.h"
#include "../reftypes.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class ObjArray;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Realms
{
class Realm;
}
namespace Core
{
class Menu;
class UContainer;
class UOExecutor;
class UObject;
}  // namespace Core
namespace Mobile
{
class Character;
}
namespace Items
{
class Item;
}
namespace Multi
{
class UBoat;
}
namespace Node
{
template <typename PolModule>
class NodeModuleWrap;
}

namespace Module
{
class UOExecutorModule : public Bscript::TmplExecutorModule<UOExecutorModule>
{
public:
  Bscript::BObjectImp* mf_SendStatus( /* mob */ );

  Bscript::BObjectImp* mf_MoveObjectToLocation();
  Bscript::BObjectImp* mf_SendCharacterRaceChanger( /* Character */ );
  Bscript::BObjectImp* mf_SendHousingTool();
  Bscript::BObjectImp* mf_SendOpenBook();
  Bscript::BObjectImp* mf_SelectColor();
  Bscript::BObjectImp* mf_AddAmount();

  Bscript::BObjectImp* mf_SendViewContainer();
  Bscript::BObjectImp* mf_GetObjPropertyNames();
  Bscript::BObjectImp* mf_CreateItemCopyAtLocation( /* x,y,z,item */ );
  Bscript::BObjectImp* mf_SendInstaResDialog();
  Bscript::BObjectImp* mf_FindAccount();
  Bscript::BObjectImp* mf_ListAccounts();
  Bscript::BObjectImp* mf_SendStringAsTipWindow();
  Bscript::BObjectImp* mf_GetCommandHelp();
  Bscript::BObjectImp* mf_PlaySoundEffectPrivate();
  Bscript::BObjectImp* mf_ConsumeSubstance();
  Bscript::BObjectImp* mf_FindSubstance();
  Bscript::BObjectImp* mf_Shutdown();
  Bscript::BObjectImp* mf_OpenPaperdoll();
  Bscript::BObjectImp* mf_SendSkillWindow();
  Bscript::BObjectImp* mf_ReserveItem();
  Bscript::BObjectImp* mf_ReleaseItem();
  Bscript::BObjectImp* mf_GetStandingHeight();
  Bscript::BObjectImp* mf_GetStandingLayers( /* x, y, flags, realm */ );
  Bscript::BObjectImp* mf_AssignRectToWeatherRegion();
  Bscript::BObjectImp* mf_SetRegionWeatherLevel();
  Bscript::BObjectImp* mf_CreateAccount();
  Bscript::BObjectImp* mf_SetScriptController();
  Bscript::BObjectImp* mf_PolCore();
  Bscript::BObjectImp* mf_GetWorldHeight();
  Bscript::BObjectImp* mf_StartSpellEffect();
  Bscript::BObjectImp* mf_GetSpellDifficulty();
  Bscript::BObjectImp* mf_SpeakPowerWords();

  Bscript::BObjectImp* mf_GetMultiDimensions();
  Bscript::BObjectImp* mf_DestroyMulti();
  Bscript::BObjectImp* mf_SendTextEntryGump();
  Bscript::BObjectImp* mf_SendGumpMenu();
  Bscript::BObjectImp* mf_CloseGump();
  Bscript::BObjectImp* mf_CloseWindow( /* chr, type, who */ );
  Bscript::BObjectImp* mf_SendEvent();
  Bscript::BObjectImp* mf_PlayMovingEffectXyz();
  Bscript::BObjectImp* mf_GetEquipmentByLayer();
  Bscript::BObjectImp* mf_GetObjtypeByName();

  Bscript::BObjectImp* mf_ListHostiles();
  Bscript::BObjectImp* mf_DisconnectClient();
  Bscript::BObjectImp* mf_GetRegionName( /* objref */ );
  Bscript::BObjectImp* mf_GetRegionNameAtLocation( /* x, y, realm */ );
  Bscript::BObjectImp* mf_GetRegionString();
  Bscript::BObjectImp* mf_GetRegionLightLevelAtLocation( /* x, y, realm */ );

  Bscript::BObjectImp* mf_PlayStationaryEffect();
  Bscript::BObjectImp* mf_GetMapInfo();
  Bscript::BObjectImp* mf_ListObjectsInBox( /* x1, y1, z1, x2, y2, z2, realm */ );
  Bscript::BObjectImp* mf_ListItemsInBoxOfObjType( /* objtype, x1, y1, z1, x2, y2, z2, realm */ );
  Bscript::BObjectImp* mf_ListObjectsInBoxOfClass( /* POL_Class, x1, y1, z1, x2, y2, z2, realm */ );
  Bscript::BObjectImp* mf_ListMobilesInBox( /* x1, y1, z1, x2, y2, z2, realm */ );
  Bscript::BObjectImp* mf_ListMultisInBox( /* x1, y1, z1, x2, y2, z2, realm */ );
  Bscript::BObjectImp* mf_ListStaticsInBox( /* x1, y1, z1, x2, y2, z2, flags, realm */ );
  Bscript::BObjectImp* mf_ListEquippedItems();
  Bscript::BObjectImp* mf_ConsumeReagents();

  Bscript::BObjectImp* mf_SendPacket();
  Bscript::BObjectImp* mf_SendQuestArrow();

  Bscript::BObjectImp* mf_PromptInput();

  Bscript::BObjectImp* mf_ReadGameClock();

  Bscript::BObjectImp* mf_GrantPrivilege();
  Bscript::BObjectImp* mf_RevokePrivilege();

  Bscript::BObjectImp* mf_EquipFromTemplate();

  Bscript::BObjectImp* mf_GetHarvestDifficulty();
  Bscript::BObjectImp* mf_HarvestResource();

  Bscript::BObjectImp* mf_RestartScript();
  Bscript::BObjectImp* mf_EnableEvents();
  Bscript::BObjectImp* mf_DisableEvents();
  Bscript::BObjectImp* mf_EquipItem();
  Bscript::BObjectImp* mf_MoveItemToContainer();
  Bscript::BObjectImp* mf_MoveItemToSecureTradeWin( /* item, character */ );
  Bscript::BObjectImp* mf_FindObjtypeInContainer();
  Bscript::BObjectImp* mf_SendOpenSpecialContainer();
  Bscript::BObjectImp* mf_SecureTradeWin( /* who, who2 */ );
  Bscript::BObjectImp* mf_CloseTradeWindow();


  Bscript::BObjectImp* mf_SendBuyWindow( /* character, container, vendor, items, flags */ );
  Bscript::BObjectImp* mf_SendSellWindow( /* character, vendor, i1, i2, i3, flags */ );
  Bscript::BObjectImp* mf_ListMobilesNearLocationEx( /* x, y, z, range, flags, realm */ );
  Bscript::BObjectImp* mf_CreateItemInContainer();
  Bscript::BObjectImp* mf_CreateItemInInventory();
  Bscript::BObjectImp* mf_SystemFindObjectBySerial();
  Bscript::BObjectImp* mf_ListItemsNearLocationOfType();
  Bscript::BObjectImp* mf_ListGhostsNearLocation();
  Bscript::BObjectImp* mf_ListMobilesInLineOfSight();
  Bscript::BObjectImp* mf_Distance();
  Bscript::BObjectImp* mf_DistanceEuclidean();
  Bscript::BObjectImp* mf_CoordinateDistance();
  Bscript::BObjectImp* mf_CoordinateDistanceEuclidean();
  Bscript::BObjectImp* mf_GetCoordsInLine();
  Bscript::BObjectImp* mf_GetFacing();
  Bscript::BObjectImp* mf_SetRegionLightLevel();
  Bscript::BObjectImp* mf_EraseObjProperty();
  Bscript::BObjectImp* mf_GetGlobalProperty();
  Bscript::BObjectImp* mf_SetGlobalProperty();
  Bscript::BObjectImp* mf_EraseGlobalProperty();
  Bscript::BObjectImp* mf_GetGlobalPropertyNames();
  Bscript::BObjectImp* mf_SaveWorldState();
  Bscript::BObjectImp* mf_CreateMultiAtLocation();
  Bscript::BObjectImp* mf_TargetMultiPlacement();

  Bscript::BObjectImp* mf_Resurrect();
  Bscript::BObjectImp* mf_CreateNpcFromTemplate();
  Bscript::BObjectImp* mf_RegisterForSpeechEvents();
  Bscript::BObjectImp* mf_EnumerateOnlineCharacters();
  Bscript::BObjectImp* mf_PrintTextAbove();
  Bscript::BObjectImp* mf_PrivateTextAbove();

  Bscript::BObjectImp* mf_Attach( /* Character */ );
  Bscript::BObjectImp* mf_Detach();

  Bscript::BObjectImp* broadcast();

  Bscript::BObjectImp* send_open_special_container();
  Bscript::BObjectImp* create_item_in_container();
  Bscript::BObjectImp* find_objtype_in_container();

  Bscript::BObjectImp* mf_Accessible();       // Character, Item
  Bscript::BObjectImp* mf_ApplyConstraint();  // ObjArray, ConfigFile, propname, minvalue

  Bscript::BObjectImp* mf_CheckLineOfSight();  // Character, Object
  Bscript::BObjectImp* mf_CheckLosAt();
  Bscript::BObjectImp* mf_CreateItemInBackpack();  // Character, ObjectType, Amount

  Bscript::BObjectImp* mf_CreateItemAtLocation( /* x,y,z,objtype,amount */ );

  Bscript::BObjectImp* mf_DestroyItem();
  Bscript::BObjectImp* mf_FindPath();         // x1, y1, z1, x2, y2, z2
  Bscript::BObjectImp* mf_GetAmount();        // Item
  Bscript::BObjectImp* mf_GetMenuObjTypes();  // MenuName
  Bscript::BObjectImp* mf_GetObjProperty();
  Bscript::BObjectImp* mf_GetObjType();  // Item
  Bscript::BObjectImp* mf_GetPosition();
  Bscript::BObjectImp* mf_GetStats();
  Bscript::BObjectImp* mf_GetStatus();

  Bscript::BObjectImp* mf_ListItemsAtLocation();  // x,y,z
  Bscript::BObjectImp* mf_ListMobilesNearLocation( /* x, y, z, range, realm */ );
  Bscript::BObjectImp* mf_ListItemsNearLocationWithFlag();  // DAVE - flag is a tiledata.mul flag
  Bscript::BObjectImp* mf_ListStaticsAtLocation( /* x, y, flags, realm */ );
  Bscript::BObjectImp* mf_ListStaticsNearLocation( /* x, y, range, flags, realm */ );
  Bscript::BObjectImp* mf_ListOfflineMobilesInRealm( /*realm*/ );

  Bscript::BObjectImp* mf_PerformAction();  // character, action
  Bscript::BObjectImp* mf_PlayAnimation();
  Bscript::BObjectImp* mf_PlayLightningBoltEffect();  // center
  Bscript::BObjectImp* mf_PlayMovingEffect();         // src dst effect speed loop explode
  Bscript::BObjectImp* mf_PlayObjectCenteredEffect();
  Bscript::BObjectImp* mf_PlaySoundEffect();
  Bscript::BObjectImp* mf_Range();  // Object, Object
  Bscript::BObjectImp* mf_SetName();
  Bscript::BObjectImp* mf_SetObjProperty();

  Bscript::BObjectImp* mf_SelectMenuItem();  // character, menuname
  Bscript::BObjectImp* mf_SendSysMessage();  // Character, Text
  Bscript::BObjectImp* mf_SubtractAmount();  // Item, Amount
  Bscript::BObjectImp* mf_Target();          // Character to choose
  Bscript::BObjectImp* mf_TargetCancel();    // Character to cancel for

  Bscript::BObjectImp* mf_ListItemsNearLocation();
  Bscript::BObjectImp* mf_EnumerateItemsInContainer();
  Bscript::BObjectImp* mf_TargetCoordinates();

  Bscript::BObjectImp* mf_CreateMenu();
  Bscript::BObjectImp* mf_AddMenuItem();

  Bscript::BObjectImp* mf_UseItem();

  Bscript::BObjectImp* mf_IsStackable();  // item1, item2

  Bscript::BObjectImp* mf_PlayMovingEffect_Ex(
      /*src,dst,effect,speed,duration,hue,render,direction,explode,effect3d*/ );
  Bscript::BObjectImp* mf_PlayMovingEffectXyz_Ex(
      /*sx,sy,sz,dx,dy,dz,realm,effect,speed,duration,hue,render,direction,explode,effect3d*/ );
  Bscript::BObjectImp* mf_PlayObjectCenteredEffect_Ex(
      /*src,effect,speed,duration,hue,render,layer,explode,effect3d*/ );
  Bscript::BObjectImp* mf_PlayStationaryEffect_Ex(
      /*x,y,z,realm,effect,speed,duration,hue,render,layer,explode,effect3d*/ );

  Bscript::BObjectImp* mf_UpdateMobile( /*mob*/ );
  Bscript::BObjectImp* mf_UpdateItem( /*item*/ );
  Bscript::BObjectImp* mf_CheckLosBetween( /*x1,y1,z1,x2,y2,z2,realm*/ );

  Bscript::BObjectImp* mf_PlaySoundEffectXYZ( /*x,y,z,effect,realm*/ );
  Bscript::BObjectImp* mf_PlayMusic( /*char, musicid*/ );

  Bscript::BObjectImp* mf_CanWalk( /*movemode, x1, y1, z1, x2_or_dir, y2 := -1, realm := DEF*/ );
  Bscript::BObjectImp* mf_SendCharProfile(
      /*chr, of_who, title, uneditable_text := array, editable_text := array*/ );
  Bscript::BObjectImp* mf_SendOverallSeason( /*season_id, playsound := 1*/ );
  Bscript::BObjectImp* mf_GetMidpointCircleCoords( /* xcenter, ycenter, radius */ );

  Bscript::BObjectImp* mf_SendPopUpMenu( /* to_whom, above, menu */ );
  Bscript::BObjectImp* mf_SingleClick( /*who, what*/ );

  Bscript::BObjectImp* mf_ListStaticsNearLocationOfType(
      /* x, y, z, range, objtype, flags, realm */ );
  Bscript::BObjectImp* mf_ListStaticsNearLocationWithFlag( /* x, y, z, range, flags, realm */ );

  Core::UOExecutor& uoexec;

  /* If we're asking a character for a target, who is it?
     that character's target_cursor_ex points to us,
     so clean up on destruct
     Same for menu selection.
     perhaps a single Character*, and an enum task?
     */
  Mobile::Character* popup_menu_selection_chr;
  Core::UObject* popup_menu_selection_above;
  Mobile::Character* prompt_chr;
  Mobile::Character* gump_chr;
  Mobile::Character* resurrect_chr;
  Mobile::Character* selcolor_chr;

  int target_options;

  Mobile::Character* attached_chr_;
  Mobile::Character* attached_npc_;
  Items::Item* attached_item_;
  Core::CharacterRef controller_;

  bool is_reserved_to_me( Items::Item* item );
  std::vector<Core::ItemRef> reserved_items_;

  bool registered_for_speech_events;

  explicit UOExecutorModule( Core::UOExecutor& exec );
  ~UOExecutorModule();

  virtual size_t sizeEstimate() const override;

protected:
  // bool getStaticMenuParam( unsigned param, Menu*& menu );
  bool getDynamicMenuParam( unsigned param, Core::Menu*& menu );
  bool getStaticOrDynamicMenuParam( unsigned param, Core::Menu*& menu );

protected:
  Bscript::BObjectImp* internal_MoveItem( Items::Item* item, Core::xcoord x, Core::ycoord y,
                                          Core::zcoord z, int flags, Realms::Realm* newrealm );
  Bscript::BObjectImp* internal_MoveCharacter( Mobile::Character* chr, Core::xcoord x,
                                               Core::ycoord y, Core::zcoord z, int flags,
                                               Realms::Realm* newrealm );
  Bscript::BObjectImp* internal_MoveBoat( Multi::UBoat* boat, Core::xcoord x, Core::ycoord y,
                                          Core::zcoord z, int flags, Realms::Realm* newrealm );
  Bscript::BObjectImp* internal_MoveContainer( Core::UContainer* container, Core::xcoord x,
                                               Core::ycoord y, Core::zcoord z, int flags,
                                               Realms::Realm* newrealm );
  static void internal_InBoxAreaChecks( unsigned short& x1, unsigned short& y1, int& z1,
                                        unsigned short& x2, unsigned short& y2, int& z2,
                                        Realms::Realm* realm );
  Bscript::BObjectImp* internal_SendUnCompressedGumpMenu( Mobile::Character* chr,
                                                          Bscript::ObjArray* layout_arr,
                                                          Bscript::ObjArray* data_arr, int x, int y,
                                                          u32 gumpid );
  Bscript::BObjectImp* internal_SendCompressedGumpMenu( Mobile::Character* chr,
                                                        Bscript::ObjArray* layout_arr,
                                                        Bscript::ObjArray* data_arr, int x, int y,
                                                        u32 gumpid );

private:  // not implemented
  UOExecutorModule( const UOExecutorModule& );
  UOExecutorModule& operator=( const UOExecutorModule& );
};
}  // namespace Module
}  // namespace Pol
#endif
