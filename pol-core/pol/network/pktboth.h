/** @file
 *
 * @par History
 * - 2009/07/23 MuadDib:   Initial creation.
 * - 2009/12/03 Turley:    toggle gargoyle flying support
 * - 2009/12/17 Turley:    bf sub 0x16 - Tomi
 *
 * @note This file is for packet structs who sent by both client and server.
 */


#ifndef __PKTBOTHH
#define __PKTBOTHH

#include "../clib/rawtypes.h"
#include "../plib/uconst.h"
#include "pktbothid.h"
#include "pktdef.h"

namespace Pol
{
namespace Core
{
#pragma pack( push, 1 )

// Walk Approved
struct PKTBI_22_APPROVED
{
  u8 msgtype;
  u8 movenum;
  u8 noto;
};
static_assert( sizeof( PKTBI_22_APPROVED ) == 3, "size missmatch" );

// Walk Resync Requested By Client
struct PKTBI_22_SYNC
{
  u8 msgtype;
  u8 zero1;
  u8 zero2;
};
static_assert( sizeof( PKTBI_22_SYNC ) == 3, "size missmatch" );

/* Message 2C:  A little bit strange.
First, the server sends "2C 00" to the client, to tell it it's dead.
Next, the client pops up the res menu.
Next, the user picks.  If the user picks ghost, the client sends "2C 02" - "2C 01" if instares.
Now, while a ghost, it seems the client transmits "2C 00" in front of MSG72 "war mode"
messages.  Not sure why.
*/
struct PKTBI_2C
{
  u8 msgtype;
  u8 choice;
};
static_assert( sizeof( PKTBI_2C ) == 2, "size missmatch" );

// All Names
struct PKTBI_98_IN
{
  u8 msgtype;
  u16 msglen;
  u32 serial;
};
static_assert( sizeof( PKTBI_98_IN ) == 7, "size missmatch" );

// All Names
// struct PKTBI_98_OUT
//{
//    u8 msgtype;
//    u16 msglen;
//    u32 serial;
//    char name[30];
//};
// static_assert( sizeof(PKTBI_98_OUT) == 37, "size missmatch" );

/* NOTE on MSG3A:
    send the header, plus skills[] for each skill,
    plus two zeros in 'skillid' after all the skills
    sent.
    I'm not sure if it's OK to only send a few skills.
    */
struct PKTBI_3A_VALUES
{
  u8 msgtype;
  u16 msglen;
  u8 unk3;
  struct
  {
    u16 skillid;
    u16 value;
    u16 value_unmod;
    u8 lock_mode;
  } skills[100];
  enum
  {
    LOCK_NONE = 0,
    LOCK_DOWN = 1,
    LOCK_LOCKED = 2
  };
  enum
  {
    FULL_LIST = 0x00,
    FULL_LIST_CAP = 0x02,
    SINGLE_SKILL = 0xFF,
    SINGLE_SKILL_CAP = 0xDF
  };
  u16 terminator;
};

//// FIXME: Join this with the one up there, with some buffer or something... -- Nando, 2009-03-17
// struct PKTBI_3A_CAPS
//{
//    u8 msgtype;
//    u16 msglen;
//    u8 unk3;
//    struct
//    {
//        u16 skillid;
//        u16 value;
//        u16 value_unmod;
//        u8 lock_mode;
//    u16 cap;
//    } skills[100];//SKILLID__CLIENT_HIGHEST - SKILLID__CLIENT_LOWEST + 1 ];
//    enum { LOCK_NONE = 0, LOCK_DOWN = 1, LOCK_LOCKED = 2 };
//    //u16 terminator;
//};

struct PKTBI_3A_LOCKS
{
  u8 msgtype;
  u16 msglen;
  u16 skillid;
  u8 lock_mode;
};
static_assert( sizeof( PKTBI_3A_LOCKS ) == 6, "size missmatch" );

struct PKTBI_3B
{
  u8 msgtype;
  u16 msglen;
  u32 vendor_serial;
  u8 status;
  enum
  {
    STATUS_NOTHING_BOUGHT = 0,
    STATUS_ITEMS_BOUGHT = 2
  };
  struct
  {
    u8 layer;
    u32 item_serial;
    u16 number_bought;
  } items[( MAXBUFFER - 8 ) / 7];
};
static_assert( sizeof( PKTBI_3B ) == 2556, "size missmatch" );

struct PKTBI_56
{
  u8 msgtype;
  u32 serial;
  u8 type;
  u8 pinidx;
  u16 pinx;
  u16 piny;
  enum
  {
    TYPE_ADD = 1,
    TYPE_INSERT,
    TYPE_CHANGE,
    TYPE_REMOVE,
    TYPE_REMOVE_ALL,
    TYPE_TOGGLE_EDIT,
    TYPE_TOGGLE_RESPONSE
  };
};
static_assert( sizeof( PKTBI_56 ) == 11, "size missmatch" );

struct PKTBI_66
{
  u8 msgtype;
  u16 msglen;
  u32 book_serial;
  u16 pages;
  u16 page;
  u16 lines;
  char text[MAXBUFFER - 13];
};
static_assert( sizeof( PKTBI_66 ) == MAXBUFFER, "size missmatch" );

// struct PKTBI_66_HDR
//{
//  u8 msgtype;
//  u16 msglen;
//  u32 book_serial;
//  u16 pages;
//};
// static_assert( sizeof( PKTBI_66_HDR ) == 9, "size missmatch" );
//
// struct PKTBI_66_CONTENTS
//{
//  u16 page;
//  u16 lines;
//};
// static_assert( sizeof( PKTBI_66_CONTENTS ) == 4, "size missmatch" );

struct PKTBI_6C
{
  u8 msgtype;
  u8 unk1;  // TARGET_CURSOR_CREATE / _DELETE ??
  u32 target_cursor_serial;
  u8 cursor_type;
  u32 selected_serial;
  u16 x;
  u16 y;
  u8 unk15;
  s8 z;
  u16 graphic;
  enum
  {
    UNK1_00,
    UNK1_01
  };
  enum CURSOR_TYPE
  {
    CURSOR_TYPE_OBJECT = 0,
    CURSOR_TYPE_COORDINATE = 1,

