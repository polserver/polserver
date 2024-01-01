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

#include "base/range.h"
#include "clib/rawtypes.h"
#include "plib/poltype.h"
#include "polmodl.h"
#include "reftypes.h"

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
namespace Module
{
class UOExecutorModule : public Bscript::TmplExecutorModule<UOExecutorModule, Core::PolModule>
{
public:
  [[nodiscard]] Bscript::BObjectImp* mf_SendStatus( /* mob */ );

  [[nodiscard]] Bscript::BObjectImp* mf_MoveObjectToLocation();
  [[nodiscard]] Bscript::BObjectImp* mf_SendCharacterRaceChanger( /* Character */ );
  [[nodiscard]] Bscript::BObjectImp* mf_SendHousingTool();
  [[nodiscard]] Bscript::BObjectImp* mf_SendOpenBook();
  [[nodiscard]] Bscript::BObjectImp* mf_SelectColor();
  [[nodiscard]] Bscript::BObjectImp* mf_AddAmount();

  [[nodiscard]] Bscript::BObjectImp* mf_SendViewContainer();
  [[nodiscard]] Bscript::BObjectImp* mf_GetObjPropertyNames();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateItemCopyAtLocation( /* x,y,z,item */ );
  [[nodiscard]] Bscript::BObjectImp* mf_SendInstaResDialog();
  [[nodiscard]] Bscript::BObjectImp* mf_FindAccount();
  [[nodiscard]] Bscript::BObjectImp* mf_ListAccounts();
  [[nodiscard]] Bscript::BObjectImp* mf_SendStringAsTipWindow();
  [[nodiscard]] Bscript::BObjectImp* mf_GetCommandHelp();
  [[nodiscard]] Bscript::BObjectImp* mf_PlaySoundEffectPrivate();
  [[nodiscard]] Bscript::BObjectImp* mf_ConsumeSubstance();
  [[nodiscard]] Bscript::BObjectImp* mf_FindSubstance();
  [[nodiscard]] Bscript::BObjectImp* mf_Shutdown();
  [[nodiscard]] Bscript::BObjectImp* mf_OpenPaperdoll();
  [[nodiscard]] Bscript::BObjectImp* mf_SendSkillWindow();
  [[nodiscard]] Bscript::BObjectImp* mf_ReserveItem();
  [[nodiscard]] Bscript::BObjectImp* mf_ReleaseItem();
  [[nodiscard]] Bscript::BObjectImp* mf_GetStandingHeight();
  [[nodiscard]] Bscript::BObjectImp* mf_GetStandingLayers( /* x, y, flags, realm, includeitems */ );
  [[nodiscard]] Bscript::BObjectImp* mf_GetStandingCoordinates();
  [[nodiscard]] Bscript::BObjectImp* mf_AssignRectToWeatherRegion();
  [[nodiscard]] Bscript::BObjectImp* mf_SetRegionWeatherLevel();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateAccount();
  [[nodiscard]] Bscript::BObjectImp* mf_SetScriptController();
  [[nodiscard]] Bscript::BObjectImp* mf_POLCore();
  [[nodiscard]] Bscript::BObjectImp* mf_GetWorldHeight();
  [[nodiscard]] Bscript::BObjectImp* mf_StartSpellEffect();
  [[nodiscard]] Bscript::BObjectImp* mf_GetSpellDifficulty();
  [[nodiscard]] Bscript::BObjectImp* mf_SpeakPowerWords();

  [[nodiscard]] Bscript::BObjectImp* mf_GetMultiDimensions();
  [[nodiscard]] Bscript::BObjectImp* mf_DestroyMulti();
  [[nodiscard]] Bscript::BObjectImp* mf_SendTextEntryGump();
  [[nodiscard]] Bscript::BObjectImp* mf_SendDialogGump();
  [[nodiscard]] Bscript::BObjectImp* mf_CloseGump();
  [[nodiscard]] Bscript::BObjectImp* mf_CloseWindow( /* chr, type, who */ );
  [[nodiscard]] Bscript::BObjectImp* mf_SendEvent();
  [[nodiscard]] Bscript::BObjectImp* mf_PlayMovingEffectXYZ();
  [[nodiscard]] Bscript::BObjectImp* mf_GetEquipmentByLayer();
  [[nodiscard]] Bscript::BObjectImp* mf_GetObjtypeByName();

  [[nodiscard]] Bscript::BObjectImp* mf_ListHostiles();
  [[nodiscard]] Bscript::BObjectImp* mf_DisconnectClient();
  [[nodiscard]] Bscript::BObjectImp* mf_GetRegionName( /* objref */ );
  [[nodiscard]] Bscript::BObjectImp* mf_GetRegionNameAtLocation( /* x, y, realm */ );
  [[nodiscard]] Bscript::BObjectImp* mf_GetRegionString();
  [[nodiscard]] Bscript::BObjectImp* mf_GetRegionLightLevelAtLocation( /* x, y, realm */ );

