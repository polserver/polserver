/*
History
=======
2005/04/28 Shinigami: added mf_SecureTradeWin - to init secure trade via script over long distances
added mf_MoveItemToSecureTradeWin - to move item to secure trade window via script
2005/06/01 Shinigami: added mf_Attach - to attach a Script to a Character
added mf_ListStaticsAtLocation - list static Items at location
added mf_ListStaticsNearLocation - list static Items around location
added mf_GetStandingLayers - get layer a mobile can stand
2005/06/06 Shinigami: flags added to mf_ListStatics*
2005/07/04 Shinigami: added mf_ListStaticsInBox - list static items in box
2005/09/23 Shinigami: added mf_SendStatus - to send full status packet to support extensions
2006/01/18 Shinigami: added attached_npc_ - to get attached NPC from AI-Script-Process Obj
2006/05/07 Shinigami: mf_SendBuyWindow & mf_SendSellWindow - added Flags to send Item Description using AoS Tooltips
2006/05/24 Shinigami: added mf_SendCharacterRaceChanger - change Hair, Beard and Color
2007/05/03 Turley:    added mf_GetRegionNameAtLocation - get name of justice region
2007/05/04 Shinigami: added mf_GetRegionName - get name of justice region by objref
2008/07/08 Turley:    Added mf_IsStackable - Is item stackable?
2009/08/08 MuadDib:   mf_SetRawSkill(),  mf_GetRawSkill(),  mf_ApplyRawDamage(), mf_GameStat(), 
                      mf_AwardRawPoints(), old replace_properties(), mf_GetSkill() cleaned out.
2009/09/10 Turley:    CompressedGump support (Grin)
2009/10/22 Turley:    added CanWalk()
2009/12/17 Turley:    CloseWindow( character, type, object ) - Tomi

Notes
=======

*/

#ifndef H_UOMOD_H
#define H_UOMOD_H

class BObject;
class UOExecutorModule;
class UOExecutor;

#include "../../bscript/execmodl.h"
#include "../uoexhelp.h"
#include "../reftypes.h"
#include "../poltype.h"

//typedef BObject (UOExecutorModule::*UOExecutorModuleFn)();

#ifdef _MSC_VER
#	pragma pack( push, 1 )
#else
/* Ok, my build of GCC supports this, yay! */
#	pragma pack(1)
#endif
struct UOFunctionDef 
{
	const char *funcname;
	BObjectImp* (UOExecutorModule::*fptr)();
	// UOExecutorModuleFn fptr;
};
#ifdef _MSC_VER
#	pragma pack( pop )
#else
#	pragma pack()
#endif

class Character;
class Item;
class Menu;
class UObject;
class UContainer;
class Realm;
enum USKILLID;

class UOExecutorModule : public ExecutorModule
{
public:
	BObjectImp* mf_SendStatus( /* mob */ );
	
	BObjectImp* mf_MoveObjectToLocation();
	BObjectImp* mf_SendCharacterRaceChanger(/* Character */);
	BObjectImp* mf_SendHousingTool();
	BObjectImp* mf_SendOpenBook();
	BObjectImp* mf_SelectColor();
	BObjectImp* mf_AddAmount();
	
	BObjectImp* mf_SendViewContainer();
	BObjectImp* mf_GetObjPropertyNames();
	BObjectImp* mf_CreateItemCopyAtLocation(/* x,y,z,item */);
	BObjectImp* mf_SendInstaResDialog();
	BObjectImp* mf_FindAccount();
	BObjectImp* mf_ListAccounts();
	BObjectImp* mf_SendStringAsTipWindow();
	BObjectImp* mf_GetCommandHelp();
	BObjectImp* mf_PlaySoundEffectPrivate();
	BObjectImp* mf_ConsumeSubstance();
	BObjectImp* mf_FindSubstance();
	BObjectImp* mf_Shutdown();
	BObjectImp* mf_OpenPaperdoll();
	BObjectImp* mf_SendSkillWindow();
	BObjectImp* mf_ReserveItem();
	BObjectImp* mf_ReleaseItem();
	BObjectImp* mf_GetStandingHeight();
	BObjectImp* mf_GetStandingLayers(/* x, y, flags, realm */);
	BObjectImp* mf_AssignRectToWeatherRegion();
	BObjectImp* mf_SetRegionWeatherLevel();
	BObjectImp* mf_CreateAccount();
	BObjectImp* mf_SetScriptController();
	BObjectImp* mf_PolCore();
	BObjectImp* mf_GetWorldHeight();
	BObjectImp* mf_StartSpellEffect();
	BObjectImp* mf_GetSpellDifficulty();
	BObjectImp* mf_SpeakPowerWords();
	
