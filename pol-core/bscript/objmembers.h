/*
History
=======
2005/01/24 Shinigami: added ObjMember character.spyonclient2 to get data from packet 0xd9 (Spy on Client 2)
2005/03/09 Shinigami: added Prop Character::Delay_Mod [ms] for WeaponDelay
2005/04/04 Shinigami: added Prop Character::CreatedAt [PolClock]
2005/08/29 Shinigami: character.spyonclient2 renamed to character.clientinfo
2005/10/02 Shinigami: added Prop Script.attached_to and Script.controller
2005/12/09 MuadDib:   added member uclang for storing Unicode Language sent during logon.
2006/05/16 Shinigami: added Prop Character.Race [RACE_* Constants] to support Elfs
2007/07/09 Shinigami: added Prop Character.isUOKR [bool] - UO:KR client used?
2008/07/08 Turley:    Added character.movemode - returns the MoveMode like given in NPCDesc
                      Added item.stackable - Is item stackable?
2008/12/17 MuadDub:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
2009/08/06 MuadDib:   Removed PasswordOnlyHash support
2009/08/19 Turley:    Added character.uo_expansion_client
2009/09/06 Turley:    Removed chr.isUOKR added chr.ClientType

Notes
=======

*/

#ifndef OBJMEMBERS_H
#define OBJMEMBERS_H