  [[nodiscard]] Bscript::BObjectImp* mf_PlayStationaryEffect();
  [[nodiscard]] Bscript::BObjectImp* mf_GetMapInfo();
  [[nodiscard]] Bscript::BObjectImp* mf_ListObjectsInBox( /* x1, y1, z1, x2, y2, z2, realm */ );
  [[nodiscard]] Bscript::BObjectImp* mf_ListItemsInBoxOfObjType(
      /* objtype, x1, y1, z1, x2, y2, z2, realm */ );
  [[nodiscard]] Bscript::BObjectImp* mf_ListObjectsInBoxOfClass(
      /* POL_Class, x1, y1, z1, x2, y2, z2, realm */ );
  [[nodiscard]] Bscript::BObjectImp* mf_ListMobilesInBox( /* x1, y1, z1, x2, y2, z2, realm */ );
  [[nodiscard]] Bscript::BObjectImp* mf_ListMultisInBox( /* x1, y1, z1, x2, y2, z2, realm */ );
  [[nodiscard]] Bscript::BObjectImp* mf_ListStaticsInBox(
      /* x1, y1, z1, x2, y2, z2, flags, realm */ );
  [[nodiscard]] Bscript::BObjectImp* mf_ListEquippedItems();
  [[nodiscard]] Bscript::BObjectImp* mf_ConsumeReagents();

  [[nodiscard]] Bscript::BObjectImp* mf_SendPacket();
  [[nodiscard]] Bscript::BObjectImp* mf_SendQuestArrow();

  [[nodiscard]] Bscript::BObjectImp* mf_RequestInput();

  [[nodiscard]] Bscript::BObjectImp* mf_ReadGameClock();

  [[nodiscard]] Bscript::BObjectImp* mf_GrantPrivilege();
  [[nodiscard]] Bscript::BObjectImp* mf_RevokePrivilege();

  [[nodiscard]] Bscript::BObjectImp* mf_EquipFromTemplate();

  [[nodiscard]] Bscript::BObjectImp* mf_GetHarvestDifficulty();
  [[nodiscard]] Bscript::BObjectImp* mf_HarvestResource();

  [[nodiscard]] Bscript::BObjectImp* mf_RestartScript();
  [[nodiscard]] Bscript::BObjectImp* mf_EnableEvents();
  [[nodiscard]] Bscript::BObjectImp* mf_DisableEvents();
  [[nodiscard]] Bscript::BObjectImp* mf_EquipItem();
  [[nodiscard]] Bscript::BObjectImp* mf_MoveItemToContainer();
  [[nodiscard]] Bscript::BObjectImp* mf_MoveItemToSecureTradeWin( /* item, character */ );
  [[nodiscard]] Bscript::BObjectImp* mf_FindObjtypeInContainer();
  [[nodiscard]] Bscript::BObjectImp* mf_SendOpenSpecialContainer();
  [[nodiscard]] Bscript::BObjectImp* mf_SecureTradeWin( /* who, who2 */ );
  [[nodiscard]] Bscript::BObjectImp* mf_CloseTradeWindow();


  [[nodiscard]] Bscript::BObjectImp* mf_SendBuyWindow(
      /* character, container, vendor, items, flags */ );
  [[nodiscard]] Bscript::BObjectImp* mf_SendSellWindow(
      /* character, vendor, i1, i2, i3, flags */ );
  [[nodiscard]] Bscript::BObjectImp* mf_ListMobilesNearLocationEx(
      /* x, y, z, range, flags, realm */ );
  [[nodiscard]] Bscript::BObjectImp* mf_CreateItemInContainer();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateItemInInventory();
  [[nodiscard]] Bscript::BObjectImp* mf_SystemFindObjectBySerial();
  [[nodiscard]] Bscript::BObjectImp* mf_ListItemsNearLocationOfType();
  [[nodiscard]] Bscript::BObjectImp* mf_ListGhostsNearLocation();
  [[nodiscard]] Bscript::BObjectImp* mf_ListMobilesInLineOfSight();
  [[nodiscard]] Bscript::BObjectImp* mf_Distance();
  [[nodiscard]] Bscript::BObjectImp* mf_DistanceEuclidean();
  [[nodiscard]] Bscript::BObjectImp* mf_CoordinateDistance();
  [[nodiscard]] Bscript::BObjectImp* mf_CoordinateDistanceEuclidean();
  [[nodiscard]] Bscript::BObjectImp* mf_GetCoordsInLine();
  [[nodiscard]] Bscript::BObjectImp* mf_GetFacing();
  [[nodiscard]] Bscript::BObjectImp* mf_SetRegionLightLevel();
  [[nodiscard]] Bscript::BObjectImp* mf_EraseObjProperty();
  [[nodiscard]] Bscript::BObjectImp* mf_GetGlobalProperty();
  [[nodiscard]] Bscript::BObjectImp* mf_SetGlobalProperty();
  [[nodiscard]] Bscript::BObjectImp* mf_EraseGlobalProperty();
  [[nodiscard]] Bscript::BObjectImp* mf_GetGlobalPropertyNames();
  [[nodiscard]] Bscript::BObjectImp* mf_SaveWorldState();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateMultiAtLocation();
  [[nodiscard]] Bscript::BObjectImp* mf_TargetMultiPlacement();