    CURSOR_TYPE_NEUTRAL = 0,
    CURSOR_TYPE_HARMFUL = 1,
    CURSOR_TYPE_HELPFUL = 2
  };
};
static_assert( sizeof( PKTBI_6C ) == 19, "size missmatch" );

struct PKTBI_6F
{
  u8 msgtype;
  u16 msglen;
  u8 action;
  u32 chr_serial;
  u32 cont1_serial;
  u32 cont2_serial;
  u8 havename;
  char name[30];

  enum
  {
    ACTION_INIT = 0,
    ACTION_CANCEL = 1,
    ACTION_STATUS = 2
  };
};
static_assert( sizeof( PKTBI_6F ) == 47, "size missmatch" );

struct PKTBI_71
{
  // One of these...
  struct HEADER
  {
    u8 msgtype;
    u16 msglen;
    u8 subcmd;
  };

  enum SUBCMDs
  {
    OUT_0_DISPLAY_BOARD = 0,
    OUT_1_MESSAGE_SUMMARY = 1,
    OUT_2_MESSAGE = 2,
    IN_3_REQ_MESSAGE = 3,
    IN_4_REQ_MESSAGE_SUMMARY = 4,
    IN_5_POST_MESSAGE = 5,
    IN_6_REMOVE_MESSAGE = 6
  };

  // Followed by a sub-cmd block...
  struct SUBCMD_OUT_0_DISPLAY_BOARD
  {
    u32 board_id;
    char name[22];
    u32 id;      // always 0x402000FF ?
    u32 unk_00;  // always 0x00
  };

  struct SUBCMD_OUT_1_MESSAGE_SUMMARY
  {
    u32 board_id;
    u32 msg_id;
    u32 parent_id;  // 0 if top-level
    struct POSTER
    {
      u8 post_len;
      char poster[1];  // post_len bytes long (null term)
    };
    struct SUBJECT
    {
      u8 subj_len;
      char subject[1];  // subj_len bytes long (null term)
    };
    struct TIME
    {
      u8 time_len;
      char time[1];  // time_len bytes long (null term)
    };
  };

  struct SUBCMD_OUT_2_MESSAGE
  {
    u32 board_id;
    u32 msg_id;
    struct POSTER
    {
      u8 post_len;
      char poster[1];  // post_len bytes long (nullptr term)
    };
    struct SUBJECT
    {
      u8 subj_len;
      char subject[1];  // subj_len bytes long (null term)
    };
    struct TIME
    {
      u8 time_len;
      char time[1];  // time_len bytes long (null term)
    };
    struct LINE_HEADER
    {
      // 01 91 84 0A 06 1E FD 01 0B 15 2E 01 0B 17 0B
      // 01 BB 20 46 04 66 13 F8 00 00 0E 75 00 00
      u8 constant_hdr[29];  // use MSG71_SUBCMD_2_CONSTANT to initialise
      u8 numlines;
    };
    struct LINES
    {
      u8 line_len;
      char body[1];  // line_len bytes long (null term)
    };
  };

