////////////////////////////////////////////////////////////////
//
//	CONSTANTS
//
////////////////////////////////////////////////////////////////

// CreateMulti flags
// only for house creation:
const CRMULTI_IGNORE_MULTIS     := 0x0001;  // ignore intersecting multis
const CRMULTI_IGNORE_OBJECTS    := 0x0002;  // ignore dynamic objects
const CRMULTI_IGNORE_WORLDZ     := 0x0004;  // ignore standability, relative Z, world height
const CRMULTI_IGNORE_ALL        := 0x0007;
// only for boat creation:
const CRMULTI_FACING_NORTH      := 0x0000;
const CRMULTI_FACING_EAST       := 0x0100;
const CRMULTI_FACING_SOUTH      := 0x0200;
const CRMULTI_FACING_WEST       := 0x0300;

//	ListHostiles exclusions

const LH_FLAG_LOS               := 1;       // only include those in LOS
const LH_FLAG_INCLUDE_HIDDEN    := 2;       // include hidden characters

// FindPath flags
const FP_IGNORE_MOBILES         := 0x01;    // ignore Mobiles
const FP_IGNORE_DOORS           := 0x02;    // ignore Doors (you've to open doors by yourself)

// Send*Window flags
const VENDOR_SEND_AOS_TOOLTIP   := 0x01;    // send Item Description using AoS Tooltips
const VENDOR_BUYABLE_CONTAINER_FILTER := 0x02; // filter items in the SendSellWindow() by what is in the buyable container

const SENDDIALOGMENU_FORCE_OLD  := 0x01;    // send UnCompressed Gump

// RegisterForSpeechEvents flags
const LISTENPT_HEAR_GHOSTS      := 0x01;    // hear ghost speech in addition to living speech
const LISTENPT_HEAR_TOKENS      := 0x02;    // hear also speechtokens (only with ssopt.SeperateSpeechTokens)
const LISTENPT_NO_SPEECH        := 0x04;    // hear no normal speech (only with ssopt.SeperateSpeechTokens)

// List[Statics/Items]* flags
const ITEMS_IGNORE_STATICS      := 0x01;    // Don't list Static Items
const ITEMS_IGNORE_MULTIS       := 0x02;    // Don't list Multi Items

// special value for List[Items/Mobiles/Statics]*
const LIST_IGNORE_Z             := 0x40000000; // Ignore Z-Value and list everything

//	ListMobilesNearLocationEx exclusions
const LISTEX_FLAG_NORMAL := 0x01;
const LISTEX_FLAG_HIDDEN := 0x02;
const LISTEX_FLAG_GHOST := 0x04;
const LISTEX_FLAG_CONCEALED := 0x08;
const LISTEX_FLAG_PLAYERS_ONLY := 0x10;
const LISTEX_FLAG_NPC_ONLY := 0x20;