  [[nodiscard]] Bscript::BObjectImp* mf_Resurrect();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateNpcFromTemplate();
  [[nodiscard]] Bscript::BObjectImp* mf_RegisterForSpeechEvents();
  [[nodiscard]] Bscript::BObjectImp* mf_EnumerateOnlineCharacters();
  [[nodiscard]] Bscript::BObjectImp* mf_PrintTextAbove();
  [[nodiscard]] Bscript::BObjectImp* mf_PrintTextAbovePrivate();

  [[nodiscard]] Bscript::BObjectImp* mf_Attach( /* Character */ );
  [[nodiscard]] Bscript::BObjectImp* mf_Detach();

  [[nodiscard]] Bscript::BObjectImp* mf_Broadcast();

  Bscript::BObjectImp* send_open_special_container();
  Bscript::BObjectImp* create_item_in_container();
  Bscript::BObjectImp* find_objtype_in_container();

  [[nodiscard]] Bscript::BObjectImp* mf_Accessible();  // Character, Item
  [[nodiscard]] Bscript::BObjectImp*
  mf_ApplyConstraint();  // ObjArray, ConfigFile, propname, minvalue

  [[nodiscard]] Bscript::BObjectImp* mf_CheckLineOfSight();  // Character, Object
  [[nodiscard]] Bscript::BObjectImp* mf_CheckLosAt();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateItemInBackpack();  // Character, ObjectType, Amount

  [[nodiscard]] Bscript::BObjectImp* mf_CreateItemAtLocation( /* x,y,z,objtype,amount */ );

  [[nodiscard]] Bscript::BObjectImp* mf_DestroyItem();
  [[nodiscard]] Bscript::BObjectImp* mf_FindPath();         // x1, y1, z1, x2, y2, z2
  [[nodiscard]] Bscript::BObjectImp* mf_GetAmount();        // Item
  [[nodiscard]] Bscript::BObjectImp* mf_GetMenuObjTypes();  // MenuName
  [[nodiscard]] Bscript::BObjectImp* mf_GetObjProperty();
  [[nodiscard]] Bscript::BObjectImp* mf_GetObjType();  // Item
  [[nodiscard]] Bscript::BObjectImp* mf_GetPosition();
  [[nodiscard]] Bscript::BObjectImp* mf_GetStats();
  [[nodiscard]] Bscript::BObjectImp* mf_GetStatus();

  [[nodiscard]] Bscript::BObjectImp* mf_ListItemsAtLocation();  // x,y,z
  [[nodiscard]] Bscript::BObjectImp* mf_ListMobilesNearLocation( /* x, y, z, range, realm */ );
  [[nodiscard]] Bscript::BObjectImp*
  mf_ListItemsNearLocationWithFlag();  // DAVE - flag is a tiledata.mul flag
  [[nodiscard]] Bscript::BObjectImp* mf_ListStaticsAtLocation( /* x, y, flags, realm */ );
  [[nodiscard]] Bscript::BObjectImp* mf_ListStaticsNearLocation( /* x, y, range, flags, realm */ );
  [[nodiscard]] Bscript::BObjectImp* mf_ListOfflineMobilesInRealm( /*realm*/ );

  [[nodiscard]] Bscript::BObjectImp* mf_PerformAction();  // character, action
  [[nodiscard]] Bscript::BObjectImp* mf_PlayAnimation();
  [[nodiscard]] Bscript::BObjectImp* mf_PlayLightningBoltEffect();  // center
  [[nodiscard]] Bscript::BObjectImp* mf_PlayMovingEffect();  // src dst effect speed loop explode
  [[nodiscard]] Bscript::BObjectImp* mf_PlayObjectCenteredEffect();
  [[nodiscard]] Bscript::BObjectImp* mf_PlaySoundEffect();
  [[nodiscard]] Bscript::BObjectImp* mf_Range();  // Object, Object
  [[nodiscard]] Bscript::BObjectImp* mf_SetName();
  [[nodiscard]] Bscript::BObjectImp* mf_SetObjProperty();