  struct SUBCMD_IN_3_REQ_MESSAGE
  {
    u32 board_id;
    u32 msg_id;
  };

  struct SUBCMD_IN_4_REQ_MESSAGE_SUMMARY
  {
    u32 board_id;
    u32 msg_id;
  };

  struct SUBCMD_IN_5_POST_MESSAGE
  {
    u32 board_id;
    u32 reply_id;  // 0 for top-level or if not a reply
    struct SUBJECT
    {
      u8 subj_len;
      char subject[1];  // subj_len bytes long (null term)
    };
    struct LINE_HEADER
    {
      u8 numlines;
    };
    struct LINES
    {
      u8 line_len;
      char body[1];  // line_len bytes long (null term)
    };
  };

  struct SUBCMD_IN_6_REMOVE_MESSAGE
  {
    u32 board_id;
    u32 msg_id;
  };
};
static_assert( sizeof( PKTBI_71::HEADER ) == 4, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_OUT_0_DISPLAY_BOARD ) == 34, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_OUT_1_MESSAGE_SUMMARY::POSTER ) == 2, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_OUT_1_MESSAGE_SUMMARY::SUBJECT ) == 2, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_OUT_1_MESSAGE_SUMMARY::TIME ) == 2, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_OUT_2_MESSAGE::POSTER ) == 2, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_OUT_2_MESSAGE::SUBJECT ) == 2, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_OUT_2_MESSAGE::TIME ) == 2, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_OUT_2_MESSAGE::LINE_HEADER ) == 30, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_OUT_2_MESSAGE::LINES ) == 2, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_IN_3_REQ_MESSAGE ) == 8, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_IN_4_REQ_MESSAGE_SUMMARY ) == 8, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_IN_5_POST_MESSAGE::SUBJECT ) == 2, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_IN_5_POST_MESSAGE::LINE_HEADER ) == 1, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_IN_5_POST_MESSAGE::LINES ) == 2, "size missmatch" );
static_assert( sizeof( PKTBI_71::SUBCMD_IN_6_REMOVE_MESSAGE ) == 8, "size missmatch" );

struct PKTBI_72
{
  u8 msgtype;
  u8 warmode; /* 00 = not, 01 = war mode */
  u8 unk2;
  u8 unk3_32;
  u8 unk4;
};
static_assert( sizeof( PKTBI_72 ) == 5, "size missmatch" );

struct PKTBI_73
{
  u8 msgtype;
  u8 unk1;
};
static_assert( sizeof( PKTBI_73 ) == 2, "size missmatch" );

struct PKTBI_93
{
  u8 msgtype;
  u32 serial;
  u8 writable;
  u8 unk_1;
  u16 npages;
  char title[60];
  char author[30];
};
static_assert( sizeof( PKTBI_93 ) == 99, "size missmatch" );

struct PKTBI_95
{
  u8 msgtype;
  u32 serial;
  u16 unk;
  u16 graphic_or_color;
};
static_assert( sizeof( PKTBI_95 ) == 9, "size missmatch" );

struct PKTBI_99
{
  u8 msgtype;
  u8 unk1_01;
  u32 deed;
  u8 unk_x06_x11[12];
  u16 graphic;
  u16 xoffset;
  u16 yoffset;
  u16 maybe_zoffset;
};
static_assert( sizeof( PKTBI_99 ) == 26, "size missmatch" );

struct PKTBI_9A
{
  u8 msgtype;
  u16 msglen;
  u32 serial;
  u32 prompt;
  u32 type;
  char text[MAXBUFFER - 15];
};
static_assert( sizeof( PKTBI_9A ) == MAXBUFFER, "size missmatch" );

struct PKTBI_B8_IN_REQUEST
{
  u32 serial;
};
static_assert( sizeof( PKTBI_B8_IN_REQUEST ) == 4, "size missmatch" );

struct PKTBI_B8_IN_UPDATE
{
  u32 serial;
  u16 cmdtype;
  u16 textlen;
  u16 wtext[SPEECH_MAX_LEN + 1];
};
static_assert( sizeof( PKTBI_B8_IN_UPDATE ) == SPEECH_MAX_LEN * 2 + 2 + 8, "size missmatch" );