	BObjectImp* mf_GetMultiDimensions();
	BObjectImp* mf_DestroyMulti();
	BObjectImp* mf_SendTextEntryGump();
	BObjectImp* mf_SendGumpMenu();
	BObjectImp* mf_CloseGump();
	BObjectImp* mf_CloseWindow(/* chr, type, who */); 
	BObjectImp* mf_SendEvent();
	BObjectImp* mf_PlayMovingEffectXyz();
	BObjectImp* mf_GetEquipmentByLayer();
	BObjectImp* mf_GetObjtypeByName();

	BObjectImp* mf_ListHostiles();
	BObjectImp* mf_DisconnectClient();
	BObjectImp* mf_GetRegionName(/* objref */);
	BObjectImp* mf_GetRegionNameAtLocation(/* x, y, realm */);
	BObjectImp* mf_GetRegionString();
	BObjectImp* mf_GetRegionLightLevelAtLocation(/* x, y, realm */);
   
	BObjectImp* mf_PlayStationaryEffect();
	BObjectImp* mf_GetMapInfo();
	BObjectImp* mf_ListObjectsInBox(/* x1, y1, z1, x2, y2, z2, realm */);
	BObjectImp* mf_ListMultisInBox(/* x1, y1, z1, x2, y2, z2, realm */);
	BObjectImp* mf_ListStaticsInBox(/* x1, y1, z1, x2, y2, z2, flags, realm */);
	BObjectImp* mf_ListEquippedItems();
	BObjectImp* mf_ConsumeReagents();

	BObjectImp* mf_SendPacket();
	BObjectImp* mf_SendQuestArrow();

	BObjectImp* mf_PromptInput();

	BObjectImp* mf_ReadGameClock();

	BObjectImp* mf_GrantPrivilege();
	BObjectImp* mf_RevokePrivilege();

	BObjectImp* mf_EquipFromTemplate();

	BObjectImp* mf_GetHarvestDifficulty();
	BObjectImp* mf_HarvestResource();

	BObjectImp* mf_RestartScript();
	BObjectImp* mf_EnableEvents();
	BObjectImp* mf_DisableEvents();
	BObjectImp* mf_EquipItem();
	BObjectImp* mf_MoveItemToContainer();
	BObjectImp* mf_MoveItemToSecureTradeWin(/* item, character */);
	BObjectImp* mf_FindObjtypeInContainer();
	BObjectImp* mf_SendOpenSpecialContainer();
	BObjectImp* mf_SecureTradeWin(/* who, who2 */);
	BObjectImp* mf_CloseTradeWindow();
   

