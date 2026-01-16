/** @file
 *
 * @par History
 * - 2005/05/24 Shinigami: added ObjMethod account.delete() to delete this account
 * - 2005/05/25 Shinigami: added ObjMethod account.split( newacctname : string, index : 1..5 )
 *                         to create a new account and move character to it
 * - 2005/05/25 Shinigami: added ObjMethod account.move_char( destacctname : string, index : 1..5 )
 *                         to move character from this account to destaccount
 * - 2005/09/12 Shinigami: added ObjMethods packet.GetIntxxFlipped and packet.SetIntxxFlipped (Byte
 * Order)
 * - 2006/09/16 Shinigami: added ObjMethods packet.GetUnicodeStringFlipped and
 * packet.SetUnicodeStringFlipped (Byte Order)
 * - 2009/08/06 MuadDib:   Added GetGottenBy to find who is holding the item. Returning char ref of
 * course.
 * - 2009/10/09 Turley:    Added spellbook.spells() & .hasspell() methods
 * - 2009/10/10 Turley:    Added spellbook.addspell() & .removespell() methods
 * - 2009/10/14 Turley:    Added char.deaf() methods & char.deafened member
 */


#ifndef OBJMETHODS_H
#define OBJMETHODS_H

#include <fmt/format.h>

namespace Pol
{
namespace Bscript
{
enum MethodID
{
  MTH_ISA,         // uobject  //0
  MTH_SET_MEMBER,  // 1
  MTH_GET_MEMBER,
  MTH_SETPOISONED,  // character
  MTH_SETPARALYZED,
  MTH_SETCRIMINAL,  // 5
  MTH_SETLIGHTLEVEL,
  MTH_SQUELCH,
  MTH_ENABLE,
  MTH_DISABLE,
  MTH_ENABLED,  // 10
  MTH_SETCMDLEVEL,
  MTH_SPENDGOLD,
  MTH_SETMURDERER,
  MTH_REMOVEREPORTABLE,
  MTH_GETGOTTENITEM,  // 15
  MTH_CLEARGOTTENITEM,
  MTH_SETWARMODE,
  MTH_SETMASTER,             // npc
  MTH_MOVE_OFFLINE_MOBILES,  // boat
  MTH_SETCUSTOM,             // house  //20
  MTH_GETPINS,               // map
  MTH_INSERTPIN,
  MTH_APPENDPIN,
  MTH_ERASEPIN,
  MTH_OPEN,  // door  //25
  MTH_CLOSE,
  MTH_TOGGLE,
  MTH_BAN,  // account
  MTH_UNBAN,
  MTH_SETPASSWORD,  // 30
  MTH_CHECKPASSWORD,
  MTH_SETNAME,
  MTH_GETCHARACTER,
  MTH_DELETECHARACTER,
  MTH_GETPROP,  // 35
  MTH_SETPROP,
  MTH_ERASEPROP,
  MTH_PROPNAMES,
  MTH_ISMEMBER,     // guild
  MTH_ISALLYGUILD,  // 40
  MTH_ISENEMYGUILD,
  MTH_ADDMEMBER,
  MTH_ADDALLYGUILD,
  MTH_ADDENEMYGUILD,
  MTH_REMOVEMEMBER,  // 45
  MTH_REMOVEALLYGUILD,
  MTH_REMOVEENEMYGUILD,
  MTH_SIZE,  // ARRAY
  MTH_ERASE,
  MTH_INSERT,  // 50
  MTH_SHRINK,
  MTH_APPEND,
  MTH_REVERSE,
  MTH_SORT,
  MTH_EXISTS,  // 55
  MTH_KEYS,
  MTH_SENDPACKET,  // packet
  MTH_SENDAREAPACKET,
  MTH_GETINT8,
  MTH_GETINT16,  // 60
  MTH_GETINT32,
  MTH_SETINT8,
  MTH_SETINT16,
  MTH_SETINT32,
  MTH_GETSTRING,  // 65
  MTH_GETUNICODESTRING,
  MTH_SETSTRING,
  MTH_SETUNICODESTRING,
  MTH_GETSIZE,
  MTH_SETSIZE,        // 70
  MTH_CREATEELEMENT,  // datastore
  MTH_FINDELEMENT,
  MTH_DELETEELEMENT,
  MTH_SENDEVENT,  // script
  MTH_KILL,       // 75
  MTH_LOADSYMBOLS,
  MTH_SET_UO_EXPANSION,
  MTH_CLEAR_EVENT_QUEUE,
  MTH_ADD_COMPONENT,
  MTH_ERASE_COMPONENT,  // 80
  MTH_DELETE,
  MTH_SPLIT,
  MTH_MOVE_CHAR,
  MTH_GETINT16FLIPPED,
  MTH_GETINT32FLIPPED,  // 85
  MTH_SETINT16FLIPPED,
  MTH_SETINT32FLIPPED,
  MTH_GETCORPSE,
  MTH_SETDEFAULTCMDLEVEL,
  MTH_PRIVILEGES,  // 90
  MTH_GETUNICODESTRINGFLIPPED,
  MTH_SETUNICODESTRINGFLIPPED,
  MTH_ADD_CHARACTER,
  MTH_SET_SWINGTIMER,
  MTH_ATTACK_ONCE,  // 95
  MTH_SETFACING,
  MTH_COMPAREVERSION,
  MTH_SETLEADER,
  MTH_ADDCANDIDATE,
  MTH_REMOVECANDIDATE,  // 100
  MTH_RANDOMENTRY,
  MTH_SEEK,
  MTH_PEEK,
  MTH_TELL,
  MTH_FLUSH,  // 105
  MTH_GETSINT8,
  MTH_GETSINT16,
  MTH_GETSINT32,
  MTH_SETSINT8,
  MTH_SETSINT16,  // 110
  MTH_SETSINT32,
  MTH_SETAGGRESSORTO,
  MTH_SETLAWFULLYDAMAGEDTO,
  MTH_CLEARAGGRESSORTO,
  MTH_CLEARLAWFULLYDAMAGEDTO,  // 115
  MTH_HASSPELL,
  MTH_SPELLS,
  MTH_REMOVESPELL,
  MTH_ADDSPELL,
  MTH_DEAF,  // 120
  MTH_SETSEASON,
  MTH_NEXTSIBLING,
  MTH_FIRSTCHILD,
  MTH_SAVEXML,
  MTH_APPENDNODE,  // 125
  MTH_SETDECLARATION,
  MTH_SETATTRIBUTE,
  MTH_REMOVEATTRIBUTE,
  MTH_REMOVENODE,
  MTH_APPENDTEXT,  // 130
  MTH_XMLTOSTRING,
  MTH_APPENDXMLCOMMENT,
  MTH_ADD_HOUSE_PART,
  MTH_ERASE_HOUSE_PART,
  MTH_ACCEPT_COMMIT,  // 135
  MTH_SPLITSTACK_AT,
  MTH_SPLITSTACK_INTO,
  MTH_CANCEL_EDITING,
  MTH_CLONENODE,
  MTH_HAS_EXISTING_STACK,  // 140
  MTH_LENGTH,
  MTH_JOIN,
  MTH_FIND,
  MTH_UPPER,
  MTH_LOWER,  // 145
  MTH_FORMAT,
  MTH_DISABLE_SKILLS_FOR,
  MTH_CYCLE,
  MTH_ADD_BUFF,
  MTH_DEL_BUFF,  // 150
  MTH_CLEAR_BUFFS,
  MTH_CALL,
  MTH_SORTEDINSERT,
  MTH_SETUTF8STRING,
  MTH_SET_PILOT,  // 155
  MTH_SET_ALTERNATE_MULTIID,
  MTH_FILTER,
  MTH_REDUCE,
  MTH_MAP,
  MTH_FINDINDEX,  // 160
  MTH_SET_MULTIID,
  MTH_NEW,
  MTH_CALL_METHOD,
  MTH_STACKTRACE,
  MTH_ASSIGN,  // 165
  MTH_MATCH,
  MTH_REPLACE,
};

inline auto format_as( MethodID id )
{
  return fmt::underlying( id );
}


using ObjMethod = struct
{
  MethodID id;
  char code[30];
  bool overridden;
};

extern ObjMethod object_methods[];
extern int n_objmethods;

ObjMethod* getKnownObjMethod( const char* token );
ObjMethod* getObjMethod( int id );
}  // namespace Bscript
}  // namespace Pol
#endif