struct PKTBI_B8_IN
{
  u8 msgtype;
  u16 msglen;
  u8 mode;
  union
  {
    PKTBI_B8_IN_REQUEST profile_request;
    PKTBI_B8_IN_UPDATE profile_update;
  };
  enum
  {
    MODE_REQUEST = 0,
    MODE_UPDATE = 1
  };
};

// struct PKTBI_B8_OUT {
//  u8 msgtype;
//  u16 msglen;
//  u32 serial;
//  char text[5*(SPEECH_MAX_LEN+1)]; // 1 Ascii variable array ( SPEECH_MAX_LEN+1) and 2 Unicode
// variable arrays 2*(SPEECH_MAX_LEN+1) each due to u16.
//};
// static_assert( sizeof(PKTBI_B8_OUT) == 1012, "size missmatch" );

struct PKTBI_BB
{
  u8 msgtype;
  u32 serial1;
  u32 serial2;
};
static_assert( sizeof( PKTBI_BB ) == 9, "size missmatch" );

struct PKTBI_BD
{
  u8 msgtype;
  u16 msglen;
  char version[MAXBUFFER - 3];
};
static_assert( sizeof( PKTBI_BD ) == MAXBUFFER, "size missmatch" );

struct PKTBI_BF_04
{
  u32 gumpid;
  u32 buttonid;  // response for 0xB1
};

struct PKTBI_BF_05
{
  u16 unk1;  // always 0
  u16 x;
  u16 y;
  u16 unk2;
};

struct PKTBI_BF_06_01_IN
{
  u32 memberid;
};
static_assert( sizeof( PKTBI_BF_06_01_IN ) == 4, "size missmatch" );

// struct PKTBI_BF_06_01_OUT
//{
//  u8 nummembers;
//  struct {
//    u32 memberid;
//  } serials[ 1 ];
//};
// static_assert( sizeof(PKTBI_BF_06_01_OUT) == 5, "size missmatch" );

struct PKTBI_BF_06_02_IN
{
  u32 memberid;
};

// struct PKTBI_BF_06_02_OUT
//{
//  u8 nummembers; //total nr of new party
//  u32 remmemberid;  //removed Player
//  struct {
//    u32 memberid;
//  } serials[ 1 ]; //party members
//};
// static_assert( sizeof(PKTBI_BF_06_02_OUT) == 9, "size missmatch" );

// struct PKTBI_BF_06_02_LIST
//{
//  u8 nummembers;
//  u32 remmemberid;
//};
// static_assert( sizeof(PKTBI_BF_06_02_LIST) == 5, "size missmatch" );

struct PKTBI_BF_06_03
{
  u32 memberid;                   //(of target, from client, of source, from server)
  u16 wtext[SPEECH_MAX_LEN + 1];  // wide-character, double-null terminated
};
static_assert( sizeof( PKTBI_BF_06_03 ) == SPEECH_MAX_LEN * 2 + 2 + 4, "size missmatch" );

struct PKTBI_BF_06_04_IN
{
  u16 wtext[SPEECH_MAX_LEN + 1];  // wide-character, double-null terminated
};
static_assert( sizeof( PKTBI_BF_06_04_IN ) == SPEECH_MAX_LEN * 2 + 2, "size missmatch" );

// struct PKTBI_BF_06_04_OUT
//{
//  u32 memberid;
//  u16 wtext[(SPEECH_MAX_LEN) + 1]; // wide-character, double-null terminated
//};
// static_assert( sizeof(PKTBI_BF_06_04_OUT) == 406, "size missmatch" );

struct PKTBI_BF_06_06
{
  u8 canloot;  // 0,1
  enum
  {
    CANLOOT_YES = 1,
    CANLOOT_NO = 0
  };
};
static_assert( sizeof( PKTBI_BF_06_06 ) == 1, "size missmatch" );

// struct PKTBI_BF_06_07
//{
//  u32 leaderid;
//};
// static_assert( sizeof(PKTBI_BF_06_07) == 4, "size missmatch" );

struct PKTBI_BF_06_08
{
  u32 leaderid;
};
static_assert( sizeof( PKTBI_BF_06_08 ) == 4, "size missmatch" );

struct PKTBI_BF_06_09
{
  u32 leaderid;
};
static_assert( sizeof( PKTBI_BF_06_09 ) == 4, "size missmatch" );

