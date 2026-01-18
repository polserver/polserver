#include "objmembers.h"
#include "objmethods.h"

#include <algorithm>
#include <string>
#include <unordered_map>


namespace Pol::Bscript
{
ObjMember object_members[] = {
    // MBR_*, "name"
    { MBR_X, "x" },  // 0
    { MBR_Y, "y" },  // 1
    { MBR_Z, "z" },
    { MBR_NAME, "name" },
    { MBR_OBJTYPE, "objtype" },
    { MBR_GRAPHIC, "graphic" },  // 5
    { MBR_SERIAL, "serial" },
    { MBR_COLOR, "color" },
    { MBR_HEIGHT, "height" },
    { MBR_FACING, "facing" },
    { MBR_DIRTY, "dirty" },  // 10
    { MBR_WEIGHT, "weight" },
    { MBR_MULTI, "multi" },
    { MBR_AMOUNT, "amount" },  // item
    { MBR_LAYER, "layer" },
    { MBR_CONTAINER, "container" },  // 15
    { MBR_USESCRIPT, "usescript" },
    { MBR_EQUIPSCRIPT, "equipscript" },
    { MBR_UNEQUIPSCRIPT, "unequipscript" },
    { MBR_DESC, "desc" },
    { MBR_MOVABLE, "movable" },  // 20
    { MBR_INVISIBLE, "invisible" },
    { MBR_DECAYAT, "decayat" },
    { MBR_SELLPRICE, "sellprice" },
    { MBR_BUYPRICE, "buyprice" },
    { MBR_NEWBIE, "newbie" },  // 25
    { MBR_ITEM_COUNT, "item_count" },
    { MBR_WARMODE, "warmode" },  // character
    { MBR_GENDER, "gender" },
    { MBR_TRUEOBJTYPE, "trueobjtype" },
    { MBR_TRUECOLOR, "truecolor" },  // 30
    { MBR_AR_MOD, "ar_mod" },
    { MBR_HIDDEN, "hidden" },
    { MBR_CONCEALED, "concealed" },
    { MBR_FROZEN, "frozen" },
    { MBR_PARALYZED, "paralyzed" },  // 35
    { MBR_POISONED, "poisoned" },
    { MBR_STEALTHSTEPS, "stealthsteps" },
    { MBR_SQUELCHED, "squelched" },
    { MBR_DEAD, "dead" },
    { MBR_AR, "ar" },  // 40
    { MBR_BACKPACK, "backpack" },
    { MBR_WEAPON, "weapon" },
    { MBR_SHIELD, "shield" },
    { MBR_ACCTNAME, "acctname" },
    { MBR_ACCT, "acct" },  // 45
    { MBR_CMDLEVEL, "cmdlevel" },
    { MBR_CMDLEVELSTR, "cmdlevelstr" },
    { MBR_CRIMINAL, "criminal" },
    { MBR_IP, "ip" },
    { MBR_GOLD, "gold" },  // 50
    { MBR_TITLE_PREFIX, "title_prefix" },
    { MBR_TITLE_SUFFIX, "title_suffix" },
    { MBR_TITLE_GUILD, "title_guild" },
    { MBR_TITLE_RACE, "title_race" },
    { MBR_GUILDID, "guildid" },  // 55
    { MBR_GUILD, "guild" },
    { MBR_MURDERER, "murderer" },
    { MBR_ATTACHED, "attached" },
    { MBR_CLIENTVERSION, "clientversion" },
    { MBR_REPORTABLES, "reportables" },  // 60
    { MBR_SCRIPT, "script" },            // npc
    { MBR_NPCTEMPLATE, "npctemplate" },
    { MBR_MASTER, "master" },
    { MBR_PROCESS, "process" },
    { MBR_EVENTMASK, "eventmask" },  // 65
    { MBR_SPEECH_COLOR, "speech_color" },
    { MBR_SPEECH_FONT, "speech_font" },
    { MBR_USE_ADJUSTMENTS, "use_adjustments" },
    { MBR_RUN_SPEED, "run_speed" },
    { MBR_LOCKED, "locked" },          // lockable //70
    { MBR_CORPSETYPE, "corpsetype" },  // corpse
    { MBR_TILLERMAN, "tillerman" },    // boat
    { MBR_PORTPLANK, "portplank" },
    { MBR_STARBOARDPLANK, "starboardplank" },
    { MBR_HOLD, "hold" },  // 75
    { MBR_HAS_OFFLINE_MOBILES, "has_offline_mobiles" },
    { MBR_COMPONENTS, "components" },  // house
    { MBR_ITEMS, "items" },            // multi
    { MBR_MOBILES, "mobiles" },
    { MBR_XEAST, "xeast" },  // map //80
    { MBR_XWEST, "xwest" },
    { MBR_YNORTH, "ynorth" },
    { MBR_YSOUTH, "ysouth" },
    { MBR_GUMPWIDTH, "gumpwidth" },
    { MBR_GUMPHEIGHT, "gumpheight" },  // 85
    { MBR_ISOPEN, "isopen" },          // door
    { MBR_QUALITY, "quality" },        // equipment
    { MBR_HP, "hp" },
    { MBR_MAXHP_MOD, "maxhp_mod" },
    { MBR_MAXHP, "maxhp" },      // 90
    { MBR_DMG_MOD, "dmg_mod" },  // weapon
    { MBR_ATTRIBUTE, "attribute" },
    { MBR_INTRINSIC, "intrinsic" },
    { MBR_HITSCRIPT, "hitscript" },
    { MBR_AR_BASE, "ar_base" },  // 95
    { MBR_ONHIT_SCRIPT, "onhitscript" },
    { MBR_ENABLED, "enabled" },  // account
    { MBR_BANNED, "banned" },
    { MBR_USERNAMEPASSWORDHASH, "usernamepasswordhash" },
    { MBR_MEMBERS, "members" },  // guild //100
    { MBR_ALLYGUILDS, "allyguilds" },
    { MBR_ENEMYGUILDS, "enemyguilds" },
    { MBR_PID, "pid" },  // script
    { MBR_STATE, "state" },
    { MBR_INSTR_CYCLES, "instr_cycles" },  // 105
    { MBR_SLEEP_CYCLES, "sleep_cycles" },
    { MBR_CONSEC_CYCLES, "consec_cycles" },
    { MBR_PC, "pc" },
    { MBR_CALL_DEPTH, "call_depth" },
    { MBR_NUM_GLOBALS, "num_globals" },  // 110
    { MBR_VAR_SIZE, "var_size" },
    { MBR_REALM, "realm" },
    { MBR_UO_EXPANSION, "uo_expansion" },
    { MBR_CUSTOM, "custom" },    // house
    { MBR_GLOBALS, "globals" },  // 115
    { MBR_FOOTPRINT, "footprint" },
    { MBR_CLIENTINFO, "clientinfo" },
    { MBR_DELAY_MOD, "delay_mod" },
    { MBR_CREATEDAT, "createdat" },
    { MBR_OPPONENT, "opponent" },  // 120
    { MBR_CONNECTED, "connected" },
    { MBR_ATTACHED_TO, "attached_to" },
    { MBR_CONTROLLER, "controller" },
    { MBR_OWNERSERIAL, "ownerserial" },
    { MBR_DEFAULTCMDLEVEL, "defaultcmdlevel" },  // 125
    { MBR_UCLANG, "uclang" },
    { MBR_RACE, "race" },
    { MBR_TRADING_WITH, "trading_with" },
    { MBR_TRADE_CONTAINER, "trade_container" },
    { MBR_ALIGNMENT, "alignment" },  // 130
    { MBR_CURSOR, "cursor" },
    { MBR_GUMP, "gump" },
    { MBR_PROMPT, "prompt" },
    { MBR_STACKABLE, "stackable" },
    { MBR_MOVEMODE, "movemode" },  // 135
    { MBR_HITCHANCE_MOD, "hitchance_mod" },
    { MBR_EVASIONCHANCE_MOD, "evasionchance_mod" },
    { MBR_TILE_LAYER, "tile_layer" },
    { MBR_CLIENTVERSIONDETAIL, "clientver_detail" },
    { MBR_SAVEONEXIT, "saveonexit" },  // 140
    { MBR_FIRE_RESIST, "resist_fire" },
    { MBR_COLD_RESIST, "resist_cold" },
    { MBR_ENERGY_RESIST, "resist_energy" },
    { MBR_POISON_RESIST, "resist_poison" },
    { MBR_PHYSICAL_RESIST, "resist_physical" },  // 145
    { MBR_FIRE_RESIST_MOD, "resist_fire_mod" },
    { MBR_COLD_RESIST_MOD, "resist_cold_mod" },
    { MBR_ENERGY_RESIST_MOD, "resist_energy_mod" },
    { MBR_POISON_RESIST_MOD, "resist_poison_mod" },
    { MBR_PHYSICAL_RESIST_MOD, "resist_physical_mod" },  // 150
    { MBR_STATCAP, "statcap" },
    { MBR_SKILLCAP, "skillcap" },
    { MBR_LUCK, "luck" },
    { MBR_FOLLOWERSMAX, "followers_max" },
    { MBR_TITHING, "tithing" },  // 155
    { MBR_FOLLOWERS, "followers" },
    { MBR_FIRE_DAMAGE, "damage_fire" },
    { MBR_COLD_DAMAGE, "damage_cold" },
    { MBR_ENERGY_DAMAGE, "damage_energy" },
    { MBR_POISON_DAMAGE, "damage_poison" },  // 160
    { MBR_PHYSICAL_DAMAGE, "damage_physical" },
    { MBR_FIRE_DAMAGE_MOD, "damage_fire_mod" },
    { MBR_COLD_DAMAGE_MOD, "damage_cold_mod" },
    { MBR_ENERGY_DAMAGE_MOD, "damage_energy_mod" },
    { MBR_POISON_DAMAGE_MOD, "damage_poison_mod" },  // 165
    { MBR_PHYSICAL_DAMAGE_MOD, "damage_physical_mod" },
    { MBR_PARTY, "party" },
    { MBR_LEADER, "leader" },
    { MBR_PARTYLOOT, "partycanloot" },
    { MBR_CANDIDATE_OF_PARTY, "candidate_of_party" },  // 170
    { MBR_CANDIDATES, "candidates" },
    { MBR_MOVECOST_WALK, "movecost_walk_mod" },
    { MBR_MOVECOST_RUN, "movecost_run_mod" },
    { MBR_MOVECOST_WALK_MOUNTED, "movecost_walk_mounted_mod" },
    { MBR_MOVECOST_RUN_MOUNTED, "movecost_run_mounted_mod" },  // 175
    { MBR_AGGRESSORTO, "aggressorto" },
    { MBR_LAWFULLYDAMAGED, "lawfullydamaged" },
    { MBR_GETGOTTENBY, "getgottenby" },
    { MBR_UO_EXPANSION_CLIENT, "uo_expansion_client" },
    { MBR_CLIENTTYPE, "clienttype" },  // 180
    { MBR_DEAFENED, "deafed" },
    { MBR_CLIENT, "client" },
    { MBR_TYPE, "type" },
    { MBR_ATTRIBUTES, "attributes" },
    { MBR_EDITING, "house_editing" },  // 185
    { MBR_HOUSEPARTS, "house_parts" },
    { MBR_DOUBLECLICKRANGE, "doubleclickrange" },
    { MBR_MOUNTEDSTEPS, "mountedsteps" },
    // New boat stuff start
    { MBR_ROPE, "rope" },
    { MBR_WHEEL, "wheel" },  // 190
    { MBR_HULL, "hull" },
    { MBR_TILLER, "tiller" },
    { MBR_RUDDER, "rudder" },
    { MBR_SAILS, "sails" },
    { MBR_STORAGE, "storage" },  // 195
    { MBR_WEAPONSLOT, "weaponslot" },
    // New boat stuff end
    { MBR_MULTIID, "multiid" },
    { MBR_TRADEWINDOW, "tradewindow" },
    { MBR_LASTCOORD, "lastcoord" },
    { MBR_FACETID, "facetid" },  // 200
    { MBR_EDITABLE, "editable" },
    { MBR_ACTIVE_SKILL, "active_skill" },
    { MBR_CASTING_SPELL, "casting_spell" },
    { MBR_CARRYINGCAPACITY_MOD, "carrying_capacity_mod" },
    { MBR_MAX_ITEMS_MOD, "max_items_mod" },  // 205
    { MBR_MAX_WEIGHT_MOD, "max_weight_mod" },
    { MBR_MAX_SLOTS_MOD, "max_slots_mod" },
    { MBR_SPEED_MOD, "speed_mod" },
    { MBR_NAME_SUFFIX, "name_suffix" },
    { MBR_TEMPORALLY_CRIMINAL, "temporally_criminal" },  // 210
    { MBR_LAST_TEXTCOLOR, "last_textcolor" },
    { MBR_INSURED, "insured" },
    { MBR_LAST_ACTIVITY_AT, "last_activity_at" },
    { MBR_LAST_PACKET_AT, "last_packet_at" },
    { MBR_HOUSE, "house" },  // 215, Item
    { MBR_SPECIFIC_NAME, "specific_name" },
    { MBR_CARRYINGCAPACITY, "carrying_capacity" },
    { MBR_NO_DROP, "no_drop" },
    { MBR_NO_DROP_EXCEPTION, "no_drop_exception" },
    { MBR_PORT, "port" },  // 220
    // Additions for new properties
    { MBR_LOWER_REAG_COST, "lower_reagent_cost" },
    { MBR_SPELL_DAMAGE_INCREASE, "spell_damage_increase" },
    { MBR_FASTER_CASTING, "faster_casting" },
    { MBR_FASTER_CAST_RECOVERY, "faster_cast_recovery" },
    { MBR_DEFENCE_CHANCE_INCREASE, "defence_increase" },  // 225
    { MBR_DEFENCE_CHANCE_INCREASE_CAP, "defence_increase_cap" },
    { MBR_LOWER_MANA_COST, "lower_mana_cost" },
    { MBR_FIRE_RESIST_CAP, "resist_fire_cap" },
    { MBR_COLD_RESIST_CAP, "resist_cold_cap" },
    { MBR_ENERGY_RESIST_CAP, "resist_energy_cap" },  // 230
    { MBR_POISON_RESIST_CAP, "resist_poison_cap" },
    { MBR_PHYSICAL_RESIST_CAP, "resist_physical_cap" },
    { MBR_HIT_CHANCE, "hit_chance" },
    // Additions for new properties mods
    { MBR_LOWER_REAG_COST_MOD, "lower_reagent_cost_mod" },
    { MBR_SPELL_DAMAGE_INCREASE_MOD, "spell_damage_increase_mod" },
    { MBR_FASTER_CASTING_MOD, "faster_casting_mod" },  // 235
    { MBR_FASTER_CAST_RECOVERY_MOD, "faster_cast_recovery_mod" },
    { MBR_DEFENCE_CHANCE_INCREASE_MOD, "defence_increase_mod" },
    { MBR_DEFENCE_CHANCE_INCREASE_CAP_MOD, "defence_increase_cap_mod" },
    { MBR_LOWER_MANA_COST_MOD, "lower_mana_cost_mod" },
    { MBR_FIRE_RESIST_CAP_MOD, "resist_fire_cap_mod" },  // 240
    { MBR_COLD_RESIST_CAP_MOD, "resist_cold_cap_mod" },
    { MBR_ENERGY_RESIST_CAP_MOD, "resist_energy_cap_mod" },
    { MBR_POISON_RESIST_CAP_MOD, "resist_poison_cap_mod" },
    { MBR_PHYSICAL_RESIST_CAP_MOD, "resist_physical_cap_mod" },
    { MBR_LUCK_MOD, "luck_mod" },  // 245
    { MBR_HIT_CHANCE_MOD, "hit_chance_mod" },
    { MBR_PACKAGE, "package" },
    { MBR_SWING_SPEED_INCREASE, "swing_speed_increase" },
    { MBR_SWING_SPEED_INCREASE_MOD, "swing_speed_increase_mod" },
    { MBR_EXPORTED_FUNCTIONS, "exported_functions" },  // 250
    { MBR_DISABLE_INACTIVITY_TIMEOUT, "disable_inactivity_timeout" },
    { MBR_CURSED, "cursed" },
    { MBR_SNOOPSCRIPT, "snoopscript" },
    { MBR_CHARACTER_OWNER, "character_owner" },
    { MBR_PARRYCHANCE_MOD, "parrychance_mod" },  // 255
    { MBR_PILOT, "pilot" },
    { MBR_BUFFS, "buffs" },
    { MBR_WEIGHT_MULTIPLIER_MOD, "weight_multiplier_mod" },
    { MBR_HELD_WEIGHT_MULTIPLIER, "held_weight_multiplier" },
    { MBR_FUNCTION, "function" },
    { MBR_POSITION_CHANGED_AT, "position_changed_at" },
    { MBR_MOVED_AT, "moved_at" },
    { MBR_VISUAL_RANGE, "visual_range" },
    { MBR_MIN_ATTACK_RANGE_INCREASE, "min_attack_range_increase" },
    { MBR_MIN_ATTACK_RANGE_INCREASE_MOD, "min_attack_range_increase_mod" },  // 265
    { MBR_MAX_ATTACK_RANGE_INCREASE, "max_attack_range_increase" },
    { MBR_MAX_ATTACK_RANGE_INCREASE_MOD, "max_attack_range_increase_mod" },
    { MBR_ITEMS_DECAY, "items_decay" },
    { MBR_LOGGED_IN, "logged_in" },
};
int n_objmembers = sizeof object_members / sizeof object_members[0];
ObjMember* getKnownObjMember( const char* token )
{
  static auto cache = []() -> std::unordered_map<std::string, ObjMember*>
  {
    std::unordered_map<std::string, ObjMember*> m;
    for ( int i = 0; i < n_objmembers; ++i )
    {
      m[object_members[i].code] = &object_members[i];
    }
    return m;
  }();
  std::string temp( token );
  std::transform( temp.begin(), temp.end(), temp.begin(),
                  []( char c ) { return static_cast<char>( ::tolower( c ) ); } );
  auto member = cache.find( temp );
  if ( member != cache.end() )
    return member->second;
  return nullptr;
}
ObjMember* getObjMember( int id )
{
  if ( id >= n_objmembers )
    return nullptr;
  return &( object_members[id] );
}

ObjMethod object_methods[] = {
    { MTH_ISA, "isa", false },                // 0
    { MTH_SET_MEMBER, "set_member", false },  // 1
    { MTH_GET_MEMBER, "get_member", false },
    { MTH_SETPOISONED, "setpoisoned", false },
    { MTH_SETPARALYZED, "setparalyzed", false },
    { MTH_SETCRIMINAL, "setcriminal", false },  // 5
    { MTH_SETLIGHTLEVEL, "setlightlevel", false },
    { MTH_SQUELCH, "squelch", false },
    { MTH_ENABLE, "enable", false },
    { MTH_DISABLE, "disable", false },
    { MTH_ENABLED, "enabled", false },  // 10
    { MTH_SETCMDLEVEL, "setcmdlevel", false },
    { MTH_SPENDGOLD, "spendgold", false },
    { MTH_SETMURDERER, "setmurderer", false },
    { MTH_REMOVEREPORTABLE, "removereportable", false },
    { MTH_GETGOTTENITEM, "getgottenitem", false },  // 15
    { MTH_CLEARGOTTENITEM, "cleargottenitem", false },
    { MTH_SETWARMODE, "setwarmode", false },
    { MTH_SETMASTER, "setmaster", false },                        // npc
    { MTH_MOVE_OFFLINE_MOBILES, "move_offline_mobiles", false },  // boat
    { MTH_SETCUSTOM, "setcustom", false },                        // house       //20
    { MTH_GETPINS, "getpins", false },                            // map
    { MTH_INSERTPIN, "insertpin", false },
    { MTH_APPENDPIN, "appendpin", false },
    { MTH_ERASEPIN, "erasepin", false },
    { MTH_OPEN, "open", false },  // door             //25
    { MTH_CLOSE, "close", false },
    { MTH_TOGGLE, "toggle", false },
    { MTH_BAN, "ban", false },  // account
    { MTH_UNBAN, "unban", false },
    { MTH_SETPASSWORD, "setpassword", false },  // 30
    { MTH_CHECKPASSWORD, "checkpassword", false },
    { MTH_SETNAME, "setname", false },
    { MTH_GETCHARACTER, "getcharacter", false },
    { MTH_DELETECHARACTER, "deletecharacter", false },
    { MTH_GETPROP, "getprop", false },  // 35
    { MTH_SETPROP, "setprop", false },
    { MTH_ERASEPROP, "eraseprop", false },
    { MTH_PROPNAMES, "propnames", false },
    { MTH_ISMEMBER, "ismember", false },        // guild
    { MTH_ISALLYGUILD, "isallyguild", false },  // 40
    { MTH_ISENEMYGUILD, "isenemyguild", false },
    { MTH_ADDMEMBER, "addmember", false },
    { MTH_ADDALLYGUILD, "addallyguild", false },
    { MTH_ADDENEMYGUILD, "addenemyguild", false },
    { MTH_REMOVEMEMBER, "removemember", false },  // 45
    { MTH_REMOVEALLYGUILD, "removeallyguild", false },
    { MTH_REMOVEENEMYGUILD, "removeenemyguild", false },
    { MTH_SIZE, "size", false },  // ARRAY
    { MTH_ERASE, "erase", false },
    { MTH_INSERT, "insert", false },  // 50
    { MTH_SHRINK, "shrink", false },
    { MTH_APPEND, "append", false },
    { MTH_REVERSE, "reverse", false },
    { MTH_SORT, "sort", false },      // dict
    { MTH_EXISTS, "exists", false },  // 55
    { MTH_KEYS, "keys", false },
    { MTH_SENDPACKET, "sendpacket", false },  // packet
    { MTH_SENDAREAPACKET, "sendareapacket", false },
    { MTH_GETINT8, "getint8", false },
    { MTH_GETINT16, "getint16", false },  // 60
    { MTH_GETINT32, "getint32", false },
    { MTH_SETINT8, "setint8", false },
    { MTH_SETINT16, "setint16", false },
    { MTH_SETINT32, "setint32", false },
    { MTH_GETSTRING, "getstring", false },  // 65
    { MTH_GETUNICODESTRING, "getunicodestring", false },
    { MTH_SETSTRING, "setstring", false },
    { MTH_SETUNICODESTRING, "setunicodestring", false },
    { MTH_GETSIZE, "getsize", false },
    { MTH_SETSIZE, "setsize", false },              // 70
    { MTH_CREATEELEMENT, "createelement", false },  // datastore
    { MTH_FINDELEMENT, "findelement", false },
    { MTH_DELETEELEMENT, "deleteelement", false },
    { MTH_SENDEVENT, "sendevent", false },  // script
    { MTH_KILL, "kill", false },            // 75
    { MTH_LOADSYMBOLS, "loadsymbols", false },
    { MTH_SET_UO_EXPANSION, "set_uo_expansion", false },
    { MTH_CLEAR_EVENT_QUEUE, "clear_event_queue", false },
    { MTH_ADD_COMPONENT, "add_component", false },
    { MTH_ERASE_COMPONENT, "erase_component", false },  // 80
    { MTH_DELETE, "delete", false },
    { MTH_SPLIT, "split", false },
    { MTH_MOVE_CHAR, "move_char", false },
    { MTH_GETINT16FLIPPED, "getint16flipped", false },
    { MTH_GETINT32FLIPPED, "getint32flipped", false },  // 85
    { MTH_SETINT16FLIPPED, "setint16flipped", false },
    { MTH_SETINT32FLIPPED, "setint32flipped", false },
    { MTH_GETCORPSE, "getcorpse", false },
    { MTH_SETDEFAULTCMDLEVEL, "setdefaultcmdlevel", false },
    { MTH_PRIVILEGES, "privileges", false },  // 90
    { MTH_GETUNICODESTRINGFLIPPED, "getunicodestringflipped", false },
    { MTH_SETUNICODESTRINGFLIPPED, "setunicodestringflipped", false },
    { MTH_ADD_CHARACTER, "addcharacter", false },
    { MTH_SET_SWINGTIMER, "setswingtimer", false },
    { MTH_ATTACK_ONCE, "attack_once", false },  // 95
    { MTH_SETFACING, "setfacing", false },
    { MTH_COMPAREVERSION, "compareversion", false },
    { MTH_SETLEADER, "setleader", false },
    { MTH_ADDCANDIDATE, "addcandidate", false },
    { MTH_REMOVECANDIDATE, "removecandidate", false },  // 100
    { MTH_RANDOMENTRY, "randomentry", false },
    { MTH_SEEK, "seek", false },
    { MTH_PEEK, "peek", false },
    { MTH_TELL, "tell", false },
    { MTH_FLUSH, "flush", false },  // 105
    { MTH_GETSINT8, "getsint8", false },
    { MTH_GETSINT16, "getsint16", false },
    { MTH_GETSINT32, "getsint32", false },
    { MTH_SETSINT8, "setsint8", false },
    { MTH_SETSINT16, "setsint16", false },  // 110
    { MTH_SETSINT32, "setsint32", false },
    { MTH_SETAGGRESSORTO, "setaggressorto", false },
    { MTH_SETLAWFULLYDAMAGEDTO, "setlawfullydamagedto", false },
    { MTH_CLEARAGGRESSORTO, "clearaggressorto", false },
    { MTH_CLEARLAWFULLYDAMAGEDTO, "clearlawfullydamagedto", false },  // 115
    { MTH_HASSPELL, "hasspell", false },
    { MTH_SPELLS, "spells", false },
    { MTH_REMOVESPELL, "removespell", false },
    { MTH_ADDSPELL, "addspell", false },
    { MTH_DEAF, "deaf", false },  // 120
    { MTH_SETSEASON, "setseason", false },
    { MTH_NEXTSIBLING, "nextxmlsibling", false },
    { MTH_FIRSTCHILD, "firstxmlchild", false },
    { MTH_SAVEXML, "savexml", false },
    { MTH_APPENDNODE, "appendxmlnode", false },  // 125
    { MTH_SETDECLARATION, "setxmldeclaration", false },
    { MTH_SETATTRIBUTE, "setxmlattribute", false },
    { MTH_REMOVEATTRIBUTE, "removexmlattribute", false },
    { MTH_REMOVENODE, "removexmlnode", false },
    { MTH_APPENDTEXT, "appendxmltext", false },  // 130
    { MTH_XMLTOSTRING, "xmltostring", false },
    { MTH_APPENDXMLCOMMENT, "appendxmlcomment", false },
    { MTH_ADD_HOUSE_PART, "addhousepart", false },
    { MTH_ERASE_HOUSE_PART, "erasehousepart", false },
    { MTH_ACCEPT_COMMIT, "acceptcommit", false },  // 135
    { MTH_SPLITSTACK_AT, "splitstackat", false },
    { MTH_SPLITSTACK_INTO, "splitstackinto", false },
    { MTH_CANCEL_EDITING, "cancelediting", false },
    { MTH_CLONENODE, "clonenode", false },
    { MTH_HAS_EXISTING_STACK, "hasexistingstack", false },  // 140
    { MTH_LENGTH, "length", false },
    { MTH_JOIN, "join", false },
    { MTH_FIND, "find", false },
    { MTH_UPPER, "upper", false },
    { MTH_LOWER, "lower", false },  // 145
    { MTH_FORMAT, "format", false },
    { MTH_DISABLE_SKILLS_FOR, "disableskillsfor", false },
    { MTH_CYCLE, "cycle", false },
    { MTH_ADD_BUFF, "addbuff", false },
    { MTH_DEL_BUFF, "delbuff", false },  // 150
    { MTH_CLEAR_BUFFS, "clearbuffs", false },
    { MTH_CALL, "call", false },
    { MTH_SORTEDINSERT, "sorted_insert", false },
    { MTH_SETUTF8STRING, "setutf8string", false },
    { MTH_SET_PILOT, "set_pilot", false },  // 155
    { MTH_SET_ALTERNATE_MULTIID, "set_alternate_multiid", false },
    { MTH_FILTER, "filter", false },
    { MTH_REDUCE, "reduce", false },
    { MTH_MAP, "map", false },
    { MTH_FINDINDEX, "findindex", false },  // 160
    { MTH_SET_MULTIID, "set_multiid", false },
    { MTH_NEW, "new", false },
    { MTH_CALL_METHOD, "call_method", false },  // internal, does not need objref.xml documentation
    { MTH_STACKTRACE, "stacktrace", false },
    { MTH_ASSIGN, "assign", false },  // 165
    { MTH_MATCH, "match", false },
    { MTH_REPLACE, "replace", false },
};
int n_objmethods = sizeof object_methods / sizeof object_methods[0];
ObjMethod* getKnownObjMethod( const char* token )
{
  // cache needs to hold a pointer to the original structure! eprog_read sets the override member
  static auto cache = []() -> std::unordered_map<std::string, ObjMethod*>
  {
    std::unordered_map<std::string, ObjMethod*> m;
    for ( int i = 0; i < n_objmethods; ++i )
    {
      m[object_methods[i].code] = &object_methods[i];
    }
    return m;
  }();
  std::string temp( token );
  std::transform( temp.begin(), temp.end(), temp.begin(),
                  []( char c ) { return static_cast<char>( ::tolower( c ) ); } );
  auto method = cache.find( temp );
  if ( method != cache.end() )
    return method->second;
  return nullptr;
}
ObjMethod* getObjMethod( int id )
{
  if ( id >= n_objmethods )
    return nullptr;
  return &( object_methods[id] );
}

}  // namespace Pol::Bscript