// ListItemsNearLocationWithFlag( x,y,z, range, flags ); tiledata flags
// Thanks to Alazane: http://dkbush.cablenet-va.com/alazane/file_formats.html#3.17
const TILEDATA_FLAG_BACKGROUND  := 0x00000001; //Background
const TILEDATA_FLAG_WEAPON      := 0x00000002; //Weapon
const TILEDATA_FLAG_TRANSPARENT := 0x00000004; //Transparent
const TILEDATA_FLAG_TRANSLUCENT := 0x00000008; //Translucent
const TILEDATA_FLAG_WALL        := 0x00000010; //Wall
const TILEDATA_FLAG_DAMAGING    := 0x00000020; //Damaging
const TILEDATA_FLAG_IMPASSABLE  := 0x00000040; //Impassable
const TILEDATA_FLAG_WET         := 0x00000080; //Wet
const TILEDATA_FLAG_UNK         := 0x00000100; //Unknown
const TILEDATA_FLAG_SURFACE     := 0x00000200; //Surface
const TILEDATA_FLAG_BRIDGE      := 0x00000400; //Bridge
const TILEDATA_FLAG_STACKABLE   := 0x00000800; //Generic/Stackable
const TILEDATA_FLAG_WINDOW      := 0x00001000; //Window
const TILEDATA_FLAG_NOSHOOT     := 0x00002000; //No Shoot
const TILEDATA_FLAG_PREFIX_A    := 0x00004000; //Prefix A
const TILEDATA_FLAG_PREFIX_AN   := 0x00008000; //Prefix An
const TILEDATA_FLAG_INTERNAL    := 0x00010000; //Internal (things like hair, beards, etc)
const TILEDATA_FLAG_FOLIAGE     := 0x00020000; //Foliage
const TILEDATA_FLAG_PARTIAL_HUE := 0x00040000; //Partial Hue
const TILEDATA_FLAG_UNK1        := 0x00080000; //Unknown 1
const TILEDATA_FLAG_MAP         := 0x00100000; //Map
const TILEDATA_FLAG_CONTAINER   := 0x00200000; //Container
const TILEDATA_FLAG_WEARABLE    := 0x00400000; //Wearable
const TILEDATA_FLAG_LIGHTSOURCE := 0x00800000; //LightSource
const TILEDATA_FLAG_ANIMATED    := 0x01000000; //Animated
const TILEDATA_FLAG_HOVEROVER   := 0x02000000; //HoverOver (gargoyle flying tiles)
const TILEDATA_FLAG_UNK2        := 0x04000000; //Unknown 2
const TILEDATA_FLAG_ARMOR       := 0x08000000; //Armor
const TILEDATA_FLAG_ROOF        := 0x10000000; //Roof
const TILEDATA_FLAG_DOOR        := 0x20000000; //Door
const TILEDATA_FLAG_STAIRBACK   := 0x40000000; //StairBack
const TILEDATA_FLAG_STAIRRIGHT  := 0x80000000; //StairRight

// GetStandingLayers( x, y, flags, realm := _DEFAULT_REALM ); mapdata flags
const MAPDATA_FLAG_NONE         := 0x0000;     // Nothing
const MAPDATA_FLAG_MOVELAND     := 0x0001;     // Move Land
const MAPDATA_FLAG_MOVESEA      := 0x0002;     // Move Sea
const MAPDATA_FLAG_BLOCKSIGHT   := 0x0004;     // Block Sight
const MAPDATA_FLAG_OVERFLIGHT   := 0x0008;     // Over Flight (gargoyle flying)
const MAPDATA_FLAG_ALLOWDROPON  := 0x0010;     // Allow DropOn
const MAPDATA_FLAG_GRADUAL      := 0x0020;     // Gradual
const MAPDATA_FLAG_BLOCKING     := 0x0040;     // Blocking
const MAPDATA_FLAG_MORE_SOLIDS  := 0x0080;     // List more Solids

const MAPDATA_FLAG_WALKBLOCK    := 0x0057;     // Move Land, Move Sea, Blocking, Block Sight, Allow DropOn
const MAPDATA_FLAG_MOVE_FLAGS   := 0x0063;     // Move Land, Move Sea, Blocking, Gradual
const MAPDATA_FLAG_DROP_FLAGS   := 0x0050;     // Blocking, Allow DropOn
const MAPDATA_FLAG_ALL          := 0xffffffff; // All

//
// Constants for MoveObjectToLocation
//
const MOVEOBJECT_NORMAL			:= 0x0;			// Implicit.  Will move if able.
const MOVEITEM_IGNOREMOVABLE	:= 0x20000000;	// Will ignore the movable property on items.
const MOVEOBJECT_FORCELOCATION	:= 0x40000000;	// Force Z location

//
// Constants for PrintTextAbove*
//
const JOURNAL_PRINT_NAME		:= 0x00;		// Implicit.  Print's the object's description / npc's name in the journal.
const JOURNAL_PRINT_YOU_SEE		:= 0x01;		// Will print "You see:" in the journal.


// Resurrect options
const RESURRECT_FORCELOCATION := 0x01;

// MoveType constants for CanInsert/OnInsert/CanRemove/OnRemove scripts

const MOVETYPE_PLAYER     := 0; // Moved by the player
const MOVETYPE_COREMOVE   := 1; // Moved with MoveItem*() (or equiv)
const MOVETYPE_CORECREATE := 2; // Created with CreateItemIn*() (or equiv)

// InsertType constants for CanInsert/OnInsert scripts
const INSERT_ADD_ITEM          := 1;
const INSERT_INCREASE_STACK    := 2;