struct PKTBI_BF_06
{
  u8 partycmd;
  union
  {
    PKTBI_BF_06_01_IN partyadd;  // client
    // PKTBI_BF_06_01_OUT partyaddout; //server
    PKTBI_BF_06_02_IN partyremove;  // client
    // PKTBI_BF_06_02_OUT partyremoveout; //server
    // PKTBI_BF_06_02_LIST partyemptylist; //server
    PKTBI_BF_06_03 partymembermsg;  // client/server
    PKTBI_BF_06_04_IN partymsg;     // client
    // PKTBI_BF_06_04_OUT partymsgout; //server
    PKTBI_BF_06_06 partylootperm;  // client
    // PKTBI_BF_06_07 invitemember; //server
    PKTBI_BF_06_08 partyaccinvite;  // client
    PKTBI_BF_06_09 partydecinvite;  // client
  };
  enum
  {
    PARTYCMD_ADD = 1,
    PARTYCMD_REMOVE = 2,
    PARTYCMD_MEMBER_MSG = 3,
    PARTYCMD_PARTY_MSG = 4,
    PARTYCMD_LOOT_PERMISSION = 6,
    PARTYCMD_INVITE_MEMBER = 7,
    PARTYCMD_ACCEPT_INVITE = 8,
    PARTYCMD_DECLINE_INVITE = 9
  };
};

struct PKTBI_BF_0F
{
  u8 unk1;  // 0x0a
  u32 clientflag;
};

struct PKTBI_BF_10
{
  u32 serial;
  u32 listid;
};

// TextID is broken into two decimal parts:
// stringID / 1000: intloc fileID
// stringID % 1000: text index
// So, say you want the 123rd text entry of intloc06, the stringID would be 6123
struct PKTBI_BF_14_ENTRIES
{
  u16 entry_tag;
  u16 text_id;
  u16 flags;
  // if flags & 0x20:
  u16 color;  // rgb 1555 color (ex, 0 = transparent, 0x8000 = solid black, 0x1F = blue, 0x3E0 =
              // green, 0x7C00 = red)

  enum
  {
    POPUP_MENU_LOCKED = 0x1,
    POPUP_MENU_ARROW = 0x2,
    POPUP_MENU_COLOR = 0x20
  };
};

struct PKTBI_BF_14
{
  u16 unk;  // always 00 01
  u32 serial;
  u8 numentries;
  PKTBI_BF_14_ENTRIES entries[( MAXBUFFER - 7 ) / 8];
};

struct PKTBI_BF_15
{
  u32 serial;
  u16 entry_tag;
};

// struct PKTBI_BF_16
//{
//  u32 window_id;
//  u32 serial; // Char serial ( if window_id = CONTAINER this is container serial )
//
//  enum
//  {
//    PAPERDOLL  = 0x1,
//    STATUS    = 0x2,
//    CHARPROFILE  = 0x8,
//    CONTAINER  = 0xC
//  };
//};

// Shows codex of wisdom's text #msg.  (msg is linearised (including sub indices) index number
// starting with 1)
struct PKTBI_BF_17
{
  u8 unk;  // always 1
  u32 msgnum;
  u8 mode;  //(0: "?" flashing, 1: directly opening)
};

// struct PKTBI_BF_18_ELEM
//{
//  u32 num_static_patches;
//  u32 num_map_patches;
//};
//
//
// struct PKTBI_BF_18
//{
//  u32 num_maps;
//  PKTBI_BF_18_ELEM elems[MAX_NUMER_REALMS];
//};

// struct PKTBI_BF_19
//{
//  u8 type; //always 2?
//  u32 serial;
//  u8 unk; //always 0?
//  u8 lockbits; //Bits: XXSS DDII (s=strength, d=dex, i=int), 0 = up, 1 = down, 2 = locked
//  enum
//  {
//    STR_MASK = 0x30,
//    DEX_MASK = 0x0C,
//    INT_MASK = 0x03
//  };
//  enum { MODE_UP, MODE_DOWN, MODE_LOCKED };
//};

struct PKTBI_BF_1A
{
  u8 stat;
  u8 mode;
  enum
  {
    STAT_STR,
    STAT_DEX,
    STAT_INT
  };
  enum
  {
    MODE_UP,
    MODE_DOWN,
    MODE_LOCKED
  };
};

