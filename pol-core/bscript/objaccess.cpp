#include "objmembers.h"
#include "objmethods.h"

#include <algorithm>
#include <string>
#include <unordered_map>

namespace Pol
{
namespace Bscript
{
ObjMember object_members[] = {
    // MBR_*, "name", read_only, hidden
    { MBR_X, "x", true, false },  // 0
    { MBR_Y, "y", true, false },  // 1
    { MBR_Z, "z", true, false },
    { MBR_NAME, "name", false, false },
    { MBR_OBJTYPE, "objtype", true, false },
    { MBR_GRAPHIC, "graphic", false, false },  // 5
    { MBR_SERIAL, "serial", true, false },
    { MBR_COLOR, "color", false, false },
    { MBR_HEIGHT, "height", true, false },
    { MBR_FACING, "facing", false, false },
    { MBR_DIRTY, "dirty", true, false },  // 10
    { MBR_WEIGHT, "weight", true, false },
    { MBR_MULTI, "multi", true, false },
    { MBR_AMOUNT, "amount", true, false },  // item
    { MBR_LAYER, "layer", true, false },
    { MBR_CONTAINER, "container", true, false },  // 15
    { MBR_USESCRIPT, "usescript", false, false },
    { MBR_EQUIPSCRIPT, "equipscript", false, false },
    { MBR_UNEQUIPSCRIPT, "unequipscript", false, false },
    { MBR_DESC, "desc", false, false },
    { MBR_MOVABLE, "movable", false, false },  // 20
    { MBR_INVISIBLE, "invisible", false, false },
    { MBR_DECAYAT, "decayat", false, false },
    { MBR_SELLPRICE, "sellprice", false, false },
    { MBR_BUYPRICE, "buyprice", false, false },
    { MBR_NEWBIE, "newbie", false, false },  // 25
    { MBR_ITEM_COUNT, "item_count", true, false },
    { MBR_WARMODE, "warmode", true, false },  // character
    { MBR_GENDER, "gender", false, false },
    { MBR_TRUEOBJTYPE, "trueobjtype", false, false },
    { MBR_TRUECOLOR, "truecolor", false, false },  // 30
    { MBR_AR_MOD, "ar_mod", false, false },
    { MBR_HIDDEN, "hidden", false, false },
    { MBR_CONCEALED, "concealed", false, false },
    { MBR_FROZEN, "frozen", false, false },
    { MBR_PARALYZED, "paralyzed", false, false },  // 35
    { MBR_POISONED, "poisoned", false, false },
    { MBR_STEALTHSTEPS, "stealthsteps", false, false },
    { MBR_SQUELCHED, "squelched", true, false },
    { MBR_DEAD, "dead", true, false },
    { MBR_AR, "ar", true, false },  // 40
    { MBR_BACKPACK, "backpack", true, false },
    { MBR_WEAPON, "weapon", true, false },
    { MBR_SHIELD, "shield", true, false },
    { MBR_ACCTNAME, "acctname", true, false },
    { MBR_ACCT, "acct", true, false },  // 45
    { MBR_CMDLEVEL, "cmdlevel", false, false },
    { MBR_CMDLEVELSTR, "cmdlevelstr", true, false },
    { MBR_CRIMINAL, "criminal", true, false },
    { MBR_IP, "ip", true, false },
    { MBR_GOLD, "gold", true, false },  // 50
    { MBR_TITLE_PREFIX, "title_prefix", false, false },
    { MBR_TITLE_SUFFIX, "title_suffix", false, false },
    { MBR_TITLE_GUILD, "title_guild", false, false },
    { MBR_TITLE_RACE, "title_race", false, false },
    { MBR_GUILDID, "guildid", true, false },  // 55
    { MBR_GUILD, "guild", true, false },
    { MBR_MURDERER, "murderer", false, false },
    { MBR_ATTACHED, "attached", true, false },
    { MBR_CLIENTVERSION, "clientversion", true, false },
    { MBR_REPORTABLES, "reportables", true, false },  // 60
    { MBR_SCRIPT, "script", false, false },           // npc
    { MBR_NPCTEMPLATE, "npctemplate", true, false },
    { MBR_MASTER, "master", true, false },
    { MBR_PROCESS, "process", true, false },
    { MBR_EVENTMASK, "eventmask", true, false },  // 65
    { MBR_SPEECH_COLOR, "speech_color", false, false },
    { MBR_SPEECH_FONT, "speech_font", false, false },
    { MBR_USE_ADJUSTMENTS, "use_adjustments", false, false },
    { MBR_RUN_SPEED, "run_speed", false, false },
    { MBR_LOCKED, "locked", false, false },         // lockable //70
    { MBR_CORPSETYPE, "corpsetype", true, false },  // corpse
    { MBR_TILLERMAN, "tillerman", true, false },    // boat
    { MBR_PORTPLANK, "portplank", true, false },
    { MBR_STARBOARDPLANK, "starboardplank", true, false },
    { MBR_HOLD, "hold", true, false },  // 75
    { MBR_HAS_OFFLINE_MOBILES, "has_offline_mobiles", true, false },
    { MBR_COMPONENTS, "components", true, false },  // house
    { MBR_ITEMS, "items", true, false },            // multi
    { MBR_MOBILES, "mobiles", true, false },
    { MBR_XEAST, "xeast", false, false },  // map //80
    { MBR_XWEST, "xwest", false, false },
    { MBR_YNORTH, "ynorth", false, false },
    { MBR_YSOUTH, "ysouth", false, false },
    { MBR_GUMPWIDTH, "gumpwidth", false, false },
    { MBR_GUMPHEIGHT, "gumpheight", false, false },  // 85
    { MBR_ISOPEN, "isopen", true, false },           // door
    { MBR_QUALITY, "quality", false, false },        // equipment
    { MBR_HP, "hp", false, false },
    { MBR_MAXHP_MOD, "maxhp_mod", false, false },
    { MBR_MAXHP, "maxhp", true, false },       // 90
    { MBR_DMG_MOD, "dmg_mod", false, false },  // weapon
    { MBR_ATTRIBUTE, "attribute", true, false },
    { MBR_INTRINSIC, "intrinsic", true, false },
    { MBR_HITSCRIPT, "hitscript", false, false },
    { MBR_AR_BASE, "ar_base", true, false },  // 95
    { MBR_ONHIT_SCRIPT, "onhitscript", false, false },
    { MBR_ENABLED, "enabled", true, false },  // account
    { MBR_BANNED, "banned", true, false },
    { MBR_USERNAMEPASSWORDHASH, "usernamepasswordhash", true, false },
    { MBR_MEMBERS, "members", true, false },  // guild //100
    { MBR_ALLYGUILDS, "allyguilds", true, false },
    { MBR_ENEMYGUILDS, "enemyguilds", true, false },
    { MBR_PID, "pid", true, false },  // script
    { MBR_STATE, "state", true, false },
    { MBR_INSTR_CYCLES, "instr_cycles", true, false },  // 105
    { MBR_SLEEP_CYCLES, "sleep_cycles", true, false },
    { MBR_CONSEC_CYCLES, "consec_cycles", true, false },
    { MBR_PC, "pc", true, false },
    { MBR_CALL_DEPTH, "call_depth", true, false },
    { MBR_NUM_GLOBALS, "num_globals", true, false },  // 110
    { MBR_VAR_SIZE, "var_size", true, false },
    { MBR_REALM, "realm", true, false },
    { MBR_UO_EXPANSION, "uo_expansion", true, false },
    { MBR_CUSTOM, "custom", true, false },    // house
    { MBR_GLOBALS, "globals", true, false },  // 115
    { MBR_FOOTPRINT, "footprint", true, false },
    { MBR_CLIENTINFO, "clientinfo", true, false },
    { MBR_DELAY_MOD, "delay_mod", false, false },
    { MBR_CREATEDAT, "createdat", true, false },
    { MBR_OPPONENT, "opponent", true, false },  // 120
    { MBR_CONNECTED, "connected", true, false },
    { MBR_ATTACHED_TO, "attached_to", true, false },
    { MBR_CONTROLLER, "controller", true, false },
    { MBR_OWNERSERIAL, "ownerserial", true, false },
    { MBR_DEFAULTCMDLEVEL, "defaultcmdlevel", true, false },  // 125
    { MBR_UCLANG, "uclang", true, false },
    { MBR_RACE, "race", false, false },
    { MBR_TRADING_WITH, "trading_with", false, false },
    { MBR_TRADE_CONTAINER, "trade_container", false, false },
    { MBR_ALIGNMENT, "alignment", false, false },  // 130
    { MBR_CURSOR, "cursor", false, false },
    { MBR_GUMP, "gump", false, false },
    { MBR_PROMPT, "prompt", false, false },
    { MBR_STACKABLE, "stackable", false, false },
    { MBR_MOVEMODE, "movemode", false, false },  // 135
    { MBR_HITCHANCE_MOD, "hitchance_mod", false, false },
    { MBR_EVASIONCHANCE_MOD, "evasionchance_mod", false, false },
    { MBR_TILE_LAYER, "tile_layer", true, false },
    { MBR_CLIENTVERSIONDETAIL, "clientver_detail", true, false },
    { MBR_SAVEONEXIT, "saveonexit", true, false },  // 140
    { MBR_FIRE_RESIST, "resist_fire", true, false },
    { MBR_COLD_RESIST, "resist_cold", true, false },
    { MBR_ENERGY_RESIST, "resist_energy", true, false },
    { MBR_POISON_RESIST, "resist_poison", true, false },
    { MBR_PHYSICAL_RESIST, "resist_physical", true, false },  // 145
    { MBR_FIRE_RESIST_MOD, "resist_fire_mod", true, false },
    { MBR_COLD_RESIST_MOD, "resist_cold_mod", true, false },
    { MBR_ENERGY_RESIST_MOD, "resist_energy_mod", true, false },
    { MBR_POISON_RESIST_MOD, "resist_poison_mod", true, false },
    { MBR_PHYSICAL_RESIST_MOD, "resist_physical_mod", true, false },  // 150
    { MBR_STATCAP, "statcap", false, false },
    { MBR_SKILLCAP, "skillcap", false, false },
    { MBR_LUCK, "luck", false, false },
    { MBR_FOLLOWERSMAX, "followers_max", false, false },
    { MBR_TITHING, "tithing", false, false },  // 155
    { MBR_FOLLOWERS, "followers", false, false },
    { MBR_FIRE_DAMAGE, "damage_fire", true, false },
    { MBR_COLD_DAMAGE, "damage_cold", true, false },
    { MBR_ENERGY_DAMAGE, "damage_energy", true, false },
    { MBR_POISON_DAMAGE, "damage_poison", true, false },  // 160
    { MBR_PHYSICAL_DAMAGE, "damage_physical", true, false },
    { MBR_FIRE_DAMAGE_MOD, "damage_fire_mod", true, false },
    { MBR_COLD_DAMAGE_MOD, "damage_cold_mod", true, false },
    { MBR_ENERGY_DAMAGE_MOD, "damage_energy_mod", true, false },
    { MBR_POISON_DAMAGE_MOD, "damage_poison_mod", true, false },  // 165
    { MBR_PHYSICAL_DAMAGE_MOD, "damage_physical_mod", true, false },
    { MBR_PARTY, "party", true, false },
    { MBR_LEADER, "leader", true, false },
    { MBR_PARTYLOOT, "partycanloot", true, false },
    { MBR_CANDIDATE_OF_PARTY, "candidate_of_party", true, false },  // 170
    { MBR_CANDIDATES, "candidates", true, false },
    { MBR_MOVECOST_WALK, "movecost_walk_mod", true, false },
    { MBR_MOVECOST_RUN, "movecost_run_mod", true, false },
    { MBR_MOVECOST_WALK_MOUNTED, "movecost_walk_mounted_mod", true, false },
    { MBR_MOVECOST_RUN_MOUNTED, "movecost_run_mounted_mod", true, false },  // 175
    { MBR_AGGRESSORTO, "aggressorto", true, false },
    { MBR_LAWFULLYDAMAGED, "lawfullydamaged", true, false },
    { MBR_GETGOTTENBY, "getgottenby", true, false },
    { MBR_UO_EXPANSION_CLIENT, "uo_expansion_client", true, false },
    { MBR_CLIENTTYPE, "clienttype", true, false },  // 180
    { MBR_DEAFENED, "deafed", true, false },
    { MBR_CLIENT, "client", true, false },
    { MBR_TYPE, "type", true, false },
    { MBR_ATTRIBUTES, "attributes", true, false },
    { MBR_EDITING, "house_editing", true, false },  // 185
    { MBR_HOUSEPARTS, "house_parts", true, false },
    { MBR_DOUBLECLICKRANGE, "doubleclickrange", false, false },
    { MBR_MOUNTEDSTEPS, "mountedsteps", false, false },
    // New boat stuff start
    { MBR_ROPE, "rope", true, false },
    { MBR_WHEEL, "wheel", true, false },  // 190
    { MBR_HULL, "hull", true, false },
    { MBR_TILLER, "tiller", true, false },
    { MBR_RUDDER, "rudder", true, false },
    { MBR_SAILS, "sails", true, false },
    { MBR_STORAGE, "storage", true, false },  // 195
    { MBR_WEAPONSLOT, "weaponslot", true, false },
    // New boat stuff end
    { MBR_MULTIID, "multiid", true, false },
    { MBR_TRADEWINDOW, "tradewindow", true, false },
    { MBR_LASTCOORD, "lastcoord", true, false },
    { MBR_FACETID, "facetid", true, false },  // 200
    { MBR_EDITABLE, "editable", true, false },
    { MBR_ACTIVE_SKILL, "active_skill", true, false },
    { MBR_CASTING_SPELL, "casting_spell", true, false },
    { MBR_CARRYINGCAPACITY_MOD, "carrying_capacity_mod", false, false },
    { MBR_MAX_ITEMS_MOD, "max_items_mod", false, false },  // 205
    { MBR_MAX_WEIGHT_MOD, "max_weight_mod", false, false },
    { MBR_MAX_SLOTS_MOD, "max_slots_mod", false, false },
    { MBR_SPEED_MOD, "speed_mod", false, false },
    { MBR_NAME_SUFFIX, "name_suffix", false, false },
    { MBR_TEMPORALLY_CRIMINAL, "temporally_criminal", true, false },  // 210
    { MBR_LAST_TEXTCOLOR, "last_textcolor", true, false },
    { MBR_INSURED, "insured", false, false },
    { MBR_LAST_ACTIVITY_AT, "last_activity_at", false, false },
    { MBR_LAST_PACKET_AT, "last_packet_at", false, false },
    { MBR_HOUSE, "house", true, false },  // 215, Item
    { MBR_SPECIFIC_NAME, "specific_name", true, false },
    { MBR_CARRYINGCAPACITY, "carrying_capacity", true, false },
    { MBR_NO_DROP, "no_drop", false, false },
    { MBR_NO_DROP_EXCEPTION, "no_drop_exception", false, false },
    { MBR_PORT, "port", false, false },  // 220
    // Additions for new properties
    { MBR_LOWER_REAG_COST, "lower_reagent_cost", true, false },
    { MBR_SPELL_DAMAGE_INCREASE, "spell_damage_increase", true, false },
    { MBR_FASTER_CASTING, "faster_casting", true, false },
    { MBR_FASTER_CAST_RECOVERY, "faster_cast_recovery", true, false },
    { MBR_DEFENCE_CHANCE_INCREASE, "defence_increase", true, false },  // 225
    { MBR_DEFENCE_CHANCE_INCREASE_CAP, "defence_increase_cap", true, false },
    { MBR_LOWER_MANA_COST, "lower_mana_cost", true, false },
    { MBR_FIRE_RESIST_CAP, "resist_fire_cap", true, false },
    { MBR_COLD_RESIST_CAP, "resist_cold_cap", true, false },
    { MBR_ENERGY_RESIST_CAP, "resist_energy_cap", true, false },  // 230
    { MBR_POISON_RESIST_CAP, "resist_poison_cap", true, false },
    { MBR_PHYSICAL_RESIST_CAP, "resist_physical_cap", true, false },
    { MBR_HIT_CHANCE, "hit_chance", true, false },
    // Additions for new properties mods
    { MBR_LOWER_REAG_COST_MOD, "lower_reagent_cost_mod", false, false },
    { MBR_SPELL_DAMAGE_INCREASE_MOD, "spell_damage_increase_mod", false, false },
    { MBR_FASTER_CASTING_MOD, "faster_casting_mod", false, false },  // 235
    { MBR_FASTER_CAST_RECOVERY_MOD, "faster_cast_recovery_mod", false, false },
    { MBR_DEFENCE_CHANCE_INCREASE_MOD, "defence_increase_mod", false, false },
    { MBR_DEFENCE_CHANCE_INCREASE_CAP_MOD, "defence_increase_cap_mod", false, false },
    { MBR_LOWER_MANA_COST_MOD, "lower_mana_cost_mod", false, false },
    { MBR_FIRE_RESIST_CAP_MOD, "resist_fire_cap_mod", false, false },  // 240
    { MBR_COLD_RESIST_CAP_MOD, "resist_cold_cap_mod", false, false },
    { MBR_ENERGY_RESIST_CAP_MOD, "resist_energy_cap_mod", false, false },
    { MBR_POISON_RESIST_CAP_MOD, "resist_poison_cap_mod", false, false },
    { MBR_PHYSICAL_RESIST_CAP_MOD, "resist_physical_cap_mod", false, false },
    { MBR_LUCK_MOD, "luck_mod", false, false },  // 245
    { MBR_HIT_CHANCE_MOD, "hit_chance_mod", false, false },
    { MBR_PACKAGE, "package", true, false },
    { MBR_SWING_SPEED_INCREASE, "swing_speed_increase", true, false },
    { MBR_SWING_SPEED_INCREASE_MOD, "swing_speed_increase_mod", false, false },
    { MBR_EXPORTED_FUNCTIONS, "exported_functions", false, false },  // 250
    { MBR_DISABLE_INACTIVITY_TIMEOUT, "disable_inactivity_timeout", false, false },
    { MBR_CURSED, "cursed", false, false },
    { MBR_SNOOPSCRIPT, "snoopscript", false, false },
    { MBR_CHARACTER_OWNER, "character_owner", true, false },
    { MBR_PARRYCHANCE_MOD, "parrychance_mod", false, false },  // 255
    { MBR_PILOT, "pilot", false, false },
    { MBR_BUFFS, "buffs", false, false },
    { MBR_WEIGHT_MULTIPLIER_MOD, "weight_multiplier_mod", false, false },
    { MBR_HELD_WEIGHT_MULTIPLIER, "held_weight_multiplier", false, false },
    { MBR_FUNCTION, "function", false, true },
};
int n_objmembers = sizeof object_members / sizeof object_members[0];
ObjMember* getKnownObjMember( const char* token )
{
  static auto cache = []() -> std::unordered_map<std::string, ObjMember*>
  {
    std::unordered_map<std::string, ObjMember*> m;
    for ( int i = 0; i < n_objmembers; ++i )
    {
      if ( !object_members[i].hidden )
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
  else
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
  else
    return &( object_methods[id] );
}

}  // namespace Bscript
}  // namespace Pol