enum MemberID
{
    MBR_X, //uobject        //0
    MBR_Y,                  //1
    MBR_Z,
    MBR_NAME,
    MBR_OBJTYPE,
    MBR_GRAPHIC,        //5
    MBR_SERIAL,
    MBR_COLOR,
    MBR_HEIGHT,
    MBR_FACING,
    MBR_DIRTY,        //10
    MBR_WEIGHT,
    MBR_MULTI,
    MBR_AMOUNT, //item
    MBR_LAYER,
    MBR_CONTAINER,        //15
    MBR_USESCRIPT,
    MBR_EQUIPSCRIPT,
    MBR_UNEQUIPSCRIPT,
    MBR_DESC,
    MBR_MOVABLE,        //20
    MBR_INVISIBLE,
    MBR_DECAYAT,
    MBR_SELLPRICE,
    MBR_BUYPRICE,
    MBR_NEWBIE,        //25
    MBR_ITEM_COUNT,
    MBR_WARMODE, //character
    MBR_GENDER,
    MBR_TRUEOBJTYPE,
    MBR_TRUECOLOR,        //30
    MBR_AR_MOD,
    MBR_HIDDEN,
    MBR_CONCEALED,
    MBR_FROZEN,
    MBR_PARALYZED,        //35
    MBR_POISONED,
    MBR_STEALTHSTEPS,
    MBR_SQUELCHED,
    MBR_DEAD,
    MBR_AR,        //40
    MBR_BACKPACK,
    MBR_WEAPON,
	MBR_SHIELD,
	MBR_ACCTNAME,
    MBR_ACCT,
    MBR_CMDLEVEL,        //45
    MBR_CMDLEVELSTR,
    MBR_CRIMINAL,
    MBR_IP,
    MBR_GOLD,
    MBR_TITLE_PREFIX,        //50
    MBR_TITLE_SUFFIX,
    MBR_TITLE_GUILD,
    MBR_TITLE_RACE,
    MBR_GUILDID,
    MBR_GUILD,        //55
    MBR_MURDERER,
    MBR_ATTACHED,
    MBR_CLIENTVERSION,
    MBR_REPORTABLES,
    MBR_SCRIPT,  //npc        //60
    MBR_NPCTEMPLATE,
    MBR_MASTER,
    MBR_PROCESS,
    MBR_EVENTMASK,
    MBR_SPEECH_COLOR,        //65
    MBR_SPEECH_FONT,
    MBR_USE_ADJUSTMENTS,
    MBR_RUN_SPEED,
    MBR_LOCKED, //lockable
    MBR_CORPSETYPE, //corpse        //70
    MBR_TILLERMAN, //boat
    MBR_PORTPLANK,
    MBR_STARBOARDPLANK,
    MBR_HOLD,
	MBR_HAS_OFFLINE_MOBILES,		//75
    MBR_COMPONENTS, //house
    MBR_ITEMS, //multi
    MBR_MOBILES,
    MBR_XEAST, //map
    MBR_XWEST,        //80
    MBR_YNORTH,
    MBR_YSOUTH,
    MBR_GUMPWIDTH,
    MBR_GUMPHEIGHT,
    MBR_ISOPEN, //door        //85
    MBR_QUALITY, //equipment
    MBR_HP,
    MBR_MAXHP_MOD,
    MBR_MAXHP,
    MBR_DMG_MOD, //weapon        //90
    MBR_ATTRIBUTE,
	MBR_INTRINSIC,
    MBR_HITSCRIPT,
    MBR_AR_BASE,
    MBR_ONHIT_SCRIPT,			//95
    MBR_ENABLED, //account        
    MBR_BANNED,
    MBR_USERNAMEPASSWORDHASH,
    MBR_MEMBERS, //guild/party
    MBR_ALLYGUILDS,        //100
    MBR_ENEMYGUILDS,
    MBR_PID, //script
    MBR_STATE,
    MBR_INSTR_CYCLES,
    MBR_SLEEP_CYCLES,        //105
    MBR_CONSEC_CYCLES,
    MBR_PC,
    MBR_CALL_DEPTH,
    MBR_NUM_GLOBALS,
    MBR_VAR_SIZE,        //110
	MBR_REALM,
	MBR_UO_EXPANSION,
	MBR_CUSTOM,
    MBR_GLOBALS,
	MBR_FOOTPRINT,		//115
    MBR_CLIENTINFO,
    MBR_DELAY_MOD,
    MBR_CREATEDAT,
	MBR_OPPONENT,
	MBR_CONNECTED,		//120
	MBR_ATTACHED_TO,
	MBR_CONTROLLER,
	MBR_OWNERSERIAL,
	MBR_DEFAULTCMDLEVEL,
	MBR_UCLANG,			//125
	MBR_RACE,
	MBR_TRADING_WITH,
	MBR_TRADE_CONTAINER,
	MBR_ALIGNMENT,
	MBR_CURSOR,
	MBR_GUMP,
	MBR_PROMPT,
	MBR_STACKABLE,
	MBR_MOVEMODE,        //135
	MBR_HITCHANCE_MOD,
	MBR_EVASIONCHANCE_MOD,
	MBR_TILE_LAYER,
	MBR_CLIENTVERSIONDETAIL,
	MBR_SAVEONEXIT,		// 140
	MBR_FIRE_RESIST,
	MBR_COLD_RESIST,
	MBR_ENERGY_RESIST,
	MBR_POISON_RESIST,
	MBR_PHYSICAL_RESIST,	// 145
	MBR_FIRE_RESIST_MOD,
	MBR_COLD_RESIST_MOD,
	MBR_ENERGY_RESIST_MOD,
	MBR_POISON_RESIST_MOD,
	MBR_PHYSICAL_RESIST_MOD,	// 150
	MBR_STATCAP,
	MBR_SKILLCAP,
	MBR_LUCK,
	MBR_FOLLOWERSMAX,
	MBR_TITHING,				// 155
	MBR_FOLLOWERS,
	MBR_FIRE_DAMAGE,
	MBR_COLD_DAMAGE,
	MBR_ENERGY_DAMAGE,
	MBR_POISON_DAMAGE,			// 160
	MBR_PHYSICAL_DAMAGE,
	MBR_FIRE_DAMAGE_MOD,
	MBR_COLD_DAMAGE_MOD,
	MBR_ENERGY_DAMAGE_MOD,
	MBR_POISON_DAMAGE_MOD,		// 165
	MBR_PHYSICAL_DAMAGE_MOD,
	MBR_PARTY,
	MBR_LEADER,
	MBR_PARTYLOOT,
	MBR_CANDIDATE_OF_PARTY, //170
	MBR_CANDIDATES,
	MBR_MOVECOST_WALK,
	MBR_MOVECOST_RUN,
	MBR_MOVECOST_WALK_MOUNTED,
	MBR_MOVECOST_RUN_MOUNTED, //175
	MBR_AGGRESSORTO,
	MBR_LAWFULLYDAMAGED,
	MBR_GETGOTTENBY,
	MBR_UO_EXPANSION_CLIENT,
	MBR_CLIENTTYPE
};


typedef struct {
    MemberID id;
    char code[30];
    bool read_only;
} ObjMember;

extern ObjMember object_members[];
extern int n_objmembers;

ObjMember* getKnownObjMember(const char* token);
ObjMember* getObjMember(int id);

#endif