// struct PKTBI_BF_1B
//{
//  u16 unk; //always 1
//  u32 book_serial;
//  u16 graphic;
//  u16 scroll_offset;
//  u8 content[8];// first bit of first byte = spell #1, second bit of first byte = spell #2, first
// bit of second byte = spell #8, etc
//
//  enum { SCROLL_OFFSET_REGULAR = 1,
//    SCROLL_OFFSET_NECRO = 101,
//    SCROLL_OFFSET_PALADIN = 201 };
//};

struct PKTBI_BF_1C
{
  u16 unk2;            // 00 02
  u16 selected_spell;  // absolute spell number. 1-64, 101-etc
};

// struct PKTBI_BF_1D
//{
//  u32 house_serial;
//  u32 revision;
//};

struct PKTBI_BF_1E
{
  u32 house_serial;
};

// struct PKTBI_BF_20
//{
//  u32 house_serial;
//  u8 unk1; //0x4 for begin 0x5 for end
//  u16 unk2; //0
//  u32 unk3; //all FF
//  u8 unk4; // FF
//};

// displays damage amount over head
// struct PKTBI_BF_22
//{
//  u8 unk; //always 1
//  u32 serial;
//  u8 damage_amt;
//};

// no clue what this spam does.
struct PKTBI_BF_24_UNKNOWN
{
  u8 spam_byte;
};

// struct PKTBI_BF_2A_CALL_RACE_CHANGER
//{
//  u8 gender; // 0 = Male, 1 = Female
//  u8 race;   // 1 = Human, 2 = Elf
//};

struct PKTBI_BF_2A_RESULT
{
  u16 BodyHue;
  u16 HairId;
  u16 HairHue;
  u16 BeardId;
  u16 BeardHue;
};

// to change Hair, Beard and Color (maybe Race, but not in this Release)
struct PKTBI_BF_RACE_CHANGER_RESULT
{
  union
  {
    // PKTBI_BF_2A_CALL_RACE_CHANGER call;
    PKTBI_BF_2A_RESULT result;
  };
};

// Toggle gargoyle flying
struct PKTBI_BF_32
{
  u16 unk1;  // always 0x0100 in my tests
  u32 unk2;  // always 0x0 in my tests
};

struct PKTBI_BF
{
  u8 msgtype;
  u16 msglen;
  u16 subcmd;
  union
  {
    u32 keys[6];  // BF.1: Cycling key-stack
    u32 addkey;   // BF.2: Adding key to top of stack
    // PKTBI_BF_04 closegump; //BF.4
    PKTBI_BF_05 screensize;  // BF.5
    PKTBI_BF_06 partydata;   // BF.6
    // u8 cursorhue;  // BF.8: Cursor Hue (0=Felucca,1=Trammel)
    char client_lang[3];            // BF.B Client language (client message, sent once at login)
    u32 serial_closed_status_gump;  // BF.C closed status gump
    PKTBI_BF_0F clienttype;         // BF.F clienttype (like char create/select)
    PKTBI_BF_10 objectcache;        // BF.10
    u32 serial_request_popup_menu;  // BF.13
    PKTBI_BF_14 displaypopup;       // BF.14
    PKTBI_BF_15 popupselect;        // BF.15
    // PKTBI_BF_16 closewindow; //BF.16
    PKTBI_BF_17 codex;  // BF.17
    // PKTBI_BF_18 mapdiffs; //BF.18
    // PKTBI_BF_19 extstatsout; //BF.19
    PKTBI_BF_1A extstatin;  // BF.1A
    // PKTBI_BF_1B newspellbook; //BF.1B
    PKTBI_BF_1C spellselect;  // BF.1C
    // PKTBI_BF_1D customhouseshort;
    PKTBI_BF_1E reqfullcustomhouse;
    // PKTBI_BF_20 activatehousetool; //BF.20
    // PKTBI_BF_22 damage; //BF.22
    u8 se_spam;                                         // BF.24
    PKTBI_BF_RACE_CHANGER_RESULT characterracechanger;  // BF.2A
    PKTBI_BF_32 toggleflying;                           // BF.32
  };