// DeleteBy constants for CanDelete script
const DELETE_BY_PLAYER := 0;
const DELETE_BY_SCRIPT := 1;

// EnumerateItemsInContainer constants
const ENUMERATE_IGNORE_LOCKED	:= 0x1; // List content of locked container.
const ENUMERATE_ROOT_ONLY		:= 0x2; // Do not list contents of sub-containers.

// FindObjtypeInContainer constants
const FINDOBJTYPE_RECURSIVE     := 0x0; // Search in sub-containers (DEFAULT).
const FINDOBJTYPE_IGNORE_LOCKED := 0x1; // Find matches in locked containers.
const FINDOBJTYPE_ROOT_ONLY     := 0x2; // Do not find matches in sub-containers.

// FindSubstance constants
const FINDSUBSTANCE_IGNORE_LOCKED := 0x1; // Find matches in locked containers
const FINDSUBSTANCE_ROOT_ONLY     := 0x2; // Do not find matches in sub-containers.
const FINDSUBSTANCE_FIND_ALL      := 0x4; // Find all matches ignoring given amount

//	SendTextEntryGump options

const TE_CANCEL_DISABLE := 0;
const TE_CANCEL_ENABLE  := 1;

const TE_STYLE_DISABLE  := 0;
const TE_STYLE_NORMAL   := 1;
const TE_STYLE_NUMERICAL:= 2;


//	SystemFindObjectBySerial options:

const SYSFIND_SEARCH_OFFLINE_MOBILES := 1;


//	Target Options - add these together and pass as second
//	param to Target()

const TGTOPT_CHECK_LOS   := 0x0001;
const TGTOPT_NOCHECK_LOS := 0x0000;	// to be explicit
const TGTOPT_HARMFUL     := 0x0002;
const TGTOPT_NEUTRAL     := 0x0000;	// to be explicit
const TGTOPT_HELPFUL     := 0x0004;

// POLCLASS_* constants - use with obj.isa(POLCLASS_*)
const POLCLASS_UOBJECT      := 1;
const POLCLASS_ITEM         := 2;
const POLCLASS_MOBILE       := 3;
const POLCLASS_NPC          := 4;
const POLCLASS_LOCKABLE     := 5;
const POLCLASS_CONTAINER    := 6;
const POLCLASS_CORPSE       := 7;
const POLCLASS_DOOR         := 8;
const POLCLASS_SPELLBOOK    := 9;
const POLCLASS_MAP          := 10;
const POLCLASS_MULTI        := 11;
const POLCLASS_BOAT         := 12;
const POLCLASS_HOUSE        := 13;
const POLCLASS_EQUIPMENT    := 14;
const POLCLASS_ARMOR        := 15;
const POLCLASS_WEAPON       := 16;

// mobile.race constants
const RACE_HUMAN    := 0;
const RACE_ELF      := 1;
const RACE_GARGOYLE := 2;

// Don't use these outside this file, use FONT_* from client.inc
//  (and I don't know what for color)
const _DEFAULT_TEXT_FONT     := 3;
const _DEFAULT_TEXT_COLOR    := 1000;
const _DEFAULT_TEXT_REQUIREDCMD    := 0;

// Realms
const _DEFAULT_REALM  		:= "britannia";
const REALM_BRITANNIA 		:= _DEFAULT_REALM;
const REALM_BRITANNIA_ALT	:= "britannia_alt";
const REALM_ILSHENAR  		:= "ilshenar";
const REALM_MALAS     		:= "malas";
const REALM_TOKUNO    		:= "tokuno";
const REALM_TERMUR    		:= "termur";

//PerformAction
const ACTION_DIR_FORWARD  := 0;
const ACTION_DIR_BACKWARD := 1;
const ACTION_NOREPEAT     := 0;
const ACTION_REPEAT       := 1;

// Masks for EnableEvents()
const EVMASK_ALL      := 0;
const EVMASK_ONLY_PC  := 1;
const EVMASK_ONLY_NPC := 2;

//CanWalk
const CANWALK_DIR := -1;

//UpdateMobile
const UPDATEMOBILE_RECREATE := 1;
const UPDATEMOBILE_UPDATE   := 0;

//CloseWindow
const CLOSE_PAPERDOLL := 1;
const CLOSE_STATUS    := 2;
const CLOSE_PROFILE   := 8;
const CLOSE_CONTAINER := 12;

