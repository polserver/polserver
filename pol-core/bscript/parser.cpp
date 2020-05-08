/** @file
 *
 * @par History
 * - 2005/01/24 Shinigami: added ObjMember character.spyonclient2 to get data from packet 0xd9 (Spy
 * on Client 2)
 * - 2005/03/09 Shinigami: added Prop Character::Delay_Mod [ms] for WeaponDelay
 * - 2005/04/04 Shinigami: added Prop Character::CreatedAt [PolClock]
 * - 2005/05/24 Shinigami: added ObjMethod account.delete() to delete this account
 * - 2005/05/25 Shinigami: added ObjMethod account.split( newacctname : string, index : 1..5 )
 *                         to create a new account and move character to it
 * - 2005/05/25 Shinigami: added ObjMethod account.move_char( destacctname : string, index : 1..5 )
 *                         to move character from this account to destaccount
 * - 2005/08/29 Shinigami: character.spyonclient2 renamed to character.clientinfo
 * - 2005/09/12 Shinigami: added ObjMethods packet.GetIntxxFlipped and packet.SetIntxxFlipped (Byte
 * Order)
 * - 2005/10/02 Shinigami: added Prop Script.attached_to and Script.controller
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2005/12/06 MuadDib:   Added uclang member for storing UC language from client.
 * - 2006/05/16 Shinigami: added Prop Character.Race [RACE_* Constants] to support Elfs
 * - 2006/09/16 Shinigami: added ObjMethods packet.GetUnicodeStringFlipped and
 * packet.SetUnicodeStringFlipped (Byte Order)
 * - 2007/07/09 Shinigami: added Prop Character.isUOKR [bool] - UO:KR client used?
 * - 2008/07/08 Turley:    Added character.movemode - returns the MoveMode like given in NPCDesc
 *                         Added item.stackable - Is item stackable?
 * - 2008/12/17 MuadDub:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
 * - 2009/08/06 MuadDib:   Removed PasswordOnlyHash support
 *                         GetGottenBy Method
 * - 2009/08/19 Turley:    Added character.uo_expansion_client
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: ParseErrorStr and ReservedWord changed little bit
 * - 2009/09/06 Turley:    Removed chr.isUOKR added chr.ClientType
 * - 2009/10/09 Turley:    Added spellbook.spells() & .hasspell() methods
 * - 2009/10/10 Turley:    Added spellbook.addspell() & .removespell() methods
 * - 2009/10/14 Turley:    Added char.deaf() methods & char.deafened member
 * - 2011/12/13 Tomi:      Boat members MBR_COMPONENT, MBR_HULL, MBR_ROPE, MBR_SAILS, MBR_WHEEL,
 * MBR_TILLER, MBR_RUDDER, MBR_STORAGE, MBR_WEAPONSLOT
 * - 2012/02/02 Tomi:      Multi member MBR_MULTIID
 * - 2012/03/26 Tomi:      Added MBR_LASTCOORD
 * - 2012/04/14 Tomi:      Added MBR_FACETID for new map message packet
 * - 2012/04/15 Tomi:      Added MBR_EDITABLE for maps
 * - 2012/06/02 Tomi:      Added MBR_ACTIVE_SKILL and MBR_CASTING_SPELL for characters
 */

#include "parser.h"

#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>

#include "../clib/clib.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../clib/strutil.h"
#include "../clib/unittest.h"
#include "compctx.h"
#include "compilercfg.h"
#include "expression.h"
#include "fmodule.h"
#include "impstr.h"
#include "modules.h"
#include "objmembers.h"
#include "objmethods.h"
#include "token.h"
#include "tokens.h"
#include <format/format.h>