	BObjectImp* mf_SendBuyWindow(/* character, container, vendor, items, flags */);
	BObjectImp* mf_SendSellWindow(/* character, vendor, i1, i2, i3, flags */);
	BObjectImp* mf_ListMobilesNearLocationEx(/* x, y, z, range, flags, realm */);
	BObjectImp* mf_CreateItemInContainer();
	BObjectImp* mf_CreateItemInInventory();
	BObjectImp* mf_SystemFindObjectBySerial();
	BObjectImp* mf_ListItemsNearLocationOfType();
	BObjectImp* mf_ListGhostsNearLocation();
	BObjectImp* mf_ListMobilesInLineOfSight();
	BObjectImp* mf_Distance();
	BObjectImp* mf_CoordinateDistance();
	BObjectImp* mf_GetCoordsInLine();
	BObjectImp* mf_GetFacing();
	BObjectImp* mf_SetRegionLightLevel();
	BObjectImp* mf_EraseObjProperty();
	BObjectImp* mf_GetGlobalProperty();
	BObjectImp* mf_SetGlobalProperty();
	BObjectImp* mf_EraseGlobalProperty();
	BObjectImp* mf_GetGlobalPropertyNames();
	BObjectImp* mf_SaveWorldState();
	BObjectImp* mf_CreateMultiAtLocation();
	BObjectImp* mf_TargetMultiPlacement();

	BObjectImp* mf_Resurrect();
	BObjectImp* mf_CreateNpcFromTemplate();
	BObjectImp* mf_RegisterForSpeechEvents();
	BObjectImp* mf_EnumerateOnlineCharacters();
	BObjectImp* mf_PrintTextAbove();
	BObjectImp* mf_PrivateTextAbove();

	BObjectImp* mf_Attach(/* Character */);
	BObjectImp* mf_Detach();

	BObjectImp* broadcast();
	
	BObjectImp* send_open_special_container();
	BObjectImp* create_item_in_container();
	BObjectImp* find_objtype_in_container();

	BObjectImp* mf_Accessible();			// Character, Item
	BObjectImp* mf_ApplyConstraint();	   // ObjArray, ConfigFile, propname, minvalue

	BObjectImp* mf_CheckLineOfSight();	  // Character, Object
	BObjectImp* mf_CheckLosAt();
	BObjectImp* mf_CreateItemInBackpack();  // Character, ObjectType, Amount

	BObjectImp* mf_CreateItemAtLocation(/* x,y,z,objtype,amount */);
		
	BObjectImp* mf_DestroyItem();
	BObjectImp* mf_FindPath();			  // x1, y1, z1, x2, y2, z2
	BObjectImp* mf_GetAmount();			 // Item
	BObjectImp* mf_GetMenuObjTypes();	   // MenuName
	BObjectImp* mf_GetObjProperty();
	BObjectImp* mf_GetObjType();			// Item
	BObjectImp* mf_GetPosition();
	BObjectImp* mf_GetStats();
	BObjectImp* mf_GetStatus();
	
	BObjectImp* mf_ListItemsAtLocation();	   // x,y,z
	BObjectImp* mf_ListMobilesNearLocation(/* x, y, z, range, realm */);
	BObjectImp* mf_ListItemsNearLocationWithFlag();	//DAVE - flag is a tiledata.mul flag
	BObjectImp* mf_ListStaticsAtLocation(/* x, y, flags, realm */);
	BObjectImp* mf_ListStaticsNearLocation(/* x, y, range, flags, realm */);

	BObjectImp* mf_PerformAction();			// character, action
	BObjectImp* mf_PlayAnimation();
	BObjectImp* mf_PlayLightningBoltEffect(); // center
	BObjectImp* mf_PlayMovingEffect();	  // src dst effect speed loop explode
	BObjectImp* mf_PlayObjectCenteredEffect();
	BObjectImp* mf_PlaySoundEffect();
	BObjectImp* mf_Range();				 // Object, Object
	BObjectImp* mf_SetName();
	BObjectImp* mf_SetObjProperty();
	
	BObjectImp* mf_SelectMenuItem();		// character, menuname
	BObjectImp* mf_SendSysMessage();		// Character, Text
	BObjectImp* mf_SubtractAmount();		// Item, Amount
	BObjectImp* mf_Target();				// Character to choose
	BObjectImp* mf_TargetCancel();			// Character to cancel for

	BObjectImp* mf_ListItemsNearLocation();
	BObjectImp* mf_EnumerateItemsInContainer();
	BObjectImp* mf_TargetCoordinates();

	BObjectImp* mf_CreateMenu();
	BObjectImp* mf_AddMenuItem();