  [[nodiscard]] Bscript::BObjectImp* mf_SelectMenuItem2();  // character, menuname
  [[nodiscard]] Bscript::BObjectImp* mf_SendSysMessage();   // Character, Text
  [[nodiscard]] Bscript::BObjectImp* mf_SubtractAmount();   // Item, Amount
  [[nodiscard]] Bscript::BObjectImp* mf_Target();           // Character to choose
  [[nodiscard]] Bscript::BObjectImp* mf_CancelTarget();     // Character to cancel for

  [[nodiscard]] Bscript::BObjectImp* mf_ListItemsNearLocation();
  [[nodiscard]] Bscript::BObjectImp* mf_EnumerateItemsInContainer();
  [[nodiscard]] Bscript::BObjectImp* mf_TargetCoordinates();

  [[nodiscard]] Bscript::BObjectImp* mf_CreateMenu();
  [[nodiscard]] Bscript::BObjectImp* mf_AddMenuItem();

  [[nodiscard]] Bscript::BObjectImp* mf_UseItem();

  [[nodiscard]] Bscript::BObjectImp* mf_IsStackable();  // item1, item2

  [[nodiscard]] Bscript::BObjectImp* mf_PlayMovingEffectEx(
      /*src,dst,effect,speed,duration,hue,render,direction,explode,effect3d*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_PlayMovingEffectXYZEx(
      /*sx,sy,sz,dx,dy,dz,realm,effect,speed,duration,hue,render,direction,explode,effect3d*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_PlayObjectCenteredEffectEx(
      /*src,effect,speed,duration,hue,render,layer,explode,effect3d*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_PlayStationaryEffectEx(
      /*x,y,z,realm,effect,speed,duration,hue,render,layer,explode,effect3d*/ );

  [[nodiscard]] Bscript::BObjectImp* mf_UpdateMobile( /*mob*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_UpdateItem( /*item*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_CheckLosBetween( /*x1,y1,z1,x2,y2,z2,realm*/ );

  [[nodiscard]] Bscript::BObjectImp* mf_PlaySoundEffectXYZ( /*x,y,z,effect,realm*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_PlayMusic( /*char, musicid*/ );

  [[nodiscard]] Bscript::BObjectImp* mf_CanWalk(
      /*movemode, x1, y1, z1, x2_or_dir, y2 := -1, realm := DEF*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_SendCharProfile(
      /*chr, of_who, title, uneditable_text := array, editable_text := array*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_SendOverallSeason( /*season_id, playsound := 1*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_GetMidpointCircleCoords( /* xcenter, ycenter, radius */ );

  [[nodiscard]] Bscript::BObjectImp* mf_SendPopUpMenu( /* to_whom, above, menu */ );
  [[nodiscard]] Bscript::BObjectImp* mf_SingleClick( /*who, what*/ );

  [[nodiscard]] Bscript::BObjectImp* mf_ListStaticsNearLocationOfType(
      /* x, y, z, range, objtype, flags, realm */ );
  [[nodiscard]] Bscript::BObjectImp* mf_ListStaticsNearLocationWithFlag(
      /* x, y, z, range, flags, realm */ );

  /* If we're asking a character for a target, who is it?
     that character's target_cursor_ex points to us,
     so clean up on destruct
     Same for menu selection.
     perhaps a single Character*, and an enum task?
     */
  Mobile::Character* target_cursor_chr;
  Mobile::Character* menu_selection_chr;
  Mobile::Character* popup_menu_selection_chr;
  Core::UObject* popup_menu_selection_above;
  Mobile::Character* prompt_chr;
  Mobile::Character* gump_chr;
  Mobile::Character* textentry_chr;
  Mobile::Character* resurrect_chr;
  Mobile::Character* selcolor_chr;

  int target_options;

  Mobile::Character* attached_chr_;
  Mobile::Character* attached_npc_;
  Core::ItemRef attached_item_;
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
  Bscript::BObjectImp* internal_MoveItem( Items::Item* item, Core::Pos4d newpos, int flags );
  Bscript::BObjectImp* internal_MoveCharacter( Mobile::Character* chr, const Core::Pos4d& newpos,
                                               int flags );
  Bscript::BObjectImp* internal_MoveBoat( Multi::UBoat* boat, const Core::Pos4d& newpos,
                                          int flags );
  Bscript::BObjectImp* internal_MoveContainer( Core::UContainer* container,
                                               const Core::Pos4d& newpos, int flags );
  static Core::Range3d internal_InBoxAreaChecks( const Core::Pos2d& p1, int z1,
                                                 const Core::Pos2d& p2, int z2,
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