  enum
  {
    TYPE_CYCLE_STACK = 0x01,
    TYPE_ADD_KEY = 0x02,
    TYPE_CLOSE_GUMP = 0x04,
    TYPE_SCREEN_SIZE = 0x05,
    TYPE_PARTY_SYSTEM = 0x06,
    TYPE_CURSOR_HUE = 0x08,
    TYPE_WRESTLING_STUN = 0x0A,
    TYPE_CLIENT_LANGUAGE = 0x0B,
    TYPE_CLOSED_STATUS_GUMP = 0x0C,
    TYPE_CLIENTTYPE = 0x0F,
    TYPE_OBJECT_CACHE = 0x10,
    TYPE_POPUP_MENU_REQUEST = 0x13,
    TYPE_DISPLAY_POPUP_MENU = 0x14,
    TYPE_POPUP_ENTRY_SELECT = 0x15,
    TYPE_CLOSE_WINDOW = 0x16,
    TYPE_CODEX_OF_WISDOM = 0x17,
    TYPE_ENABLE_MAP_DIFFS = 0x18,
    TYPE_EXTENDED_STATS_OUT = 0x19,
    TYPE_EXTENDED_STATS_IN = 0x1A,
    TYPE_NEW_SPELLBOOK = 0x1B,
    TYPE_SPELL_SELECT = 0x1C,
    TYPE_CUSTOM_HOUSE_SHORT = 0x1D,
    TYPE_REQ_FULL_CUSTOM_HOUSE = 0x1E,
    TYPE_ABILITY_ICON_CONFIRM = 0x21,
    TYPE_ACTIVATE_CUSTOM_HOUSE_TOOL = 0x20,
    TYPE_DAMAGE = 0x22,
    TYPE_SESPAM = 0x24,
    TYPE_CHARACTER_RACE_CHANGER = 0x2A,
    TYPE_TOGGLE_FLYING = 0x32
  };
  enum
  {
    CURSORHUE_FELUCCA = 0,
    CURSORHUE_TRAMMEL = 1
  };
};

struct PKTBI_C2
{
  u8 msgtype;
  u16 msglen;
  u32 serial;
  u32 msg_id;
  // When a server-sent message, these next 10+ bytes must all be 0x00.
  u32 unk;                        // unknown -- always 0 0 0 1 (from client)
  char lang[4];                   // "enu" - US english
  u16 wtext[SPEECH_MAX_LEN + 1];  // wide-character, double-null terminated

  enum
  {
    SERVER_MSGLEN = 0x15
  };
};
static_assert( sizeof( PKTBI_C2 ) == SPEECH_MAX_LEN * 2 + 2 + 19, "size missmatch" );

struct PKTBI_C8
{
  u8 msgtype;
  u8 range;
};
static_assert( sizeof( PKTBI_C8 ) == 2, "size missmatch" );

struct PKTBI_D1
{
  u8 msgtype;
  u8 unk_1;
};
static_assert( sizeof( PKTBI_D1 ) == 2, "size missmatch" );

struct PKTBI_D6_IN
{
  u8 msgtype;
  u16 msglen;
  struct
  {
    u32 serial;
  } serials[( MAXBUFFER - 3 ) / 4];
};
static_assert( sizeof( PKTBI_D6_IN ) == 2559, "size missmatch" );

// struct AOS_OBJECT_PROPERTY_LIST_ELEM
//{
//  u32 localization_num;
//  u16 textlen;
//  u16 text; //little endian Unicode text, not 0 terminated
//};

/*
Big-Endian for u16/32 unless otherwise noted.
This is the AOS Version of the packet
*/
// struct PKTBI_D6_OUT
//{
//  u8 msgtype;
//  u16 msglen;
//  u16 unk1; //always 1
//  u32 serial;
//  u8 unk2; //always 0
//  u8 unk3; //always 0
//  u32 listid;
//  AOS_OBJECT_PROPERTY_LIST_ELEM data[1];
//};

// fixme: which 0 is z?
struct CH_ERASE
{
  u8 unk1;  // 0x00
  u8 unk2;  // 0x00
  u8 unk3;  // 0x00
  u16 tileID;
  u8 unk4;      // 0x00
  u32 xoffset;  // from multi object center
  u8 unk5;      // 0x00
  u32 yoffset;  // from multi object center
  u8 unk6;      // 0x00
  u32 z;        // 0x00
};

struct CH_ADD
{
  u8 unk1;  // 0x00
  u8 unk2;  // 0x00
  u8 unk3;  // 0x00
  u16 tileID;
  u8 unk4;      // 0x00
  u32 xoffset;  // from multi object center
  u8 unk5;      // 0x00
  u32 yoffset;  // from multi object center
};

struct CH_ADD_MULTI
{
  u8 unk1;  // 0x00
  u8 unk2;  // 0x00
  u8 unk3;  // 0x00
  u16 multiID;
  u8 unk4;      // 0x00
  u32 xoffset;  // from multi object center
  u8 unk5;      // 0x00
  u32 yoffset;  // from multi object center
};