	BObjectImp* mf_UseItem();

	BObjectImp* mf_IsStackable(); // item1, item2

	BObjectImp* mf_PlayMovingEffect_Ex(/*src,dst,effect,speed,duration,hue,render,direction,explode,effect3d*/);
	BObjectImp* mf_PlayMovingEffectXyz_Ex(/*sx,sy,sz,dx,dy,dz,realm,effect,speed,duration,hue,render,direction,explode,effect3d*/);
	BObjectImp* mf_PlayObjectCenteredEffect_Ex(/*src,effect,speed,duration,hue,render,layer,explode,effect3d*/);
	BObjectImp* mf_PlayStationaryEffect_Ex(/*x,y,z,realm,effect,speed,duration,hue,render,layer,explode,effect3d*/);

	BObjectImp* mf_UpdateMobile(/*mob*/);
	BObjectImp* mf_CheckLosBetween(/*x1,y1,z1,x2,y2,z2,realm*/);

	BObjectImp* mf_PlaySoundEffectXYZ(/*x,y,z,effect,realm*/);
	BObjectImp* mf_PlayMusic(/*char, musicid*/);

    BObjectImp* mf_CanWalk(/*movemode, x1, y1, z1, x2_or_dir, y2 := -1, realm := DEF*/);
	BObjectImp* mf_SendCharProfile(/*chr, of_who, title, uneditable_text := array, editable_text := array*/);
	BObjectImp* mf_SendOverallSeason(/*season_id, playsound := 1*/);

	UOExecutor& uoexec;
	
	/* If we're asking a character for a target, who is it?
	   that character's target_cursor_ex points to us,
	   so clean up on destruct
	   Same for menu selection.
	   perhaps a single Character*, and an enum task?
	 */
	Character* target_cursor_chr;
	Character* menu_selection_chr;
	Character* prompt_chr;
	Character* gump_chr;
	Character* textentry_chr;
	Character* resurrect_chr;
	Character* selcolor_chr;

	long target_options;

	Character* attached_chr_;
	Character* attached_npc_;
	CharacterRef controller_;

	bool is_reserved_to_me( Item* item );
	std::vector< ItemRef > reserved_items_;

	bool registered_for_speech_events;
	
	explicit UOExecutorModule(UOExecutor& exec);
	~UOExecutorModule();

protected:
	// bool getStaticMenuParam( unsigned param, Menu*& menu );
	bool getDynamicMenuParam( unsigned param, Menu*& menu );
	bool getStaticOrDynamicMenuParam( unsigned param, Menu*& menu );
		
	// class machinery
protected:
	virtual BObjectImp* execFunc( unsigned idx );
	virtual int functionIndex( const char *func );
	static UOFunctionDef function_table[];
	BObjectImp* internal_MoveItem(Item* item, xcoord x, ycoord y, zcoord z, long flags, Realm* newrealm);
	BObjectImp* internal_MoveCharacter(Character* chr, xcoord x, ycoord y, zcoord z, long flags, Realm* newrealm);
	BObjectImp* internal_MoveBoat(UBoat* boat, xcoord x, ycoord y, zcoord z, long flags, Realm* newrealm);
	BObjectImp* internal_MoveContainer(UContainer* container, xcoord x, ycoord y, zcoord z, long flags, Realm* newrealm);
	void internal_InBoxAreaChecks(unsigned short &x1, unsigned short &y1, short &z1, unsigned short &x2, unsigned short &y2, short &z2, Realm* realm);
	BObjectImp* internal_SendUnCompressedGumpMenu(Character* chr, ObjArray* layout_arr, ObjArray* data_arr, long x,long y);
	BObjectImp* internal_SendCompressedGumpMenu(Character* chr, ObjArray* layout_arr, ObjArray* data_arr, long x,long y);
	
private: // not implemented
	UOExecutorModule( const UOExecutorModule& );
	UOExecutorModule& operator=( const UOExecutorModule& );
};

#endif