namespace Pol
{
namespace Bscript
{
static void init_tables();

Parser::Parser() : quiet( 0 ), err( PERR_NONE ), contains_tabs( false )
{
  memset( &ext_err, 0, sizeof( ext_err ) );
  memset( &buffer, 0, sizeof( buffer ) );
  init_tables();
}

const char* ParseErrorStr[PERR_NUM_ERRORS] = {"(No Error, or not specified)",
                                              "Unexpected ')'",
                                              "Missing '('",
                                              "Missing ')'",
                                              "Bad Token",
                                              "Unknown Operator",
                                              "Waaah!",
                                              "Unterminated String Literal",
                                              "Invalid escape sequence in String",
                                              "Too Few Arguments",
                                              "Too Many Arguments",
                                              "Unexpected Comma",
                                              "Illegal Construction",
                                              "Missing ';'",
                                              "Token not legal here",
                                              "Procedure calls not allowed here",
                                              "Unexpected Semicolon",
                                              "Expected 'while'",
                                              "Unexpected ']'",
                                              "Missing ']'"};
char operator_brk[] = "+-/*(),<=>,:;%";

char ident_allowed[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "_";  // $%@& removed 9/17/1998 Syz


int allowed_table[8][8] = {
    /* this token is a(n)... */
    /*           binary   unary
     * TERMINATOR OPERAND OPERATOR OPERATOR LPAREN RPAREN  LBRACK  RBRACK*/
    /* Last token was a(n)... */
    {1, 1, 0, 1, 1, 0, 0, 0}, /* TERMINATOR */
    {1, 0, 1, 0, 0, 1, 1, 1}, /* OPERAND */
    {0, 1, 0, 1, 1, 0, 0, 0}, /* BINARY OPERATOR */
    {0, 1, 0, 0, 1, 0, 0, 0}, /* UNARY OPERATOR */
    {0, 1, 0, 1, 1, 0, 0, 0}, /* LEFT PARENTHESIS */
    {1, 0, 1, 0, 0, 1, 0, 1}, /* RIGHT PARENTHESIS */
    {0, 1, 0, 0, 1, 0, 1, 0}, /* LEFT BRACKET */
    {1, 0, 1, 0, 0, 1, 1, 1}, /* RIGHT BRACKET */
    /* Separators are always allowed.  Terminators are mostly always allowed. */
};
/* examples matrix: -- connected denotes unary operator

legal:
{  TT     T AB       T -AB  T (               },
{  AB T        AB +           AB )  A[    A]  },
{       * AB         *-    * (              },
{        -AB            -(              },
{       ( AB         (-    ( (              },
{  ) T        ) -            ) )      )]  }
{       [A    .    [-    [(       [[     .  }
{  ] T   .      ] *     .     .    ])   ][    ][  }

illegal:
{            T b-A          T )  T [    T ]   },
{      AB AB        AB~   AB (              },
{  * T        * /            - )  *[    *]  },
{  b- T        -*    --         -)  -[    -]  },
{  ( T        ( *            ( )  ([    (]  },
{       ) AB         )-    ) (      )[      }
{  [ T        [+             [)       []  }
{       ]A         ]-    ] (           .  }

*/


/* operator characters      // (and precedence table)
    ( )   [ ]
    + - (unary-arithmetic)   ! (unary-logical)  ~ (unary-boolean)
    * / %
    + -
    < <= > >=
    == <>
    &  (band ?)
    ^  (bxor ?)
    |  (bor  ?)
    and
    or
    :=
    ,

    Problem: How to recognize a unary operator?
    Proposed solution: If a binary operator would be legal, use it
    otherwise, it must be a unary operator.
    Examples: (-5+7)
    1) grabs '('.  now only lparens and operands are legal.
    2) grabs '-'.  Since this is a binary operator (TYP_OPERATOR)
    by default, this is illegal.  Since it is illegal,
    parseToken() tries the unary operator table, and finds it.
    ( now, only left parens and operands are legal. )
    3) gets 5, an operand.
    Alternative: Pass getToken()a parameter denoting what is legal and
    let it ignore possibilities that are illegal.

    See end of file for handling unary operators
    */

/*
        The precedence table should be split from the operator
        recognition table, because some operators are words and
        have to be picked up in the reserved word table.
        */


Operator binary_operators[] = {

    {"(", TOK_LPAREN, PREC_PAREN, TYP_LEFTPAREN, false, false},
    {")", TOK_RPAREN, PREC_PAREN, TYP_RIGHTPAREN, false, false},
    {"[", TOK_LBRACKET, PREC_PAREN, TYP_LEFTBRACKET, false, false},
    {"]", TOK_RBRACKET, PREC_PAREN, TYP_RIGHTBRACKET, false, false},
    {"{", TOK_LBRACE, PREC_PAREN, TYP_LEFTBRACE, false, false},
    {"}", TOK_RBRACE, PREC_PAREN, TYP_RIGHTBRACE, false, false},

    {".", TOK_MEMBER, PREC_PAREN, TYP_OPERATOR, true, false},
    {"->", TOK_DICTKEY, PREC_ASSIGN, TYP_RESERVED, false, false},

    {"*", TOK_MULT, PREC_MULT, TYP_OPERATOR, true, false},
    {"/", TOK_DIV, PREC_MULT, TYP_OPERATOR, true, false},
    {"%", TOK_MODULUS, PREC_MULT, TYP_OPERATOR, true, false},

    {"+", TOK_ADD, PREC_PLUS, TYP_OPERATOR, true, false},
    {"-", TOK_SUBTRACT, PREC_PLUS, TYP_OPERATOR, true, false},

    {"+=", TOK_PLUSEQUAL, PREC_ASSIGN, TYP_OPERATOR, false, false},
    {"-=", TOK_MINUSEQUAL, PREC_ASSIGN, TYP_OPERATOR, false, false},
    {"*=", TOK_TIMESEQUAL, PREC_ASSIGN, TYP_OPERATOR, false, false},
    {"/=", TOK_DIVIDEEQUAL, PREC_ASSIGN, TYP_OPERATOR, false, false},
    {"%=", TOK_MODULUSEQUAL, PREC_ASSIGN, TYP_OPERATOR, false, false},

    {"<=", TOK_LESSEQ, PREC_LESSTHAN, TYP_OPERATOR, false, false},
    {"<", TOK_LESSTHAN, PREC_LESSTHAN, TYP_OPERATOR, true, false},
    {">=", TOK_GREQ, PREC_LESSTHAN, TYP_OPERATOR, false, false},
    {">", TOK_GRTHAN, PREC_LESSTHAN, TYP_OPERATOR, true, false},

    {">>", TOK_BSRIGHT, PREC_BSRIGHT, TYP_OPERATOR, false, false},
    {"<<", TOK_BSLEFT, PREC_BSLEFT, TYP_OPERATOR, false, false},
    {"&", TOK_BITAND, PREC_BITAND, TYP_OPERATOR, true, false},
    {"^", TOK_BITXOR, PREC_BITXOR, TYP_OPERATOR, false, false},
    {"|", TOK_BITOR, PREC_BITOR, TYP_OPERATOR, true, false},

    {"<>", TOK_NEQ, PREC_EQUALTO, TYP_OPERATOR, false, false},
    {"!=", TOK_NEQ, PREC_EQUALTO, TYP_OPERATOR, false, false},
    {"=", TOK_EQUAL1, PREC_EQUALTO, TYP_OPERATOR, true, false},  // deprecated: :=/==
    {"==", TOK_EQUAL, PREC_EQUALTO, TYP_OPERATOR, false, false},

    //  { "and",  TOK_AND,  PREC_LOGAND,  TYP_OPERATOR, false, false },
    {"&&", TOK_AND, PREC_LOGAND, TYP_OPERATOR, false, false},

    //  { "or",  TOK_OR,     PREC_LOGOR,  TYP_OPERATOR, false, false },
    {"||", TOK_OR, PREC_LOGOR, TYP_OPERATOR, false, false},

    {"?:", RSV_ELVIS, PREC_ELVIS, TYP_OPERATOR, false, false},

    {":=", TOK_ASSIGN, PREC_ASSIGN, TYP_OPERATOR, false, false},
    {".+", TOK_ADDMEMBER, PREC_ASSIGN, TYP_OPERATOR, false, false},
    {".-", TOK_DELMEMBER, PREC_ASSIGN, TYP_OPERATOR, false, false},
    {".?", TOK_CHKMEMBER, PREC_ASSIGN, TYP_OPERATOR, false, false},

    {",", TOK_COMMA, PREC_COMMA, TYP_SEPARATOR, false, false},
    {"", TOK_TERM, PREC_TERMINATOR, TYP_TERMINATOR, false, false},
    {"++", TOK_ADD, PREC_PLUS, TYP_UNARY_PLACEHOLDER, false,
     false},  // fake entry will be converted to unary
    {"--", TOK_SUBTRACT, PREC_PLUS, TYP_UNARY_PLACEHOLDER, false,
     false},  // fake entry will be converted to unary
};
int n_operators = sizeof binary_operators / sizeof binary_operators[0];

Operator unary_operators[] = {
    {"+", TOK_UNPLUS, PREC_UNARY_OPS, TYP_UNARY_OPERATOR, false, false},
    {"-", TOK_UNMINUS, PREC_UNARY_OPS, TYP_UNARY_OPERATOR, false, false},
    {"!", TOK_LOG_NOT, PREC_UNARY_OPS, TYP_UNARY_OPERATOR, false, false},
    {"~", TOK_BITWISE_NOT, PREC_UNARY_OPS, TYP_UNARY_OPERATOR, false, false},
    {"@", TOK_FUNCREF, PREC_UNARY_OPS, TYP_FUNCREF, false, false},
    {"++", TOK_UNPLUSPLUS, PREC_UNARY_OPS, TYP_UNARY_OPERATOR, true, false},
    {"--", TOK_UNMINUSMINUS, PREC_UNARY_OPS, TYP_UNARY_OPERATOR, true, false},
    //  { "not", TOK_LOG_NOT, PREC_UNARY_OPS, TYP_UNARY_OPERATOR, false, false }
    // "refto", TOK_REFTO, 12, TYP_UNARY_OPERATOR, false, false
};
int n_unary = sizeof unary_operators / sizeof unary_operators[0];

ObjMember object_members[] = {
    // MBR_*, "name", read_only
    {MBR_X, "x", true},  // 0
    {MBR_Y, "y", true},  // 1
    {MBR_Z, "z", true},
    {MBR_NAME, "name", false},
    {MBR_OBJTYPE, "objtype", true},
    {MBR_GRAPHIC, "graphic", false},  // 5
    {MBR_SERIAL, "serial", true},
    {MBR_COLOR, "color", false},
    {MBR_HEIGHT, "height", true},
    {MBR_FACING, "facing", false},
    {MBR_DIRTY, "dirty", true},  // 10
    {MBR_WEIGHT, "weight", true},
    {MBR_MULTI, "multi", true},
    {MBR_AMOUNT, "amount", true},  // item
    {MBR_LAYER, "layer", true},
    {MBR_CONTAINER, "container", true},  // 15
    {MBR_USESCRIPT, "usescript", false},
    {MBR_EQUIPSCRIPT, "equipscript", false},
    {MBR_UNEQUIPSCRIPT, "unequipscript", false},
    {MBR_DESC, "desc", false},
    {MBR_MOVABLE, "movable", false},  // 20
    {MBR_INVISIBLE, "invisible", false},
    {MBR_DECAYAT, "decayat", false},
    {MBR_SELLPRICE, "sellprice", false},
    {MBR_BUYPRICE, "buyprice", false},
    {MBR_NEWBIE, "newbie", false},  // 25
    {MBR_ITEM_COUNT, "item_count", true},
    {MBR_WARMODE, "warmode", true},  // character
    {MBR_GENDER, "gender", false},
    {MBR_TRUEOBJTYPE, "trueobjtype", false},
    {MBR_TRUECOLOR, "truecolor", false},  // 30
    {MBR_AR_MOD, "ar_mod", false},
    {MBR_HIDDEN, "hidden", false},
    {MBR_CONCEALED, "concealed", false},
    {MBR_FROZEN, "frozen", false},
    {MBR_PARALYZED, "paralyzed", false},  // 35
    {MBR_POISONED, "poisoned", false},
    {MBR_STEALTHSTEPS, "stealthsteps", false},
    {MBR_SQUELCHED, "squelched", true},
    {MBR_DEAD, "dead", true},
    {MBR_AR, "ar", true},  // 40
    {MBR_BACKPACK, "backpack", true},
    {MBR_WEAPON, "weapon", true},
    {MBR_SHIELD, "shield", true},
    {MBR_ACCTNAME, "acctname", true},
    {MBR_ACCT, "acct", true},  // 45
    {MBR_CMDLEVEL, "cmdlevel", false},
    {MBR_CMDLEVELSTR, "cmdlevelstr", true},
    {MBR_CRIMINAL, "criminal", true},
    {MBR_IP, "ip", true},
    {MBR_GOLD, "gold", true},  // 50
    {MBR_TITLE_PREFIX, "title_prefix", false},
    {MBR_TITLE_SUFFIX, "title_suffix", false},
    {MBR_TITLE_GUILD, "title_guild", false},
    {MBR_TITLE_RACE, "title_race", false},
    {MBR_GUILDID, "guildid", true},  // 55
    {MBR_GUILD, "guild", true},
    {MBR_MURDERER, "murderer", false},
    {MBR_ATTACHED, "attached", true},
    {MBR_CLIENTVERSION, "clientversion", true},
    {MBR_REPORTABLES, "reportables", true},  // 60
    {MBR_SCRIPT, "script", false},           // npc
    {MBR_NPCTEMPLATE, "npctemplate", true},
    {MBR_MASTER, "master", true},
    {MBR_PROCESS, "process", true},
    {MBR_EVENTMASK, "eventmask", true},  // 65
    {MBR_SPEECH_COLOR, "speech_color", false},
    {MBR_SPEECH_FONT, "speech_font", false},
    {MBR_USE_ADJUSTMENTS, "use_adjustments", false},
    {MBR_RUN_SPEED, "run_speed", false},
    {MBR_LOCKED, "locked", false},         // lockable //70
    {MBR_CORPSETYPE, "corpsetype", true},  // corpse
    {MBR_TILLERMAN, "tillerman", true},    // boat
    {MBR_PORTPLANK, "portplank", true},
    {MBR_STARBOARDPLANK, "starboardplank", true},
    {MBR_HOLD, "hold", true},  // 75
    {MBR_HAS_OFFLINE_MOBILES, "has_offline_mobiles", true},
    {MBR_COMPONENTS, "components", true},  // house
    {MBR_ITEMS, "items", true},            // multi
    {MBR_MOBILES, "mobiles", true},
    {MBR_XEAST, "xeast", false},  // map //80
    {MBR_XWEST, "xwest", false},
    {MBR_YNORTH, "ynorth", false},
    {MBR_YSOUTH, "ysouth", false},
    {MBR_GUMPWIDTH, "gumpwidth", false},
    {MBR_GUMPHEIGHT, "gumpheight", false},  // 85
    {MBR_ISOPEN, "isopen", true},           // door
    {MBR_QUALITY, "quality", false},        // equipment
    {MBR_HP, "hp", false},
    {MBR_MAXHP_MOD, "maxhp_mod", false},
    {MBR_MAXHP, "maxhp", true},       // 90
    {MBR_DMG_MOD, "dmg_mod", false},  // weapon
    {MBR_ATTRIBUTE, "attribute", true},
    {MBR_INTRINSIC, "intrinsic", true},
    {MBR_HITSCRIPT, "hitscript", false},
    {MBR_AR_BASE, "ar_base", true},  // 95
    {MBR_ONHIT_SCRIPT, "onhitscript", false},
    {MBR_ENABLED, "enabled", true},  // account
    {MBR_BANNED, "banned", true},
    {MBR_USERNAMEPASSWORDHASH, "usernamepasswordhash", true},
    {MBR_MEMBERS, "members", true},  // guild //100
    {MBR_ALLYGUILDS, "allyguilds", true},
    {MBR_ENEMYGUILDS, "enemyguilds", true},
    {MBR_PID, "pid", true},  // script
    {MBR_STATE, "state", true},
    {MBR_INSTR_CYCLES, "instr_cycles", true},  // 105
    {MBR_SLEEP_CYCLES, "sleep_cycles", true},
    {MBR_CONSEC_CYCLES, "consec_cycles", true},
    {MBR_PC, "pc", true},
    {MBR_CALL_DEPTH, "call_depth", true},
    {MBR_NUM_GLOBALS, "num_globals", true},  // 110
    {MBR_VAR_SIZE, "var_size", true},
    {MBR_REALM, "realm", true},
    {MBR_UO_EXPANSION, "uo_expansion", true},
    {MBR_CUSTOM, "custom", true},    // house
    {MBR_GLOBALS, "globals", true},  // 115
    {MBR_FOOTPRINT, "footprint", true},
    {MBR_CLIENTINFO, "clientinfo", true},
    {MBR_DELAY_MOD, "delay_mod", false},
    {MBR_CREATEDAT, "createdat", true},
    {MBR_OPPONENT, "opponent", true},  // 120
    {MBR_CONNECTED, "connected", true},
    {MBR_ATTACHED_TO, "attached_to", true},
    {MBR_CONTROLLER, "controller", true},
    {MBR_OWNERSERIAL, "ownerserial", true},
    {MBR_DEFAULTCMDLEVEL, "defaultcmdlevel", true},  // 125
    {MBR_UCLANG, "uclang", true},
    {MBR_RACE, "race", false},
    {MBR_TRADING_WITH, "trading_with", false},
    {MBR_TRADE_CONTAINER, "trade_container", false},
    {MBR_ALIGNMENT, "alignment", false},  // 130
    {MBR_CURSOR, "cursor", false},
    {MBR_GUMP, "gump", false},
    {MBR_PROMPT, "prompt", false},
    {MBR_STACKABLE, "stackable", false},
    {MBR_MOVEMODE, "movemode", false},  // 135
    {MBR_HITCHANCE_MOD, "hitchance_mod", false},
    {MBR_EVASIONCHANCE_MOD, "evasionchance_mod", false},
    {MBR_TILE_LAYER, "tile_layer", true},
    {MBR_CLIENTVERSIONDETAIL, "clientver_detail", true},
    {MBR_SAVEONEXIT, "saveonexit", true},  // 140
    {MBR_FIRE_RESIST, "resist_fire", true},
    {MBR_COLD_RESIST, "resist_cold", true},
    {MBR_ENERGY_RESIST, "resist_energy", true},
    {MBR_POISON_RESIST, "resist_poison", true},
    {MBR_PHYSICAL_RESIST, "resist_physical", true},  // 145
    {MBR_FIRE_RESIST_MOD, "resist_fire_mod", true},
    {MBR_COLD_RESIST_MOD, "resist_cold_mod", true},
    {MBR_ENERGY_RESIST_MOD, "resist_energy_mod", true},
    {MBR_POISON_RESIST_MOD, "resist_poison_mod", true},
    {MBR_PHYSICAL_RESIST_MOD, "resist_physical_mod", true},  // 150
    {MBR_STATCAP, "statcap", false},
    {MBR_SKILLCAP, "skillcap", false},
    {MBR_LUCK, "luck", false},
    {MBR_FOLLOWERSMAX, "followers_max", false},
    {MBR_TITHING, "tithing", false},  // 155
    {MBR_FOLLOWERS, "followers", false},
    {MBR_FIRE_DAMAGE, "damage_fire", true},
    {MBR_COLD_DAMAGE, "damage_cold", true},
    {MBR_ENERGY_DAMAGE, "damage_energy", true},
    {MBR_POISON_DAMAGE, "damage_poison", true},  // 160
    {MBR_PHYSICAL_DAMAGE, "damage_physical", true},
    {MBR_FIRE_DAMAGE_MOD, "damage_fire_mod", true},
    {MBR_COLD_DAMAGE_MOD, "damage_cold_mod", true},
    {MBR_ENERGY_DAMAGE_MOD, "damage_energy_mod", true},
    {MBR_POISON_DAMAGE_MOD, "damage_poison_mod", true},  // 165
    {MBR_PHYSICAL_DAMAGE_MOD, "damage_physical_mod", true},
    {MBR_PARTY, "party", true},
    {MBR_LEADER, "leader", true},
    {MBR_PARTYLOOT, "partycanloot", true},
    {MBR_CANDIDATE_OF_PARTY, "candidate_of_party", true},  // 170
    {MBR_CANDIDATES, "candidates", true},
    {MBR_MOVECOST_WALK, "movecost_walk_mod", true},
    {MBR_MOVECOST_RUN, "movecost_run_mod", true},
    {MBR_MOVECOST_WALK_MOUNTED, "movecost_walk_mounted_mod", true},
    {MBR_MOVECOST_RUN_MOUNTED, "movecost_run_mounted_mod", true},  // 175
    {MBR_AGGRESSORTO, "aggressorto", true},
    {MBR_LAWFULLYDAMAGED, "lawfullydamaged", true},
    {MBR_GETGOTTENBY, "getgottenby", true},
    {MBR_UO_EXPANSION_CLIENT, "uo_expansion_client", true},
    {MBR_CLIENTTYPE, "clienttype", true},  // 180
    {MBR_DEAFENED, "deafed", true},
    {MBR_CLIENT, "client", true},
    {MBR_TYPE, "type", true},
    {MBR_ATTRIBUTES, "attributes", true},
    {MBR_EDITING, "house_editing", true},  // 185
    {MBR_HOUSEPARTS, "house_parts", true},
    {MBR_DOUBLECLICKRANGE, "doubleclickrange", false},
    {MBR_MOUNTEDSTEPS, "mountedsteps", false},
    // New boat stuff start
    {MBR_ROPE, "rope", true},
    {MBR_WHEEL, "wheel", true},  // 190
    {MBR_HULL, "hull", true},
    {MBR_TILLER, "tiller", true},
    {MBR_RUDDER, "rudder", true},
    {MBR_SAILS, "sails", true},
    {MBR_STORAGE, "storage", true},  // 195
    {MBR_WEAPONSLOT, "weaponslot", true},
    // New boat stuff end
    {MBR_MULTIID, "multiid", true},
    {MBR_TRADEWINDOW, "tradewindow", true},
    {MBR_LASTCOORD, "lastcoord", true},
    {MBR_FACETID, "facetid", true},  // 200
    {MBR_EDITABLE, "editable", true},
    {MBR_ACTIVE_SKILL, "active_skill", true},
    {MBR_CASTING_SPELL, "casting_spell", true},
    {MBR_CARRYINGCAPACITY_MOD, "carrying_capacity_mod", false},
    {MBR_MAX_ITEMS_MOD, "max_items_mod", false},  // 205
    {MBR_MAX_WEIGHT_MOD, "max_weight_mod", false},
    {MBR_MAX_SLOTS_MOD, "max_slots_mod", false},
    {MBR_SPEED_MOD, "speed_mod", false},
    {MBR_NAME_SUFFIX, "name_suffix", false},
    {MBR_TEMPORALLY_CRIMINAL, "temporally_criminal", true},  // 210
    {MBR_LAST_TEXTCOLOR, "last_textcolor", true},
    {MBR_INSURED, "insured", false},
    {MBR_LAST_ACTIVITY_AT, "last_activity_at", false},
    {MBR_LAST_PACKET_AT, "last_packet_at", false},
    {MBR_HOUSE, "house", true},  // 215, Item
    {MBR_SPECIFIC_NAME, "specific_name", true},
    {MBR_CARRYINGCAPACITY, "carrying_capacity", true},
    {MBR_NO_DROP, "no_drop", false},
    {MBR_NO_DROP_EXCEPTION, "no_drop_exception", false},
    {MBR_PORT, "port", false},  // 220
    // Additions for new properties
    {MBR_LOWER_REAG_COST, "lower_reagent_cost", true},
    {MBR_SPELL_DAMAGE_INCREASE, "spell_damage_increase", true},
    {MBR_FASTER_CASTING, "faster_casting", true},
    {MBR_FASTER_CAST_RECOVERY, "faster_cast_recovery", true},
    {MBR_DEFENCE_CHANCE_INCREASE, "defence_increase", true},  // 225
    {MBR_DEFENCE_CHANCE_INCREASE_CAP, "defence_increase_cap", true},
    {MBR_LOWER_MANA_COST, "lower_mana_cost", true},
    {MBR_FIRE_RESIST_CAP, "resist_fire_cap", true},
    {MBR_COLD_RESIST_CAP, "resist_cold_cap", true},
    {MBR_ENERGY_RESIST_CAP, "resist_energy_cap", true},  // 230
    {MBR_POISON_RESIST_CAP, "resist_poison_cap", true},
    {MBR_PHYSICAL_RESIST_CAP, "resist_physical_cap", true},
    {MBR_HIT_CHANCE, "hit_chance", true},
    // Additions for new properties mods
    {MBR_LOWER_REAG_COST_MOD, "lower_reagent_cost_mod", false},
    {MBR_SPELL_DAMAGE_INCREASE_MOD, "spell_damage_increase_mod", false},
    {MBR_FASTER_CASTING_MOD, "faster_casting_mod", false},  // 235
    {MBR_FASTER_CAST_RECOVERY_MOD, "faster_cast_recovery_mod", false},
    {MBR_DEFENCE_CHANCE_INCREASE_MOD, "defence_increase_mod", false},
    {MBR_DEFENCE_CHANCE_INCREASE_CAP_MOD, "defence_increase_cap_mod", false},
    {MBR_LOWER_MANA_COST_MOD, "lower_mana_cost_mod", false},
    {MBR_FIRE_RESIST_CAP_MOD, "resist_fire_cap_mod", false},  // 240
    {MBR_COLD_RESIST_CAP_MOD, "resist_cold_cap_mod", false},
    {MBR_ENERGY_RESIST_CAP_MOD, "resist_energy_cap_mod", false},
    {MBR_POISON_RESIST_CAP_MOD, "resist_poison_cap_mod", false},
    {MBR_PHYSICAL_RESIST_CAP_MOD, "resist_physical_cap_mod", false},
    {MBR_LUCK_MOD, "luck_mod", false},  // 245
    {MBR_HIT_CHANCE_MOD, "hit_chance_mod", false},
    {MBR_PACKAGE, "package", true},
    {MBR_SWING_SPEED_INCREASE, "swing_speed_increase", true},
    {MBR_SWING_SPEED_INCREASE_MOD, "swing_speed_increase_mod", false},


};
int n_objmembers = sizeof object_members / sizeof object_members[0];
ObjMember* getKnownObjMember( const char* token )
{
  static auto cache = []() -> std::unordered_map<std::string, ObjMember*> {
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
  else
    return &( object_members[id] );
}

ObjMethod object_methods[] = {
    {MTH_ISA, "isa", false},                // 0
    {MTH_SET_MEMBER, "set_member", false},  // 1
    {MTH_GET_MEMBER, "get_member", false},
    {MTH_SETPOISONED, "setpoisoned", false},
    {MTH_SETPARALYZED, "setparalyzed", false},
    {MTH_SETCRIMINAL, "setcriminal", false},  // 5
    {MTH_SETLIGHTLEVEL, "setlightlevel", false},
    {MTH_SQUELCH, "squelch", false},
    {MTH_ENABLE, "enable", false},
    {MTH_DISABLE, "disable", false},
    {MTH_ENABLED, "enabled", false},  // 10
    {MTH_SETCMDLEVEL, "setcmdlevel", false},
    {MTH_SPENDGOLD, "spendgold", false},
    {MTH_SETMURDERER, "setmurderer", false},
    {MTH_REMOVEREPORTABLE, "removereportable", false},
    {MTH_GETGOTTENITEM, "getgottenitem", false},  // 15
    {MTH_CLEARGOTTENITEM, "cleargottenitem", false},
    {MTH_SETWARMODE, "setwarmode", false},
    {MTH_SETMASTER, "setmaster", false},                        // npc
    {MTH_MOVE_OFFLINE_MOBILES, "move_offline_mobiles", false},  // boat
    {MTH_SETCUSTOM, "setcustom", false},                        // house       //20
    {MTH_GETPINS, "getpins", false},                            // map
    {MTH_INSERTPIN, "insertpin", false},
    {MTH_APPENDPIN, "appendpin", false},
    {MTH_ERASEPIN, "erasepin", false},
    {MTH_OPEN, "open", false},  // door             //25
    {MTH_CLOSE, "close", false},
    {MTH_TOGGLE, "toggle", false},
    {MTH_BAN, "ban", false},  // account
    {MTH_UNBAN, "unban", false},
    {MTH_SETPASSWORD, "setpassword", false},  // 30
    {MTH_CHECKPASSWORD, "checkpassword", false},
    {MTH_SETNAME, "setname", false},
    {MTH_GETCHARACTER, "getcharacter", false},
    {MTH_DELETECHARACTER, "deletecharacter", false},
    {MTH_GETPROP, "getprop", false},  // 35
    {MTH_SETPROP, "setprop", false},
    {MTH_ERASEPROP, "eraseprop", false},
    {MTH_PROPNAMES, "propnames", false},
    {MTH_ISMEMBER, "ismember", false},        // guild
    {MTH_ISALLYGUILD, "isallyguild", false},  // 40
    {MTH_ISENEMYGUILD, "isenemyguild", false},
    {MTH_ADDMEMBER, "addmember", false},
    {MTH_ADDALLYGUILD, "addallyguild", false},
    {MTH_ADDENEMYGUILD, "addenemyguild", false},
    {MTH_REMOVEMEMBER, "removemember", false},  // 45
    {MTH_REMOVEALLYGUILD, "removeallyguild", false},
    {MTH_REMOVEENEMYGUILD, "removeenemyguild", false},
    {MTH_SIZE, "size", false},  // ARRAY
    {MTH_ERASE, "erase", false},
    {MTH_INSERT, "insert", false},  // 50
    {MTH_SHRINK, "shrink", false},
    {MTH_APPEND, "append", false},
    {MTH_REVERSE, "reverse", false},
    {MTH_SORT, "sort", false},      // dict
    {MTH_EXISTS, "exists", false},  // 55
    {MTH_KEYS, "keys", false},
    {MTH_SENDPACKET, "sendpacket", false},  // packet
    {MTH_SENDAREAPACKET, "sendareapacket", false},
    {MTH_GETINT8, "getint8", false},
    {MTH_GETINT16, "getint16", false},  // 60
    {MTH_GETINT32, "getint32", false},
    {MTH_SETINT8, "setint8", false},
    {MTH_SETINT16, "setint16", false},
    {MTH_SETINT32, "setint32", false},
    {MTH_GETSTRING, "getstring", false},  // 65
    {MTH_GETUNICODESTRING, "getunicodestring", false},
    {MTH_SETSTRING, "setstring", false},
    {MTH_SETUNICODESTRING, "setunicodestring", false},
    {MTH_GETSIZE, "getsize", false},
    {MTH_SETSIZE, "setsize", false},              // 70
    {MTH_CREATEELEMENT, "createelement", false},  // datastore
    {MTH_FINDELEMENT, "findelement", false},
    {MTH_DELETEELEMENT, "deleteelement", false},
    {MTH_SENDEVENT, "sendevent", false},  // script
    {MTH_KILL, "kill", false},            // 75
    {MTH_LOADSYMBOLS, "loadsymbols", false},
    {MTH_SET_UO_EXPANSION, "set_uo_expansion", false},
    {MTH_CLEAR_EVENT_QUEUE, "clear_event_queue", false},
    {MTH_ADD_COMPONENT, "add_component", false},
    {MTH_ERASE_COMPONENT, "erase_component", false},  // 80
    {MTH_DELETE, "delete", false},
    {MTH_SPLIT, "split", false},
    {MTH_MOVE_CHAR, "move_char", false},
    {MTH_GETINT16FLIPPED, "getint16flipped", false},
    {MTH_GETINT32FLIPPED, "getint32flipped", false},  // 85
    {MTH_SETINT16FLIPPED, "setint16flipped", false},
    {MTH_SETINT32FLIPPED, "setint32flipped", false},
    {MTH_GETCORPSE, "getcorpse", false},
    {MTH_SETDEFAULTCMDLEVEL, "setdefaultcmdlevel", false},
    {MTH_PRIVILEGES, "privileges", false},  // 90
    {MTH_GETUNICODESTRINGFLIPPED, "getunicodestringflipped", false},
    {MTH_SETUNICODESTRINGFLIPPED, "setunicodestringflipped", false},
    {MTH_ADD_CHARACTER, "addcharacter", false},
    {MTH_SET_SWINGTIMER, "setswingtimer", false},
    {MTH_ATTACK_ONCE, "attack_once", false},  // 95
    {MTH_SETFACING, "setfacing", false},
    {MTH_COMPAREVERSION, "compareversion", false},
    {MTH_SETLEADER, "setleader", false},
    {MTH_ADDCANDIDATE, "addcandidate", false},
    {MTH_REMOVECANDIDATE, "removecandidate", false},  // 100
    {MTH_RANDOMENTRY, "randomentry", false},
    {MTH_SEEK, "seek", false},
    {MTH_PEEK, "peek", false},
    {MTH_TELL, "tell", false},
    {MTH_FLUSH, "flush", false},  // 105
    {MTH_GETSINT8, "getsint8", false},
    {MTH_GETSINT16, "getsint16", false},
    {MTH_GETSINT32, "getsint32", false},
    {MTH_SETSINT8, "setsint8", false},
    {MTH_SETSINT16, "setsint16", false},  // 110
    {MTH_SETSINT32, "setsint32", false},
    {MTH_SETAGGRESSORTO, "setaggressorto", false},
    {MTH_SETLAWFULLYDAMAGEDTO, "setlawfullydamagedto", false},
    {MTH_CLEARAGGRESSORTO, "clearaggressorto", false},
    {MTH_CLEARLAWFULLYDAMAGEDTO, "clearlawfullydamagedto", false},  // 115
    {MTH_HASSPELL, "hasspell", false},
    {MTH_SPELLS, "spells", false},
    {MTH_REMOVESPELL, "removespell", false},
    {MTH_ADDSPELL, "addspell", false},
    {MTH_DEAF, "deaf", false},  // 120
    {MTH_SETSEASON, "setseason", false},
    {MTH_NEXTSIBLING, "nextxmlsibling", false},
    {MTH_FIRSTCHILD, "firstxmlchild", false},
    {MTH_SAVEXML, "savexml", false},
    {MTH_APPENDNODE, "appendxmlnode", false},  // 125
    {MTH_SETDECLARATION, "setxmldeclaration", false},
    {MTH_SETATTRIBUTE, "setxmlattribute", false},
    {MTH_REMOVEATTRIBUTE, "removexmlattribute", false},
    {MTH_REMOVENODE, "removexmlnode", false},
    {MTH_APPENDTEXT, "appendxmltext", false},  // 130
    {MTH_XMLTOSTRING, "xmltostring", false},
    {MTH_APPENDXMLCOMMENT, "appendxmlcomment", false},
    {MTH_ADD_HOUSE_PART, "addhousepart", false},
    {MTH_ERASE_HOUSE_PART, "erasehousepart", false},
    {MTH_ACCEPT_COMMIT, "acceptcommit", false},  // 135
    {MTH_SPLITSTACK_AT, "splitstackat", false},
    {MTH_SPLITSTACK_INTO, "splitstackinto", false},
    {MTH_CANCEL_EDITING, "cancelediting", false},
    {MTH_CLONENODE, "clonenode", false},
    {MTH_HAS_EXISTING_STACK, "hasexistingstack", false},  // 140
    {MTH_LENGTH, "length", false},
    {MTH_JOIN, "join", false},
    {MTH_FIND, "find", false},
    {MTH_UPPER, "upper", false},
    {MTH_LOWER, "lower", false},  // 145
    {MTH_FORMAT, "format", false},
    {MTH_DISABLE_SKILLS_FOR, "disableskillsfor", false},
    {MTH_CYCLE, "cycle", false},
    {MTH_ADD_BUFF, "addbuff", false},
    {MTH_DEL_BUFF, "delbuff", false},  // 150
    {MTH_CLEAR_BUFFS, "clearbuffs", false},
    {MTH_CALL, "call", false},
    {MTH_SORTEDINSERT, "sorted_insert", false},
};
int n_objmethods = sizeof object_methods / sizeof object_methods[0];
ObjMethod* getKnownObjMethod( const char* token )
{
  // cache needs to hold a pointer to the original structure! eprog_read sets the override member
  static auto cache = []() -> std::unordered_map<std::string, ObjMethod*> {
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

void testparserdefinitions()
{
  for ( int i = 0; i < n_objmethods; i++ )
  {
    if ( object_methods[i].id != i )
    {
      INFO_PRINT << "ERROR: Object Method definition of " << object_methods[i].code
                 << " has an invalid index!\n";
    }
    auto c = reinterpret_cast<unsigned char*>( object_methods[i].code );
    while ( *c )
    {
      if ( *c != tolower( *c ) )
      {
        INFO_PRINT << "ERROR: Object Method definition of " << object_methods[i].code
                   << " is not lowercase!\n";
        break;
      }
      ++c;
    }
  }
  for ( int i = 0; i < n_objmembers; i++ )
  {
    if ( object_members[i].id != i )
    {
      INFO_PRINT << "ERROR: Object Member definition of " << object_members[i].code
                 << " has an invalid index!\n";
    }
    auto c = reinterpret_cast<unsigned char*>( object_members[i].code );
    while ( *c )
    {
      if ( *c != tolower( *c ) )
      {
        INFO_PRINT << "ERROR: Object Member definition of " << object_members[i].code
                   << " is not lowercase!\n";
        break;
      }
      ++c;
    }
  }
}
Clib::UnitTest testparserdefinitions_obj( testparserdefinitions );

void matchOperators( Operator* oplist, int n_ops, char* buf, int* nPartial,
                     Operator** pTotalMatchOperator )
{
  // all operators are 1 or 2 characters.
  // they also don't have case.
  int lenbuf = buf[1] ? 2 : 1;

  *nPartial = 0;
  *pTotalMatchOperator = nullptr;

  if ( lenbuf == 1 )
  {
    Operator* op = &oplist[0];
    for ( int i = 0; i < n_ops; ++i, ++op )
    {
      if ( op->code[0] == buf[0] )
      {
        if ( op->code[1] == '\0' )
        {
          ( *pTotalMatchOperator ) = op;
          if ( !op->ambig )
            return;
        }
        else
        {
          ( *nPartial )++;
        }
      }
    }
  }
  else  // lenbuf == 2
  {
    Operator* op = &oplist[0];
    for ( int i = 0; i < n_ops; ++i, ++op )
    {
      if ( op->code[0] == buf[0] && op->code[1] == buf[1] )
      {
        ( *pTotalMatchOperator ) = op;
        return;
      }
    }
  }
}

/*
void matchOperators(char *buf, int *nPartial, Operator** ppMatch)
{
matchOperators(binary_operators, n_operators,
buf, nPartial, ppMatch);
}
void matchUnaryOperators(char *buf, int *nPartial, Operator** ppMatch)
{
matchOperators(unary_operators, n_unary,
buf, nPartial, ppMatch);
}
*/

// FIXME this really should be a word, type, and id
// (or better yet, eliminate type altogether.)
// reserved words first get read out as literals, then
// are recognized to be reserved words.
// the "and", "or", and "not" operators won't be recognized
// right, for the moment.

typedef struct
{
  const char* word;
  BTokenId id;
  BTokenType type;
  Precedence precedence;
  bool deprecated;
} ReservedWord;


ReservedWord reserved_words[] = {
    {"if", RSV_ST_IF, TYP_RESERVED, PREC_TERMINATOR, false},
    {"then", RSV_THEN, TYP_RESERVED, PREC_TERMINATOR, false},
    {"elseif", RSV_ELSEIF, TYP_RESERVED, PREC_TERMINATOR, false},
    {"endif", RSV_ENDIF, TYP_RESERVED, PREC_TERMINATOR, false},
    {"else", RSV_ELSE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"_OptionBracketed", RSV_OPTION_BRACKETED, TYP_RESERVED, PREC_TERMINATOR, false},

    {"goto", RSV_GOTO, TYP_RESERVED, PREC_TERMINATOR, false},
    {"gosub", RSV_GOSUB, TYP_RESERVED, PREC_TERMINATOR, false},
    {"return", RSV_RETURN, TYP_RESERVED, PREC_TERMINATOR, false},

    //  { "global",    RSV_GLOBAL,   TYP_RESERVED, PREC_DEPRECATED, true }, // internal only
    //  { "local",    RSV_LOCAL,    TYP_RESERVED, PREC_DEPRECATED, true }, // internal only
    {"const", RSV_CONST, TYP_RESERVED, PREC_TERMINATOR, false},
    {"var", RSV_VAR, TYP_RESERVED, PREC_TERMINATOR, false},

    //  { "begin",    RSV_BEGIN,    TYP_RESERVED, PREC_DEPRECATED, true }, // deprecated
    //  { "end",    RSV_ENDB,     TYP_RESERVED, PREC_DEPRECATED, true }, // deprecated

    {"do", RSV_DO, TYP_RESERVED, PREC_TERMINATOR, false},
    {"dowhile", RSV_DOWHILE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"while", RSV_WHILE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"endwhile", RSV_ENDWHILE, TYP_RESERVED, PREC_TERMINATOR, false},

    {"exit", RSV_EXIT, TYP_RESERVED, PREC_TERMINATOR, false},

    {"declare", RSV_DECLARE, TYP_RESERVED, PREC_TERMINATOR, false},

    {"function", RSV_FUNCTION, TYP_RESERVED, PREC_TERMINATOR, false},
    {"endfunction", RSV_ENDFUNCTION, TYP_RESERVED, PREC_TERMINATOR, false},
    {"exported", RSV_EXPORTED, TYP_RESERVED, PREC_TERMINATOR, false},

    {"use", RSV_USE_MODULE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"include", RSV_INCLUDE_FILE, TYP_RESERVED, PREC_TERMINATOR, false},

    {"break", RSV_BREAK, TYP_RESERVED, PREC_TERMINATOR, false},
    {"continue", RSV_CONTINUE, TYP_RESERVED, PREC_TERMINATOR, false},

    {"for", RSV_FOR, TYP_RESERVED, PREC_TERMINATOR, false},
    {"endfor", RSV_ENDFOR, TYP_RESERVED, PREC_TERMINATOR, false},
    {"to", RSV_TO, TYP_RESERVED, PREC_TERMINATOR, false},
    {"next", RSV_NEXT, TYP_RESERVED, PREC_TERMINATOR, false},

    {"foreach", RSV_FOREACH, TYP_RESERVED, PREC_TERMINATOR, false},
    {"endforeach", RSV_ENDFOREACH, TYP_RESERVED, PREC_TERMINATOR, false},

    {"repeat", RSV_REPEAT, TYP_RESERVED, PREC_TERMINATOR, false},
    {"until", RSV_UNTIL, TYP_RESERVED, PREC_TERMINATOR, false},

    {"program", RSV_PROGRAM, TYP_RESERVED, PREC_TERMINATOR, false},
    {"endprogram", RSV_ENDPROGRAM, TYP_RESERVED, PREC_TERMINATOR, false},

    {"case", RSV_SWITCH, TYP_RESERVED, PREC_TERMINATOR, false},
    // { "case",     RSV_CASE,     TYP_RESERVED, PREC_TERMINATOR, false },
    {"default", RSV_DEFAULT, TYP_RESERVED, PREC_TERMINATOR, false},
    {"endcase", RSV_ENDSWITCH, TYP_RESERVED, PREC_TERMINATOR, false},

    {"enum", RSV_ENUM, TYP_RESERVED, PREC_TERMINATOR, false},
    {"endenum", RSV_ENDENUM, TYP_RESERVED, PREC_TERMINATOR, false},

    {"downto", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"step", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"reference", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"out", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"inout", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    // { "ByRef",    RSV_FUTURE,   TYP_RESERVED, PREC_TERMINATOR, false },
    {"ByVal", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},

    {"string", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"long", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"integer", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"unsigned", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"signed", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"real", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"float", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"double", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"as", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"is", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},

    {"and", TOK_AND, TYP_OPERATOR, PREC_LOGAND, false},
    {"or", TOK_OR, TYP_OPERATOR, PREC_LOGOR, false},
    {"not", TOK_LOG_NOT, TYP_UNARY_OPERATOR, PREC_UNARY_OPS, false},
    {"byref", TOK_REFTO, TYP_RESERVED, PREC_TERMINATOR, false},  // UNARY_OPERATOR, 12 },
    {"unused", TOK_UNUSED, TYP_RESERVED, PREC_TERMINATOR, false},
    {"error", TOK_ERROR, TYP_OPERAND, PREC_TERMINATOR, false},
    {"hash", RSV_FUTURE, TYP_RESERVED, PREC_TERMINATOR, false},
    {"dictionary", TOK_DICTIONARY, TYP_OPERAND, PREC_TERMINATOR, false},
    {"struct", TOK_STRUCT, TYP_OPERAND, PREC_TERMINATOR, false},
    {"array", TOK_ARRAY, TYP_OPERAND, PREC_TERMINATOR, false},
    {"stack", TOK_STACK, TYP_OPERAND, PREC_TERMINATOR, false},
    {"in", TOK_IN, TYP_OPERATOR, PREC_EQUALTO, false}
    //  { "bitand",   TOK_BITAND,   TYP_OPERATOR, PREC_BITAND },
    //  { "bitxor",   TOK_BITXOR,   TYP_OPERATOR, PREC_BITXOR },
    //  { "bitor",    TOK_BITOR,    TYP_OPERATOR, PREC_BITOR }
    /*  "/""*",      RSV_COMMENT_START,
        "*""/",      RSV_COMMENT_END,
        "/""/",      RSV_COMMENT_TO_EOL,
        "--",      RSV_COMMENT_TO_EOL
        */
};
unsigned n_reserved = sizeof reserved_words / sizeof reserved_words[0];

typedef std::map<std::string, ReservedWord*, Clib::ci_cmp_pred> ReservedWords;
ReservedWords reservedWordsByName;
static void init_tables()
{
  static std::once_flag flag;
  std::call_once( flag, []() {
    for ( unsigned i = 0; i < n_reserved; ++i )
    {
      reservedWordsByName[reserved_words[i].word] = &reserved_words[i];
    }
  } );
}

void Parser::write_words( std::ostream& os )
{
  os << "Reserved:" << std::endl;
  for ( unsigned i = 0; i < n_reserved; ++i )
  {
    os << reserved_words[i].word << ( reserved_words[i].deprecated ? " (deprecated)" : "" )
       << std::endl;
  }
  os << std::endl;
  os << "Binary:" << std::endl;
  for ( int i = 0; i < n_operators; ++i )
  {
    os << binary_operators[i].code << ( binary_operators[i].deprecated ? " (deprecated)" : "" )
       << std::endl;
  }
  os << std::endl;
  os << "Unary:" << std::endl;
  for ( int i = 0; i < n_unary; ++i )
  {
    os << unary_operators[i].code << ( unary_operators[i].deprecated ? " (deprecated)" : "" )
       << std::endl;
  }
  os << std::endl;
  os << "Methodlist:" << std::endl;
  for ( int i = 0; i < n_objmethods; i++ )
  {
    os << object_methods[i].id << " " << object_methods[i].code << std::endl;
  }
  os << std::endl;
  os << "Memberlist:" << std::endl;
  for ( int i = 0; i < n_objmembers; i++ )
  {
    os << object_members[i].id << " " << object_members[i].code << std::endl;
  }
}

#if 0
  void matchReservedWords(char *buf,
               int *nPartial,
               int *nTotal)
  {
    int lenbuf = strlen(buf);
    assert(nPartial && nTotal);
    *nPartial = 0;
    *nTotal = 0;
    for(int i = 0; i < n_reserved; i++)
    {
    if (strnicmp(reserved_words[i].word, buf, lenbuf)==0)
      (*nPartial)++;
    if (stricmp(reserved_words[i].word, buf)==0)
      (*nTotal)++;
    }
  }
#endif

/*
  WTF is going on in this function?  It seems like it waits for a match, followed
  by a nonmatch?  eh?

  What does this mean for variables like "IfDone" etc?
  */

#if 0
  int Parser::tryReservedWord(Token& tok, char *t, char **s)
  {
    char opbuf[10];
    int bufp = 0;
    int thisMatchPartial, thisMatchTotal;
    int lastMatchTotal = 0;

    while (t && *t) {
    /* let's try to match it as we go. */
    if (bufp==10) { err = PERR_BADTOKEN; return -1; }
    opbuf[bufp++] = *t++;
    opbuf[bufp] = '\0';
    matchReservedWords(opbuf, &thisMatchPartial, &thisMatchTotal);
    if (!thisMatchPartial) { /* can't match a bloody thing! */
      switch(lastMatchTotal) {
      case 0:
        return 0; // this just wasn't a reserved word..
      case 1: // this is the only way it will work..
        // here, we don't match now but if we don't count
        // this character, it was a unique match.
        opbuf[bufp-1] = '\0';
        tok.nulStr();
        recognize_reserved_word(tok, opbuf);
        *s = t-1;
        return 1;
      case 2: // here, with this character there is no match
        // but before there were multiple matches.
        // really shouldn't happen.
        err = PERR_BADOPER;
        return -1;
      }
    } else { /* this partially matches.. */
      // "Remember....."
      lastMatchTotal = thisMatchTotal;
    }
    }

    if (thisMatchTotal == 1) {
    tok.nulStr();
    recognize_reserved_word( tok, opbuf );
    *s = t;
    return 1;
    }

    return 0; // didn't find one!
  }
#endif

/**
 * Tries to read a an operator from context
 *
 * @param tok Token&: The token to store the found literal into
 * @param opList: The list of possible operators to look for, as Operator[]
 * @param n_ops: Number of operators in the list
 * @param t: todo
 * @param s: todo
 * @param opbuf: todo
 * @return 0 when no matching text is found, 1 on success, -1 on error (also sets err)
 */
int Parser::tryOperator( Token& tok, const char* t, const char** s, Operator* opList, int n_ops,
                         char* opbuf )
{
  int bufp = 0;
  int thisMatchPartial;
  Operator* pLastMatch = nullptr;
  Operator* pMatch = nullptr;

  while ( t && *t )
  {
    //    if (strchr(operator_brk, *t)) mustBeOperator = 1;

    /* let's try to match it as we go. */
    if ( bufp == 4 )
    {
      err = PERR_BADTOKEN;
      return -1;
    }
    opbuf[bufp++] = *t++;
    opbuf[bufp] = '\0';
    matchOperators( opList, n_ops, opbuf, &thisMatchPartial, &pMatch );
    if ( !thisMatchPartial )
    {
      if ( pMatch )
      {  // no partial matches, but a total match.
        break;
      }
      else if ( !pLastMatch )
      {
        // no total matches, no partial matches.
        return 0;
      }
      else  // (pLastMatch)
      {
        // had a match before, but no partials this time.
        break;
      }
    }
    else
    { /* this partially matches.. */
      // "Remember....."
      if ( pMatch )
      {
        pLastMatch = pMatch;
      }
    }
  }

  if ( pMatch == nullptr )
    pMatch = pLastMatch;

  if ( pMatch )
  {
    *s += strlen( pMatch->code );

    tok.module = Mod_Basic;
    tok.id = pMatch->id;
    tok.type = pMatch->type;
    tok.precedence = pMatch->precedence;
    tok.deprecated = pMatch->deprecated;

    tok.setStr( pMatch->code );
    return 1;
  }

  return 0;  // didn't find one!
}

/**
 * Tries to read a binary operator from context
 *
 * @param tok Token&: The token to store the found literal into
 * @param ctx CompilerContext&: The context to look into
 * @return 0 when no matching text is found, 1 on success, -1 on error (also sets err)
 */
int Parser::tryBinaryOperator( Token& tok, CompilerContext& ctx )
{
  char opbuf[10];
  return tryOperator( tok, ctx.s, &ctx.s, binary_operators, n_operators, opbuf );
}

/**
 * Tries to read an unary operator from context
 *
 * @param tok Token&: The token to store the found literal into
 * @param ctx CompilerContext&: The context to look into
 * @return 0 when no matching text is found, 1 on success, -1 on error (also sets err)
 */
int Parser::tryUnaryOperator( Token& tok, CompilerContext& ctx )
{
  char opbuf[10];
  return tryOperator( tok, ctx.s, &ctx.s, unary_operators, n_unary, opbuf );
}

/**
 * Tries to read a numeric value from context
 *
 * @param tok Token&: The token to store the found literal into
 * @param ctx CompilerContext&: The context to look into
 * @return 0 when no matching text is found, 1 on success, -1 on error (also sets err)
 */
int Parser::tryNumeric( Token& tok, CompilerContext& ctx )
{
  if ( isdigit( ctx.s[0] ) || ctx.s[0] == '.' )
  {
    char *endptr, *endptr2;
    int l = strtol( ctx.s, &endptr, 0 );
    double d = strtod( ctx.s, &endptr2 );

    // 2015-01-21 Bodom: weird trick to remove an unwanted feature from Microsoft compiler
    //                   interpreting 'd' as 'e' (exponent), but 'd' in UO means dice,
    //                   leading to confusion
    // TODO: The best solution would be to reimplement the int/double parsing
    if ( !( ctx.s[0] == '0' && ctx.s[1] && ( ctx.s[1] == 'x' || ctx.s[1] == 'X' ) ) )
    {
      // This is not hex, so no 'd' can be valid
      for ( const char* i = ctx.s; i < endptr2; i++ )
      {
        if ( *i == 'd' || *i == 'D' )
        {
          // A 'd' has been eaten, bug could have occurred:
          // re-perform parsing on a cleaned version of the string
          size_t safelen = i - ctx.s + 1;
          std::unique_ptr<char[]> safeptr( new char[safelen]() );
          strncpy( safeptr.get(), ctx.s, safelen - 1 );
          d = strtod( safeptr.get(), &endptr2 );
          size_t newlen = endptr2 - safeptr.get();
          endptr2 = const_cast<char*>( ctx.s + newlen );
          break;
        }
      }
    }

    tok.type = TYP_OPERAND;
    if ( endptr >= endptr2 )
    {  // long got more out of it, we'll go with that
      tok.id = TOK_LONG;
      tok.lval = l;
      ctx.s = endptr;
      return 1;
    }
    else
    {
      tok.id = TOK_DOUBLE;
      tok.dval = d;
      ctx.s = endptr2;
      return 1;
    }
  }
  return 0;  // not numeric
}

/**
 * Tries to read a literal (string/variable name) from context
 *
 * @param tok Token&: The token to store the found literal into
 * @param ctx CompilerContext&: The context to look into
 * @return 0 when no matching text is found, 1 on success, -1 on error (also sets err)
 */
int Parser::tryLiteral( Token& tok, CompilerContext& ctx )
{
  if ( ctx.s[0] == '\"' )
  {
    const char* end = &ctx.s[1];
    std::string lit;
    bool escnext = false;  // true when waiting for 2nd char in an escape sequence
    u8 hexnext = 0;        // tells how many more chars in a \xNN escape sequence
    char hexstr[3];        // will contain the \x escape chars to be processed
    memset( hexstr, 0, 3 );

    for ( ;; )
    {
      if ( !*end )
      {
        err = PERR_UNTERMSTRING;
        return -1;
      }

      passert_always_r( !( escnext && hexnext ),
                        "Bug in the compiler. Please report this on the forums." );

      if ( escnext )
      {
        // waiting for 2nd character after a backslash
        escnext = false;
        if ( *end == 'n' )
          lit += '\n';
        else if ( *end == 't' )
          lit += '\t';
        else if ( *end == 'x' )
          hexnext = 2;
        else
          lit += *end;
      }
      else if ( hexnext )
      {
        // waiting for next (two) chars in hex escape sequence (eg. \xFF)
        hexstr[2 - hexnext] = *end;
        if ( !--hexnext )
        {
          char* endptr;
          char ord = static_cast<char>( strtol( hexstr, &endptr, 16 ) );
          if ( *endptr != '\0' )
          {
            err = PERR_INVESCAPE;
            return -1;
          }
          lit += ord;
        }
      }
      else
      {
        if ( *end == '\\' )
          escnext = true;
        else if ( *end == '\"' )
          break;
        else
          lit += *end;
      }
      ++end;
    }
    /*
            char *end = strchr(&ctx.s[1], '\"');
            if (!end)
            {
            err = PERR_UNTERMSTRING;
            return -1;
            }
            */
    // int len = end - ctx.s;   //   "abd" len = 5-1 = 4
    if ( !Clib::isValidUnicode( lit ) )
    {
      if ( !ctx.silence_unicode_warnings )
      {
        compiler_warning( &ctx, "Warning: invalid unicode character detected. Assuming ISO8859\n" );
      }
      Clib::sanitizeUnicodeWithIso( &lit );
    }
    tok.id = TOK_STRING;  // this is a misnomer I think!
    tok.type = TYP_OPERAND;
    tok.copyStr( lit.c_str() );

    ctx.s = end + 1;  // skip past the ending delimiter
    return 1;
  }
  else if ( isalpha( ctx.s[0] ) || ctx.s[0] == '_' )
  {  // we have a variable/label/verb.
    const char* end = ctx.s;
    while ( *end && !isspace( *end ) && strchr( ident_allowed, *end ) )
    {
      ++end;
    }
    // Catch identifiers of the form module::name
    if ( end[0] == ':' && end[1] == ':' )
    {
      end += 2;
      while ( *end && !isspace( *end ) && strchr( ident_allowed, *end ) )
      {
        ++end;
      }
    }

    int len = static_cast<int>( end - ctx.s + 1 );  //   "abcd"

    tok.copyStr( ctx.s, len - 1 );

    tok.id = TOK_IDENT;
    tok.type = TYP_OPERAND;

    ctx.s = end;
    return 1;
  }
  return 0;
}

int Parser::recognize_binary( Token& tok, const char* buf, const char** /*s*/ )
{
  for ( int i = 0; i < n_operators; i++ )
  {
    if ( stricmp( buf, binary_operators[i].code ) == 0 )
    {
      tok.module = Mod_Basic;
      tok.id = binary_operators[i].id;
      tok.type = binary_operators[i].type;
      tok.precedence = binary_operators[i].precedence;
      /*
                  if (tok.type == TYP_OPERATOR &&
                  (tok.id == TOK_ADDMEMBER ||
                  tok.id == TOK_DELMEMBER ||
                  tok.id == TOK_CHKMEMBER ||
                  tok.id == TOK_MEMBER) )
                  {
                  int res;
                  Token tk2;
                  res = getToken( s, tk2 );
                  if (res < 0) return res;
                  if (tk2.type != TYP_OPERAND || tk2.id != TOK_IDENT)
                  return -1;
                  tok.copyStr( tk2.tokval() );
                  }
                  else
                  {
                  */
      tok.setStr( binary_operators[i].code );
      // tok.nulStr();
      /*
                  }
                  */

      return 1;
    }
  }
  return 0;
}

int Parser::recognize_unary( Token& tok, const char* buf )
{
  for ( int i = 0; i < n_unary; i++ )
  {
    if ( stricmp( buf, unary_operators[i].code ) == 0 )
    {
      tok.module = Mod_Basic;
      tok.id = unary_operators[i].id;
      tok.type = unary_operators[i].type;
      tok.precedence = unary_operators[i].precedence;
      tok.setStr( unary_operators[i].code );

      return 1;
    }
  }
  return 0;
}


int Parser::recognize( Token& tok, const char* buf, const char** s )
{
  if ( recognize_binary( tok, buf, s ) )
    return 1;
  return recognize_unary( tok, buf );
}


bool Parser::recognize_reserved_word( Token& tok, const char* buf )
{
  if ( tok.id != TOK_IDENT )
    return false;

  auto itr = reservedWordsByName.find( buf );
  if ( itr != reservedWordsByName.end() )
  {
    ReservedWord* rv = ( *itr ).second;

    tok.module = Mod_Basic;
    tok.id = rv->id;
    tok.type = rv->type;
    tok.precedence = rv->precedence;
    tok.deprecated = rv->deprecated;
    tok.setStr( rv->word );
    return true;
  }
  return false;
}


/*
int isOperator(Token& token)
{
for(int i=0;i < n_operators; i++) {
if (stricmp(binary_operators[i].code, token )==0) {
token.setStr(binary_operators[i].code);
token.id = binary_operators[i].id;
token.type = binary_operators[i].type;
return 1;
}
}
return 0;
}
*/

Precedence find_precedence( Token& token )
{
  return static_cast<Precedence>( token.precedence );
}

void Parser::reinit( Expression& ex )
{
  // qCA.clear();
  while ( !ex.CA.empty() )
  {
    delete ex.CA.front();
    ex.CA.pop();
  }

  // TX.clear()
  while ( !ex.TX.empty() )
  {
    delete ex.TX.top();
    ex.TX.pop();
  }


  err = PERR_NONE;
  ext_err[0] = '\0';
}

/**
 * Reads next token from given context
 *
 * what is a token? a set of homogeneous characters
 * a Label:
 * begins with [A-Za-z_], followed by [A-Za-z0-9]
 *
 * A character literal:
 * begins with '"', ends with '"'. anything goes in between.
 *
 * a Number:
 * begins with [0-9] (note: not plus or minus, these get eaten as unary ops)
 * can be either a float (stored as double), or a long.
 * 0xABC is hex, which is read okay.
 * So is 0.5e+17. I let strtod do most of the work.
 * basically whichever of strtod or strtol can do more with it
 * (endptr arg is greater on exit), that's what i decide that it is.
 *
 * an operator:
 * any collection of the operator characters
 * [ ( ) * / + - < = > ,] not separated by whitespace, digits, or alphas
 * note a collection of more than one is considered a SINGLE operator.
 * So if you put 6*-7, *- is the operator. nasty I know, but
 * what am I supposed to do? (Maximal munch, is what is actually done!)
 *
 * @param tok Token&: The token to store the found literal into
 * @param ctx CompilerContext&: The context to look into
 * @param expr unused
 * @return 0 when no matching text is found, 1 on success, -1 on error (also sets err)
 */
int Parser::getToken( CompilerContext& ctx, Token& tok, Expression* /* expr not used */ )
{
  int hit = 0;

  int res = ctx.skipcomments();
  if ( res )
    return res;

  tok.dbg_filenum = ctx.dbg_filenum;
  tok.dbg_linenum = ctx.line;

  if ( ctx.s[0] == ';' )
  {
    tok.module = Mod_Basic;
    tok.id = TOK_SEMICOLON;
    tok.type = TYP_DELIMITER;
    tok.setStr( ";" );
    ctx.s++;
    return 0;
  }

  hit = tryLiteral( tok, ctx );
  if ( hit == -1 )
  {
    return -1;
  }
  else if ( hit )
  {
    recognize_reserved_word( tok, tok.tokval() );
    return 0;
  }

  /*
      hit = tryReservedWord(tok, t, s);
      if (hit==-1)  return -1;
      else if (hit) return 0;
      */

  hit = tryBinaryOperator( tok, ctx );
  if ( hit == -1 )
    return -1;
  else if ( hit )
    return 0;

  hit = tryUnaryOperator( tok, ctx );
  if ( hit == -1 )
    return -1;
  else if ( hit )
    return 0;

  // label:
  // A:=4;


  hit = tryNumeric( tok, ctx );
  if ( hit == -1 )
    return -1;
  else if ( hit )
    return 0;

  if ( ctx.s[0] == ':' )
  {
    ++ctx.s;
    tok.id = RSV_COLON;
    tok.type = TYP_RESERVED;
    tok.setStr( ":" );
    return 0;
  }

  compiler_error( "Your syntax frightens and confuses me.\n" );
  err = PERR_WAAH;
  return -1;
}

/**
 * Reads next token from given context, but without modifying the context
 *
 * @see Parser::getToken( CompilerContext& ctx, Token& tok )
 */
int Parser::peekToken( const CompilerContext& ctx, Token& token, Expression* expr )
{
  CompilerContext tctx( ctx );
  tctx.silence_unicode_warnings = true;
  return getToken( tctx, token, expr );
}

/* Parser::parseToken deleted. */
/* not used? ens 12/10/1998
int Parser::IP(Expression& expr, char *s)
{
reinit(expr);

Token *ptoken;
if (!quiet) cout << "Parsing \"" << s << '\"' << endl;

expr.TX.push(new Token); // push terminator token

ptoken = new Token;
while (getToken(&s, *ptoken)==0)
{
parseToken(expr, ptoken);
ptoken = new Token;
}
parseToken(expr, ptoken);

return 0;
}
*/


int SmartParser::isOkay( const Token& token, const Token& last_token )
{
  BTokenType last_type = last_token.type;
  BTokenType this_type = token.type;
  if ( !quiet )
    INFO_PRINT << "isOkay(" << this_type << "," << last_type << ")\n";
  if ( last_type == TYP_FUNC || last_type == TYP_USERFUNC || last_type == TYP_METHOD ||
       last_type == TYP_FUNCREF )
    last_type = TYP_OPERAND;
  if ( this_type == TYP_FUNC || this_type == TYP_USERFUNC || this_type == TYP_METHOD ||
       this_type == TYP_FUNCREF )
    this_type = TYP_OPERAND;
  if ( token.id == TOK_LBRACE )  // an array declared somewhere out there
    this_type = TYP_OPERAND;

  if ( last_type == TYP_UNARY_PLACEHOLDER || this_type == TYP_UNARY_PLACEHOLDER )
    return 0;  // always invalid
  if ( token.id == TOK_UNPLUSPLUS_POST ||
       token.id == TOK_UNMINUSMINUS_POST )  // valid when other direction is valid
    std::swap( this_type, last_type );
  if ( last_token.id == TOK_UNPLUSPLUS_POST ||
       last_token.id == TOK_UNMINUSMINUS_POST )  // behaves like the operand before
    last_type = TYP_OPERAND;
  if ( last_type > TYP_TESTMAX )
    return 1;  // assumed okay
  if ( this_type > TYP_TESTMAX )
    return 1;                                  // assumed okay
  return allowed_table[last_type][this_type];  // maybe okay
}

/*
int SmartParser::isFunc(Token& token, Verb **v)
{
if (token.id != TOK_IDENT) return 0; // ain't no verb..
// note that this catches string literals.
assert(token.tokval());

if (isInTable(parser_verbs, n_parser_verbs, token.tokval(), v))
{
token.id = (*v)->id;
token.lval = (*v)->narg;
token.type = TYP_FUNC;
token.module = Mod_Basic;
return 1;
}
return 0;
}
*/

/**
 * Like Parser::tryLiteral, with extra elements supported
 *
 * Labels.
 * A label is an ident operand, followed by a colon, followed by
 * either whitespace or end-of-file.
 * Note, this definition just happens to exclude ':=' and '::',
 * which is important.
 *
 * @see Parser::tryLiteral( Token& tok, CompilerContext& ctx )
 */
int SmartParser::tryLiteral( Token& tok, CompilerContext& ctx )
{
  int res;
  res = Parser::tryLiteral( tok, ctx );
  if ( res == 1 && tok.id == TOK_IDENT )
  {  // check for "label:"

    // whitespace used to be skipped here.
    //   while (*t && isspace(*t)) t++;
    if ( !ctx.s[0] )  // ident EOF can't be a label
    {
      return 1;
    }

// this might be a nice place to look for module::function, too.
#if 0
    if (t[0] == ':' && t[1] == ':')
    {
      t += 2;
      *s = t;
      Token tok2;
      int res2 = Parser::tryLiteral( tok2, t, s );
      if (res2 < 0) return res2;
      if (res2 == 0)
      return -1;
      // append '::{tok2 tokval}' to tok.tokval
      // (easier when/if token uses string)
    }
#endif
    if ( ctx.s[0] == ':' && ( ctx.s[1] == '\0' || isspace( ctx.s[1] ) ) )
    {
      tok.id = CTRL_LABEL;
      tok.type = TYP_LABEL;
      ++ctx.s;
    }
    return 1;
  }
  return res;
}

int SmartParser::parseToken( CompilerContext& ctx, Expression& expr, Token* token )
{
  //  return Parser::parseToken(token);
  if ( !quiet )
  {
    fmt::Writer _tmp;
    _tmp << "parseToken( " << *token << ")\n";
    _tmp << "  CA: ";
    std::queue<Token*> ca( expr.CA );
    while ( !ca.empty() )
    {
      Token* tk = ca.front();
      _tmp << *tk << " ";
      ca.pop();
    }
    _tmp << "\n";
    _tmp << "  TX: ";
    std::stack<Token*> tx( expr.TX );
    while ( !tx.empty() )
    {
      Token* tk = tx.top();
      _tmp << *tk << " ";
      tx.pop();
    }
    INFO_PRINT << _tmp.str() << "\n";
  }

  for ( ;; )
  {
    Token* last = expr.TX.top();
    switch ( token->type )
    {
    case TYP_FUNC:
    case TYP_USERFUNC:
    case TYP_OPERAND:
      expr.CA.push( token );
      return 0;
    case TYP_TERMINATOR:
      switch ( last->type )
      {
      case TYP_TERMINATOR:
        delete token;
        delete expr.TX.top();
        expr.TX.pop();
        return 1;  // all done!
      case TYP_UNARY_OPERATOR:
        expr.CA.push( expr.TX.top() );
        expr.TX.pop();
        break;
      case TYP_OPERATOR:
        expr.CA.push( expr.TX.top() );
        expr.TX.pop();
        break;
      case TYP_LEFTPAREN:
        err = PERR_MISSRPAREN;
        return -1;
      case TYP_LEFTBRACKET:
        err = PERR_MISSRBRACKET;
        return -1;
      default:
        err = PERR_WAAH;
        return -1;
      }
      break;
    case TYP_UNARY_OPERATOR:
    case TYP_OPERATOR:
    case TYP_LEFTBRACKET:
      // these start out at 0, but need to start at one.
      if ( token->type == TYP_LEFTBRACKET )
        token->lval = 1;
      switch ( last->type )
      {
      case TYP_TERMINATOR:
      case TYP_LEFTPAREN:
      case TYP_LEFTBRACKET:
        // possible check CA for array dereference operator here.
        // if one found, remove it, and change this assignment to a "deref and assign"
        expr.TX.push( token );
        return 0;
      case TYP_UNARY_OPERATOR:
      case TYP_OPERATOR:
        int this_prec;
        int last_prec;

        this_prec = find_precedence( *token );
        last_prec = find_precedence( *last );
        if ( this_prec > last_prec )
        {
          expr.TX.push( token );
          return 0;
        }
        else
        {
          expr.CA.push( expr.TX.top() );
          expr.TX.pop();
        }
        break;
      default:
        err = PERR_WAAH;
        return -1;
        break;
      }
      break;
    case TYP_LEFTPAREN:
      expr.TX.push( token );
      return 0;
    case TYP_RIGHTPAREN:
      switch ( last->type )
      {
      case TYP_TERMINATOR:
        err = PERR_UNEXRPAREN;
        return -1;
      case TYP_UNARY_OPERATOR:
      case TYP_OPERATOR:
        expr.CA.push( expr.TX.top() );
        expr.TX.pop();
        break;
      case TYP_LEFTPAREN:
        delete token;
        delete expr.TX.top();
        expr.TX.pop();
        return 0;
      default:
        compiler_error( "Unmatched ')' in expression. (Trying to match against a '", *last, "')\n",
                        ctx, "parseToken(): Not sure what to do.\n", "Token: ", *token, "\n",
                        "Last:  ", *last, "\n" );
        throw std::runtime_error( "Error in parseToken() (1)" );
      }
      break;

    // case TYP_LEFTBRACKET:
    //  expr.TX.push(token);
    // return 0;
    case TYP_RIGHTBRACKET:
      switch ( last->type )
      {
      case TYP_TERMINATOR:
        err = PERR_UNEXRBRACKET;
        return -1;
      case TYP_UNARY_OPERATOR:
      case TYP_OPERATOR:
        expr.CA.push( expr.TX.top() );
        expr.TX.pop();
        break;
      case TYP_LEFTBRACKET:
        Token* ptkn;
        ptkn = new Token( TOK_ARRAY_SUBSCRIPT, TYP_OPERATOR );
        ptkn->lval = last->lval;
        ptkn->dbg_filenum = token->dbg_filenum;
        ptkn->dbg_linenum = token->dbg_linenum;

        if ( ptkn->lval != 1 )
          ptkn->id = INS_MULTISUBSCRIPT;
        expr.CA.push( ptkn );

        delete token;
        delete expr.TX.top();
        expr.TX.pop();
        return 0;
      default:
        compiler_error( "Unmatched ']' in expression. (Trying to match against a '", *last, "')\n",
                        ctx, "parseToken(): Not sure what to do.\n", "Token: ", *token, "\n",
                        "Last:  ", *last, "\n" );
        throw std::runtime_error( "Error in parseToken() (2)" );
      }
      break;

    /* Special Case: two syntaxes are supported for array indexing:
            A[3][4] and A[3,4].
            If this is a comma, and we are working in a left bracket area,
            increase the number of indices (stored in lval of the leftbracket)
            */
    case TYP_SEPARATOR:
      if ( last->type == TYP_LEFTBRACKET )
      {
        ++last->lval;
        // Token *ptkn = new Token( Mod_Basic, TOK_ARRAY_SUBSCRIPT, TYP_OPERATOR );
        // ptkn->lval = last->lval++;
        // expr.CA.push( ptkn );

        delete token;
        return 0;
      }
      else if ( last->type == TYP_UNARY_OPERATOR || last->type == TYP_OPERATOR )
      {
        expr.CA.push( expr.TX.top() );
        expr.TX.pop();
        break;  // try again
      }
      else
      {
        err = PERR_UNEXPCOMMA;
        return -1;
      }

    default:
      compiler_error( "Don't know what to do with '", *token, "' in SmartParser::parseToken\n",
                      ctx );
      err = PERR_WAAH;
      return -1;
    }
  }
}

/**
 * Some identifiers are functions (user-defined or module-defined)
 * these are recognized here.  HOWEVER, in some cases these should
 * be ignored - particularly, after the "." operator and its ilk.
 * For example, if the LEN function is defined,
 * "print a.len;" should still be valid (assuming A is a variable
 * with a 'len' member).  In these cases, the operator in question
 * will be at the top of the TX stack.  So, if we find this operator
 * there, we won't check for functions.
 * This is also the perfect opportunity to morph would-be identifiers
 * into strings, or "member names" if that turns out the way to go.
 * (Normally, we would emit TOK_IDENT(left) TOK_IDENT(right) TOK_MEMBER.
 * The problem here is that TOK_IDENT(left) is seen as a variable
 * (quite rightly), but so is TOK_IDENT(right), which is wrong.
 * We used to transform this in addToken, but this is better
 * I think.)
 *
 * @note let's suppose..
 * an overridden getToken is really smart, and figures out if IDENTS
 * are variable names, verbs, functions,  or labels.  To this end it
 * pulls out the ':' if necessary.
 * TYP_OPERAND, TOK_VARNAME
 * TYP_FUNC,  TOK_MID,  <-- TYP_OPERAND for purposes of legality
 * TYP_PROC,  TOK_PRINT,
 * TYP_LABEL,   (don't care)
 *
 * IP still does the same thing only it no longer looks for isVerb.
 * have the new getToken put, say, the verb number in lval, so we now
 * have an array element number.
 *
 * @see Parser::getToken( CompilerContext& ctx, Token& token )
 */
int SmartParser::getToken( CompilerContext& ctx, Token& token, Expression* pexpr )
{
  int res = Parser::getToken( ctx, token );
  if ( res == -1 )
    return -1;
  if ( token.id == TOK_IDENT )
  {
    if ( pexpr )
    {
      if ( !pexpr->TX.empty() )
      {
        Token* tkn = pexpr->TX.top();
        if ( tkn->id == TOK_MEMBER || tkn->id == TOK_ADDMEMBER || tkn->id == TOK_DELMEMBER ||
             tkn->id == TOK_CHKMEMBER )
        {
          token.id = TOK_STRING;
          return res;
        }
      }
    }
    if ( isFunc( token, &modfunc_ ) )
      return 0;
    if ( isUserFunc( token, &userfunc_ ) )
      return 0;
  }
  return res;
}

bool SmartParser::callingMethod( CompilerContext& ctx )
{
  // if we have something like x.foo(), change to call-method
  CompilerContext tctx( ctx );
  Token tk_name, tk_paren;

  if ( getToken( tctx, tk_name ) == 0 && getToken( tctx, tk_paren ) == 0 &&
       ( tk_name.id == TOK_IDENT || tk_name.id == TOK_FUNC || tk_name.id == TOK_USERFUNC ) &&
       tk_paren.id == TOK_LPAREN )
  {
    return true;
  }
  return false;
}

/**
 * if comma terminator is allowed, (reading args, or declaring variables)
 * leaves the terminator/comma.
 * if comma term not allowed, eats the semicolon.
 * if right paren allowed, leaves the right paren.
 *
 * @note (obviously, this function's behavior needs some work!)
 **/
int SmartParser::IIP( Expression& expr, CompilerContext& ctx, unsigned flags )
{
  BTokenType last_type = TYP_TERMINATOR;
  Token last_token;
  // Token* debug_last_tx_token = nullptr;
  int done = 0;
  int res = 0;  // 1=done, -1=error, 0=not done

  int semicolon_term_allowed = flags & EXPR_FLAG_SEMICOLON_TERM_ALLOWED;
  int comma_term_allowed = flags & EXPR_FLAG_COMMA_TERM_ALLOWED;
  int rightparen_term_allowed = flags & EXPR_FLAG_RIGHTPAREN_TERM_ALLOWED;
  int rightbrace_term_allowed = flags & EXPR_FLAG_RIGHTBRACE_TERM_ALLOWED;
  int dictkey_term_allowed = flags & EXPR_FLAG_DICTKEY_TERM_ALLOWED;
  int endenum_term_allowed = flags & EXPR_FLAG_ENDENUM_TERM_ALLOWED;
  int to_term_allowed = flags & EXPR_FLAG_TO_TERM_ALLOWED;
  int auto_term_allowed = flags & EXPR_FLAG_AUTO_TERM_ALLOWED;
  if ( !quiet )
  {
    char buf[80];
    Clib::stracpy( buf, ctx.s, 80 );
    strtok( buf, "\r\n" );
    INFO_PRINT << "Parsing " << buf << "\n";
  }

  expr.TX.push( new Token ); /* push a terminator token */
  int leftbracket_count = 0;
  while ( !done && ( res == 0 ) )
  {
    const char* t = ctx.s;
    Token token;

    res = peekToken( ctx, token );

    if ( res )
      break;
    if ( !isLegal( token ) )
    {
      err = PERR_NOTLEGALHERE;
      res = -1;
      break;
    }

    //    if (token.type == TYP_DELIMITER) break;
    // debug_last_tx_token = expr.TX.top();

    /*
            if (comma_term_allowed)
            {
            Token* tkn = expr.TX.top();
            if (leftbracket_count > 0) // tkn != nullptr && tkn->id == TOK_LBRACKET )
            {
            // ignore commas if we're in a left bracket situation.
            ;
            }
            else if (token.id == TOK_COMMA || token.id == TOK_SEMICOLON)
            {
            break;
            }
            }
            */
    if ( leftbracket_count == 0 )
    {
      if ( comma_term_allowed && ( token.id == TOK_COMMA || token.id == TOK_SEMICOLON ) )
      {
        break;
      }
      if ( endenum_term_allowed && ( token.id == RSV_ENDENUM ) )
      {
        break;
      }
      if ( to_term_allowed && ( token.id == RSV_TO ) )
      {
        break;
      }
    }

    if ( rightbrace_term_allowed && token.id == TOK_RBRACE )
    {
      break;
    }
    if ( dictkey_term_allowed && token.id == TOK_DICTKEY )
    {
      break;
    }

    /*
            else
            {
            if (token.type == TYP_SEPARATOR)
            {
            err = PERR_UNEXPCOMMA;
            res = -1;
            break;
            }
            }
            */

    // auto-terminated expressions need to not eat what broke them out...
    CompilerContext save_ctx( ctx );

    res = getToken( ctx, token, &expr );

    if ( semicolon_term_allowed && token.id == TOK_SEMICOLON )
      break;

    if ( token.id == TOK_LBRACKET )
      ++leftbracket_count;
    else if ( token.id == TOK_RBRACKET )
      --leftbracket_count;

    if ( !isOkay( token, last_token ) )

    {
      if ( token.type == TYP_OPERATOR ||
           token.type == TYP_UNARY_PLACEHOLDER )  // check for a unary operator that looks the same.
        recognize_unary( token, token.tokval() );
      if ( token.id == TOK_UNPLUSPLUS && last_type == TYP_OPERAND )  // switching to post increment
        token.id = TOK_UNPLUSPLUS_POST;
      else if ( token.id == TOK_UNMINUSMINUS &&
                last_type == TYP_OPERAND )  // switching to post decrement
        token.id = TOK_UNMINUSMINUS_POST;
      if ( !isOkay( token, last_token ) )
      {
        if ( auto_term_allowed )
        {
          ctx = save_ctx;
          break;
        }

        res = -1;
        err = PERR_ILLEGALCONS;
        ctx.s = t;  // FIXME operator=
        compiler_error( "Token '", token, "' cannot follow token '", last_token, "'\n" );
        if ( last_token.type == TYP_OPERAND && token.id == TOK_LPAREN )
        {
          compiler_error( "Function ", last_token, "() is not defined.\n" );
        }
        break;
      }
    }
    else if ( last_type == TYP_TERMINATOR && token.type == TYP_LEFTBRACE &&
              compilercfg.ParanoiaWarnings )
    {
      compiler_warning(
          &ctx,
          "Warning: Using { } is inappropriate; please define array, struct or dictionary.\n" );
    }
    last_type = token.type;
    last_token = token;

    //
    // only certain types of things can be in an auto-terminated expr
    // stuff like 'var' and statements can't.
    //
    if ( auto_term_allowed )
    {
      if ( token.type != TYP_FUNC && token.type != TYP_USERFUNC && token.type != TYP_OPERAND &&
           token.type != TYP_UNARY_OPERATOR && token.type != TYP_OPERATOR &&
           token.type != TYP_LEFTPAREN && token.type != TYP_RIGHTPAREN &&
           token.type != TYP_LEFTBRACKET && token.type != TYP_RIGHTBRACKET &&
           token.type != TYP_SEPARATOR && token.id != TOK_LBRACE && token.id != TOK_RBRACE )
      {
        ctx = save_ctx;
        break;
      }
    }

    Token* ptok2;
    if ( token.type == TYP_USERFUNC )
    {
      res = getUserArgs( expr, ctx );
      if ( res < 0 )
        return res;

      ptok2 = new Token( token );
      res = parseToken( ctx, expr, ptok2 );
      if ( res < 0 )
      {
        delete ptok2;
        ctx.s = t;
      }
    }
    else if ( token.type == TYP_FUNC )
    {
      userfunc_ = modfunc_->uf;
      res = getUserArgs( expr, ctx, false );
      if ( res < 0 )
      {
        compiler_error( "Error getting arguments for function ", token.tokval(), "\n", ctx, "\n" );
        return res;
      }
      ptok2 = new Token( token );
      res = parseToken( ctx, expr, ptok2 );
      if ( res < 0 )
      {
        delete ptok2;
        ctx.s = t;  // FIXME operator=

        if ( ( err == PERR_UNEXRPAREN ) && rightparen_term_allowed )
        {
          err = PERR_NONE;
          res = 0;
          done = 1;
        }
      }
    }
    else if ( token.id == TOK_ARRAY )
    {
      auto array_tkn = new Token( TOK_ARRAY, TYP_OPERAND );
      array_tkn->dbg_filenum = token.dbg_filenum;
      array_tkn->dbg_linenum = token.dbg_linenum;

      res = parseToken( ctx, expr, array_tkn );
      if ( res < 0 )
        return res;

      // expr.CA.push( array_tkn );

      // 'array' can be of the following forms:
      // var x := array;          // preferred
      // var x := array { 2, 4, 6, 1 };   // preferred
      // var x := array ( 2, 4, 6, 1 );   // not preferred, looks too much like a multi-dim

      Token peek_token;
      res = peekToken( ctx, peek_token );
      if ( res == 0 )
      {
        if ( peek_token.id == TOK_LPAREN )
        {
          res = getArrayElements( expr, ctx );
        }
        else if ( peek_token.id == TOK_LBRACE )
        {
          // this form expects that the LBRACE has been eaten, so do so.
          getToken( ctx, peek_token );
          res = getNewArrayElements( expr, ctx );
        }
        if ( res < 0 )
          compiler_error( "Error getting elements for array\n" );
      }
    }
    else if ( token.id == TOK_LBRACE )  // a bare array declaration, like var x := { 2, 4 };
    {
      expr.CA.push( new Token( TOK_ARRAY, TYP_OPERAND ) );
      res = getNewArrayElements( expr, ctx );
      if ( res < 0 )
        compiler_error( "Error getting elements for array\n" );
    }
    else if ( token.id == TOK_ERROR )
    {
      auto error_tkn = new Token( TOK_ERROR, TYP_OPERAND );
      error_tkn->dbg_filenum = token.dbg_filenum;
      error_tkn->dbg_linenum = token.dbg_linenum;
      expr.CA.push( error_tkn );
      res = getStructMembers( expr, ctx );
      if ( res < 0 )
        compiler_error( "Error reading members for error\n" );
    }
    else if ( token.id == TOK_STRUCT )
    {
      auto struct_tkn = new Token( TOK_STRUCT, TYP_OPERAND );
      struct_tkn->dbg_filenum = token.dbg_filenum;
      struct_tkn->dbg_linenum = token.dbg_linenum;
      expr.CA.push( struct_tkn );
      res = getStructMembers( expr, ctx );
      if ( res < 0 )
        compiler_error( "Error reading members for struct\n" );
    }
    else if ( token.id == TOK_DICTIONARY )
    {
      auto dict_tkn = new Token( token );  // Mod_Basic, TOK_DICTIONARY, TYP_OPERAND );
      // struct_tkn->dbg_filenum = token.dbg_filenum;
      // struct_tkn->dbg_linenum = token.dbg_linenum;
      expr.CA.push( dict_tkn );
      res = getDictionaryMembers( expr, ctx );
      if ( res < 0 )
        compiler_error( "Error reading members for dictionary\n" );
    }
    else if ( token.id == TOK_FUNCREF )
    {
      auto ref_tkn = new Token( token );
      res = getFunctionPArgument( expr, ctx, ref_tkn );
      if ( res < 0 )
        compiler_error( "Error reading function reference argument\n" );
      expr.CA.push( ref_tkn );
    }
    else if ( token.id == TOK_MEMBER && callingMethod( ctx ) )
    {
      // look for something like x.foo(b);
      // we'll catch this on the 'foo'
      // so check for preceding TOK_MEMBER, and succeeding TOK_LPAREN

      ptok2 = new Token( token );  // this we will turn into an INS_CALL_METHOD.
      int _res = parseToken( ctx, expr, ptok2 );
      if ( _res < 0 )
      {
        delete ptok2;
        return _res;
      }

      // grab the method name
      getToken( ctx, token, &expr );
      std::string methodName( token.tokval() );
      Clib::mklowerASCII( methodName );
      int nargs;
      res = getMethodArguments( expr, ctx, nargs );
      // our ptok2 is now sitting in TX.  Move it to CA.
      Token* _t = expr.TX.top();
      expr.TX.pop();
      expr.CA.push( _t );

      ObjMethod* objmeth = getKnownObjMethod( methodName.c_str() );
      if ( objmeth != nullptr && compilercfg.OptimizeObjectMembers )
      {
        ptok2->id = INS_CALL_METHOD_ID;
        ptok2->type = TYP_METHOD;
        ptok2->lval = nargs;
        ptok2->dval = objmeth->id;
      }
      else
      {
        ptok2->id = INS_CALL_METHOD;
        ptok2->type = TYP_METHOD;
        ptok2->lval = nargs;
        ptok2->copyStr( methodName.c_str() );
      }
      last_type = TYP_OPERAND;
      last_token.type = TYP_OPERAND;  // FIXME: kind of a hack
    }
    else
    {
      if ( res >= 0 )
      {
        ptok2 = new Token( token );
        res = parseToken( ctx, expr, ptok2 );
        if ( res < 0 )
        {
          delete ptok2;
          ctx.s = t;  // FIXME operator=

          if ( ( err == PERR_UNEXRPAREN ) && rightparen_term_allowed )
          {
            err = PERR_NONE;
            res = 0;
            done = 1;
          }
        }
      }
    }
    if ( flags & EXPR_FLAG_SINGLE_ELEMENT )
      done = 1;
  }

  int nres = parseToken( ctx, expr, new Token ); /* end of expression */
  if ( res >= 0 )
    res = nres;

  if ( !comma_term_allowed && !quiet )
  {
    INFO_PRINT << "Result: " << res << "\n";
  }
  return res;
}

}  // namespace Bscript
}  // namespace Pol