struct CH_SELECT_FLOOR
{
  u32 unk1;  // (0x00)
  u8 floornumber;
};

struct CH_SELECT_ROOF
{
  u8 unk1;  // 0x00
  u8 unk2;  // 0x00
  u8 unk3;  // 0x00
  u16 tileID;
  u8 unk4;      // 0x00
  u32 xoffset;  // from multi object center
  u8 unk5;      // 0x00
  u32 yoffset;  // from multi object center
  u8 unk6;
  u32 zoffset;
};

struct CH_DELETE_ROOF
{
  u8 unk1;  // 0x00
  u8 unk2;  // 0x00
  u8 unk3;  // 0x00
  u16 tileID;
  u8 unk4;      // 0x00
  u32 xoffset;  // from multi object center
  u8 unk5;      // 0x00
  u32 yoffset;  // from multi object center
  u8 unk6;
  u32 zoffset;
};


struct FIGHT_BOOK_ABILITY
{
  u32 unk1;    // (0x00)
  u8 ability;  // 1 armor ignore, 2 bleed attack � 13 whirlwind attack
};


struct CUSTOM_HOUSE_DATA_TYPE0
{
  u16 itemid;
  s8 x;
  s8 y;
  s8 z;
};
struct CUSTOM_HOUSE_DATA_TYPE1
{
  u16 itemid;
  s8 x;
  s8 y;
};
struct CUSTOM_HOUSE_DATA_TYPE2
{
  u16 itemid;
};

// must send msg A9 with flags | 0x20
// Remember, with any housing change, you must increment the
// House Revision number. Probably good to make a multi Revision
// number for this, seperate from revision that Tooltips use. :x
// Revision can be, in 0xD8, a u32.
struct PKTBI_D7
{
  u8 msgtype;
  u16 msglen;
  u32 serial;
  u16 subcmd;
  union
  {
    CH_ERASE ch_erase;
    CH_ADD ch_add;
    CH_ADD_MULTI ch_add_multi;
    CH_SELECT_FLOOR ch_select_floor;
    FIGHT_BOOK_ABILITY fight_book_ability;
    CH_SELECT_ROOF ch_select_roof;
    CH_DELETE_ROOF ch_delete_roof;
  };
  u8 unk;  // Not always 07, no clue what this is for just yet.
  enum
  {
    CUSTOM_HOUSE_BACKUP = 0x02,
    CUSTOM_HOUSE_RESTORE = 0x03,
    CUSTOM_HOUSE_COMMIT = 0x04,
    CUSTOM_HOUSE_ERASE = 0x05,
    CUSTOM_HOUSE_ADD = 0x06,
    CUSTOM_HOUSE_QUIT = 0x0C,
    CUSTOM_HOUSE_ADD_MULTI = 0x0D,
    CUSTOM_HOUSE_SYNCH = 0x0E,
    CUSTOM_HOUSE_CLEAR = 0x10,
    CUSTOM_HOUSE_SELECT_FLOOR = 0x12,
    CUSTOM_HOUSE_SELECT_ROOF = 0x13,  // SE Introduced?
    CUSTOM_HOUSE_DELETE_ROOF = 0x14,  // SE Introduced?
    FIGHT_BOOK = 0x19,
    CUSTOM_HOUSE_REVERT = 0x1A,
    GUILD_BUTTON = 0x28,
    QUEST_BUTTON = 0x32
  };
};

struct PKTBI_F0_00
{
  u32 serial;
  u16 x;
  u16 y;
  u8 map;
};

struct PKTBI_F0_01_GUILD_MEMBER_LOCATION
{
  u16 x;
  u16 y;
  u8 map;
};

struct PKTBI_F0_01_GUILD_MEMBER
{
  u32 serial;
  PKTBI_F0_01_GUILD_MEMBER_LOCATION location;
};

struct PKTBI_F0_01
{
  u8 include_locations;
  PKTBI_F0_01_GUILD_MEMBER member;
};

struct PKTBI_F0
{
  u8 msgtype;
  u16 msglen;
  u8 subcmd;
  union
  {
    PKTBI_F0_00 query_party;
    PKTBI_F0_01 query_guild;
  };
  enum
  {
    QUERY_PARTY = 0x00,
    QUERY_GUILD = 0x01,
  };
};
static_assert( sizeof( PKTBI_F0 ) == 14, "size missmatch" );


#pragma pack( pop )
}  // namespace Core
}  // namespace Pol
#endif