//SendCharProfile
const CHARPROFILE_NO_UNEDITABLE_TEXT := "";
const CHARPROFILE_NO_EDITABLE_TEXT := "";

//Accessible
const ACCESSIBLE_DEFAULT := -1; // uses the default from ssopt
const ACCESSIBLE_IGNOREDISTANCE := -2; // ignores the range check

////////////////////////////////////////////////////////////////
//
//	FUNCTIONS
//
////////////////////////////////////////////////////////////////

Accessible( by_character, item, range := ACCESSIBLE_DEFAULT );
AddAmount( item, amount );
AddMenuItem( menu, objtype, text, color:=0 );
ApplyConstraint( objlist, configfile, propertyname, minvalue );
AssignRectToWeatherRegion( region, xwest, ynorth, xeast, ysouth );
Attach( character );
Broadcast( text, font := _DEFAULT_TEXT_FONT, color := _DEFAULT_TEXT_COLOR, requiredcmdlevel := _DEFAULT_TEXT_REQUIREDCMD );
CancelTarget( of_whom );
CanWalk(movemode, x1, y1, z1, x2_or_dir, y2 := CANWALK_DIR, realm := _DEFAULT_REALM);
CheckLineOfSight( object1, object2 );
CheckLosAt( character, x, y, z );
CheckLosBetween( x1, y1, z1, x2, y2, z2, realm := _DEFAULT_REALM );
CloseGump( character, gumpid, response := 0 );
CloseTradeWindow( character );
CloseWindow( character, type, object );
ConsumeReagents( who, spellid );
ConsumeSubstance( container, objtype, amount );
CoordinateDistance(x1, y1, x2, y2);
CoordinateDistanceEuclidean(x1, y1, x2, y2);
CreateAccount( acctname, password, enabled );
CreateItemAtLocation( x, y, z, objtype, amount := 1, realm := _DEFAULT_REALM );
CreateItemCopyAtLocation(x, y, z, item, realm := _DEFAULT_REALM);
CreateItemInBackpack( of_character, objtype, amount := 1 );
CreateItemInContainer( container, objtype, amount := 1 );
CreateItemInInventory( container, objtype, amount := 1 );
CreateMenu( title );
CreateMultiAtLocation( x, y, z, objtype, flags := 0, realm := _DEFAULT_REALM );
CreateNpcFromTemplate( template, x, y, z, override_properties := 0, realm := _DEFAULT_REALM, force := 0);
DestroyItem( item );
DestroyMulti( multi );
Detach();
DisableEvents( eventtype );     // eventtype combination of constants from SYSEVENT.INC
DisconnectClient( character );
Distance( obj1, obj2 );
DistanceEuclidean( obj1, obj2 );
EnableEvents( eventtype, range := -1, evmask := EVMASK_ALL );  // eventtype combination of constants from SYSEVENT.INC
EnumerateItemsInContainer( container, flags := 0 );
EnumerateOnlineCharacters();
EquipFromTemplate( character, template ); // reads from equip.cfg
EquipItem( mobile, item );
EraseGlobalProperty( propname );
EraseObjProperty( object, propname );
FindAccount( acctname );
FindObjtypeInContainer( container, objtype, flags := FINDOBJTYPE_RECURSIVE );
FindPath( x1, y1, z1, x2, y2, z2, realm := _DEFAULT_REALM, flags := FP_IGNORE_MOBILES, searchskirt := 5 );
FindSubstance( container, objtype, amount, makeinuse := 0, flags := 0 );
GetAmount( item );
GetCommandHelp( character, command );
GetCoordsInLine(x1, y1, x2, y2);
GetEquipmentByLayer( character, layer );
GetFacing(from_x, from_y, to_x, to_y);
GetGlobalProperty( propname );
GetGlobalPropertyNames();
GetHarvestDifficulty( resource, x, y, tiletype, realm := _DEFAULT_REALM );
GetMapInfo( x, y, realm := _DEFAULT_REALM );
GetMenuObjTypes( menuname );
GetMultiDimensions( multiid );
GetObjProperty( object, property_name );
GetObjPropertyNames( object );
GetObjType( object );
GetObjtypeByName( name );
GetRegionLightLevelAtLocation( x, y, realm := _DEFAULT_REALM );
GetRegionName( object );
GetRegionNameAtLocation( x, y, realm := _DEFAULT_REALM );
GetRegionString( resource, x, y, propertyname, realm := _DEFAULT_REALM );
GetSpellDifficulty( spellid );
GetStandingHeight( x, y, startz, realm := _DEFAULT_REALM );
GetStandingLayers( x, y, flags := MAPDATA_FLAG_ALL, realm := _DEFAULT_REALM );
GetWorldHeight( x, y, realm := _DEFAULT_REALM );
GrantPrivilege( character, privilege );
HarvestResource( resource, x, y, b, n, realm := _DEFAULT_REALM ); // returns b*a where 0 <= a <= n
IsStackable( item1, item2 ); // checks if items can stack
ListAccounts( );
ListEquippedItems( who );
ListGhostsNearLocation( x, y, z, range, realm := _DEFAULT_REALM );
ListHostiles( character, range := 20, flags := 0 );
ListItemsAtLocation( x, y, z, realm := _DEFAULT_REALM );
ListItemsInBoxOfObjType( objtype, x1,y1,z1, x2,y2,z2, realm := _DEFAULT_REALM );
ListItemsNearLocation( x, y, z, range, realm := _DEFAULT_REALM );
ListItemsNearLocationOfType( x,y,z, range, objtype, realm := _DEFAULT_REALM );
ListItemsNearLocationWithFlag( x,y,z, range, flags, realm := _DEFAULT_REALM );
ListMobilesInLineOfSight( object, range );
ListMobilesNearLocation( x, y, z, range, realm := _DEFAULT_REALM );
ListMobilesNearLocationEx( x,y,z, range, flags, realm := _DEFAULT_REALM );
ListMultisInBox( x1,y1,z1, x2,y2,z2, realm := _DEFAULT_REALM );
ListMobilesInBox( x1,y1,z1, x2,y2,z2, realm := _DEFAULT_REALM );
ListObjectsInBox( x1,y1,z1, x2,y2,z2, realm := _DEFAULT_REALM );
ListObjectsInBoxOfClass( POL_Class, x1, y1, z1, x2, y2, z2, realm := _DEFAULT_REALM );
ListOfflineMobilesInRealm(realm := _DEFAULT_REALM);
ListStaticsAtLocation( x, y, z, flags := 0, realm := _DEFAULT_REALM );
ListStaticsInBox( x1,y1,z1, x2,y2,z2, flags := 0, realm := _DEFAULT_REALM );
ListStaticsNearLocation( x, y, z, range, flags := 0, realm := _DEFAULT_REALM );
ListStaticsNearLocationOfType( x,y,z, range, objtype, flags := 0, realm := _DEFAULT_REALM );
ListStaticsNearLocationWithFlag( x,y,z, range, flags, realm := _DEFAULT_REALM );
MoveItemToContainer( item, container, x := -1, y := -1, add_to_existing_stack := 0 );
MoveItemToSecureTradeWin( item, character );
MoveObjectToLocation( object, x, y, z, realm := _DEFAULT_REALM, flags := MOVEOBJECT_NORMAL );
OpenPaperdoll( towhom, forwhom );
PerformAction( character, action, framecount := 5, repeatcount := 1, backward := ACTION_DIR_FORWARD, repeatflag := ACTION_NOREPEAT, delay := 1 );
PlayLightningBoltEffect( center_object );
PlayMovingEffect( source, target, effect, speed, loop := 0, explode := 0 );
PlayMovingEffectEx( source, target, effect, speed, duration := 0, hue := 0, render := 0, fixeddirection := 0, explode := 0, effect3d := 0, effect3dexplode := 0, effect3dsound := 0 );
PlayMovingEffectXYZ( srcx, srcy, srcz, dstx, dsty, dstz, effect, speed, loop := 0, explode := 0, realm := _DEFAULT_REALM );
PlayMovingEffectXYZEx( srcx, srcy, srcz, dstx, dsty, dstz, realm := _DEFAULT_REALM, effect, speed, duration := 0, hue := 0, render := 0, fixeddirection := 0, explode := 0, effect3d := 0, effect3dexplode := 0, effect3dsound := 0 );
PlayObjectCenteredEffect( center, effect, speed, loop := 0 );
PlayObjectCenteredEffectEx( center, effect, speed, duration := 0, hue := 0, render := 0, layer := 0, effect3d := 0 );
PlaySoundEffect( object, effect );
PlaySoundEffectPrivate( object, effect, playfor );
PlaySoundEffectXYZ( x, y, z, effect, realm := _DEFAULT_REALM );
PlayMusic( chr, music_id := 0 );
PlayStationaryEffect( x, y, z, effect, speed, loop := 0, explode := 0, realm := _DEFAULT_REALM );
PlayStationaryEffectEx( x, y, z, realm := _DEFAULT_REALM, effect, speed, duration := 0, hue := 0, render := 0, effect3d := 0 );
PrintTextAbove( above_object, text, font := _DEFAULT_TEXT_FONT, color := _DEFAULT_TEXT_COLOR, journal_print := JOURNAL_PRINT_NAME );
PrintTextAbovePrivate( above_object, text, character, font := _DEFAULT_TEXT_FONT, color := _DEFAULT_TEXT_COLOR, journal_print := JOURNAL_PRINT_NAME );
ReadGameClock();
RegisterForSpeechEvents( at_object, range, flags := 0 );
ReleaseItem( item );
RequestInput( character, item, prompt ); // item is a placeholder, just pass any item
ReserveItem( item );
RestartScript( npc );
Resurrect( mobile, flags := 0 ); // flags: RESURRECT_*
RevokePrivilege( character, privilege );
SaveWorldState();
SecureTradeWin( character, character2 );
SelectColor( character, item );
SelectMenuItem2( character, menuname );
SendBuyWindow( character, container, vendor, items, flags := 0 );
SendCharProfile( character, of_who, title, uneditable_text := CHARPROFILE_NO_UNEDITABLE_TEXT, editable_text := CHARPROFILE_NO_EDITABLE_TEXT );
SendCharacterRaceChanger( character );
SendDialogGump( who, layout, textlines, x := 0, y := 0, flags := 0, gumpid := 0 );
SendEvent( npc, event );
SendHousingTool( who, house );
SendInstaResDialog( character );
SendOpenBook( character, book );
SendOpenSpecialContainer( character, container );
SendOverallSeason( season_id, playsound := 1 );
SendPacket( to_whom, packet_hex_string );
SendPopUpMenu( to_whom, above, menu );
SendQuestArrow( to_whom, x := -1, y := -1, arrowid := 0 ); // no params (-1x,-1y) turns the arrow off, target is required for HSA clients
SendSellWindow( character, vendor, i1, i2, i3, flags := 0 );
SendSkillWindow( towhom, forwhom );
SendStatus( character );
SendStringAsTipWindow( character, text );
SendSysMessage( character, text, font := _DEFAULT_TEXT_FONT, color := _DEFAULT_TEXT_COLOR );
SendTextEntryGump( who, line1, cancel := TE_CANCEL_ENABLE, style := TE_STYLE_NORMAL, maximum := 40, line2 := "" );
SendViewContainer( character, container );
SetGlobalProperty( propname, propval );
SetName( object, name );
SetObjProperty( object, property_name, property_value );
SetRegionLightLevel( regionname, lightlevel );
SetRegionWeatherLevel( region, type, severity, aux := 0, lightoverride := -1);
SetScriptController( who );
SingleClick( who, what );
Shutdown( exit_code := 0 );
SpeakPowerWords( who, spellid, font := _DEFAULT_TEXT_FONT, color := _DEFAULT_TEXT_COLOR );
StartSpellEffect( who, spellid );
SubtractAmount( item, amount );
SystemFindObjectBySerial( serial, sysfind_flags := 0 );
Target( by_character, options := TGTOPT_CHECK_LOS+TGTOPT_NEUTRAL);
TargetCoordinates( by_character );
TargetMultiPlacement( character, objtype, flags := 0, xoffset := 0, yoffset := 0, hue := 0 );
UpdateItem( item );
UpdateMobile( mobile, recreate := UPDATEMOBILE_UPDATE );
UseItem(item, character);
POLCore();
GetMidpointCircleCoords(xcenter, ycenter, radius);
