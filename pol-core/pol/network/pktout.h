/** @file
 *
 * @par History
 * - 2009/07/23 MuadDib:   Initial creation. This file is for packet structs who sent by server
 * only.
 * - 2009/07/24 MuadDib:   Rewrote packet 0x1C structure to remove secondary struct.
 * - 2009/07/26 MuadDib:   Added packet 0x0B
 * - 2009/07/27 MuadDib:   Added packet 0x97
 * - 2009/08/01 MuadDib:   Added packet 0x86, 0x9C. Completed moving all "Outbound only" Packet
 * structs to here.
 * - 2009/08/09 MuadDib:   Re factor of Packet 0x25, 0x11 for naming convention
 * - 2009/08/14 Turley:    PKTOUT_B9_V2 removed unk u16 and changed flag to u32
 * - 2009/09/10 Turley:    CompressedGump support (Grin)
 * - 2009/09/06 Turley:    Added more known Flags to 0xA9
 * - 2009/12/02 Turley:    PKTOUT_F3 -Tomi
 *
 * @note This file is for packet structs who sent by server.
 */

#ifndef __PKTOUTH
#define __PKTOUTH

#include "../clib/rawtypes.h"
#include "../plib/uconst.h"
#include "layers.h"
#include "pktdef.h"
#include "pktoutid.h"


namespace Pol::Core
{
#pragma pack( push, 1 )

// struct PKTOUT_0B {
//  u8 msgtype;
//  u32 serial;
//  u16 damage;
//};
// static_assert( sizeof(PKTOUT_0B) == 7, "size missmatch" );
//
// struct PKTOUT_11_V1 {
//    u8 msgtype;
//    u16 msglen;
//    u32 serial;
//    char name[30];
//    u16 hits;
//    u16 max_hits;
//    u8 renameable; // 00, or FF if can rename
//    u8 moreinfo; // is the data following present?
//    u8 gender; // GENDER_MALE or GENDER_FEMALE (see ../plib/uconst.h)
//    u16 str;
//    u16 dex;
//    u16 intel;
//
//    u16 stamina;
//    u16 max_stamina;
//
//    u16 mana;
//    u16 max_mana;
//
//    u32 gold;
//    u16 AR;
//    u16 weight;
//  // If moreinfo is 3 or 4
//  u16 statcap;
//  u8 followers;
//  u8 followers_max;
//  // If moreinfo is 4
//  u16 fireresist;
//  u16 coldresist;
//  u16 poisonresist;
//  u16 energyresist;
//  u16 luck;
//  u16 damage_min;
//  u16 damage_max;
//  u32 titching;
//};
// static_assert( sizeof( PKTOUT_11_V1 ) == 88, "size missmatch" );
//
// struct PKTOUT_11_V2 {
//    u8 msgtype;
//    u16 msglen;
//    u32 serial;
//    char name[30];
//    u16 hits;
//    u16 max_hits;
//    u8 renameable; // 00, or FF if can rename
//    u8 moreinfo; // is the data following present?
//    u8 gender; // GENDER_MALE or GENDER_FEMALE (see ../plib/uconst.h)
//    u16 str;
//    u16 dex;
//    u16 intel;
//
//    u16 stamina;
//    u16 max_stamina;
//
//    u16 mana;
//    u16 max_mana;
//
//    u32 gold;
//    u16 AR;
//    u16 weight;
//  // If moreinfo is 5 or higher (ML Introduced this mid-packet
//  u16 max_weight;
//  u8 race;
//  // If moreinfo is 3 or higher
//  u16 statcap;
//  u8 followers;
//  u8 followers_max;
//  // If moreinfo is 4 or higher
//  u16 fireresist;
//  u16 coldresist;
//  u16 poisonresist;
//  u16 energyresist;
//  u16 luck;
//  u16 damage_min;
//  u16 damage_max;
//  u32 titching;
//};
// static_assert( sizeof( PKTOUT_11_V2 ) == 91, "size missmatch" );

// Health bar status update (KR)
// struct PKTOUT_17 {
//  u8 msgtype;
//  u16 msglen;
//  u32 serial;
//  u16 unk;
//  u16 status_type;
//  u8 flag;
//};
// static_assert( sizeof(PKTOUT_17) == 12, "size missmatch" );

// struct PKTOUT_1A_A {
//    u8 msgtype;
//    u16 msglen;
//    u32 serial; // bit 0x80000000 enables piles
//    u16 graphic;
//    u16 amount;
//    u16 x;
//    u16 y; // bits 0x80 and 0x40 are Dye and Move (dunno which is which)
//    u8 z;
//    u16 color;
//    u8 flags;
//};
// static_assert( sizeof(PKTOUT_1A_A) == 19, "size missmatch" );

// use message type B if facing of an item is nonzero
// struct PKTOUT_1A_B {
//    u8 msgtype;
//    u16 msglen;
//    u32 serial; // bit 0x80000000 enables piles
//    u16 graphic;
//    u16 amount;
//    u16 x;          // set bit 0x8000 for this msg type
//    u16 y; // bits 0x80 and 0x40 are Dye and Move (dunno which is which)
//    u8 facing;
//    u8 z;
//    u16 color;
//    u8 flags;
//};
// static_assert( sizeof(PKTOUT_1A_B) == 20, "size missmatch" );

// struct PKTOUT_1A_C {
//    u8 msgtype;
//    u16 msglen;
//    u32 serial;
//    u16 graphic;
//    u16 x;
//    u16 y;
//    u8 z;
//};
// static_assert( sizeof(PKTOUT_1A_C) == 0x0E, "size missmatch" );

// use message type D when?
struct PKTOUT_1A_D
{
  u8 msgtype;
  u16 msglen;
  u32 serial;  // bit 0x80000000 enables piles
  u16 graphic;
  u16 amount;  // or is this color?
  u16 x;
  u16 y;  // bits 0x80 and 0x40 are Dye and Move (dunno which is which)
  u8 z;
};
static_assert( sizeof( PKTOUT_1A_D ) == 0x10, "size missmatch" );

// Character Startup - Login actually completed.
// struct PKTOUT_1B {
//    u8 msgtype;
//    u32 serial;
//    u8 unk5, unk6, unk7, unk8;
//    u16 graphic;
//    u16 x;
//    u16 y;
//    u8 unk_15;
//    u8 z;
//    u8 dir;
//    u8 unk18;
//    u8 unk19;
//    u8 unk20;
//    u8 unk21_7F;
//    u8 unk22;
//
//    u16 map_startx;
//    u16 map_starty;
//    u16 map_width;
//  u16 map_height;
//    u8 unk31;
//    u8 unk32;
//    u8 unk33;
//    u8 unk34;
//    u8 unk35;
//    u8 unk36;
//};
// static_assert( sizeof(PKTOUT_1B) == 37, "size missmatch" );

// Send ASCII Speech
// struct PKTOUT_1C {
//    u8 msgtype;
//    u16 msglen;
//    u32 source_serial;
//    u16 source_graphic;
//    u8 type;
//    u16 color;
//    u16 font;
//    char speaker_name[ 30 ];
//    char text[ SPEECH_MAX_LEN + 1 ];
//};
// static_assert( sizeof(PKTOUT_1C) == 44 + SPEECH_MAX_LEN + 1, "size missmatch" );

// struct PKTOUT_1D {
//    u8 msgtype;
//    u32 serial;
//};
// static_assert( sizeof(PKTOUT_1D) == 5, "size missmatch" );

struct PKTOUT_1F
{
  u8 msgtype;
  u8 unk1_7[7];
};
static_assert( sizeof( PKTOUT_1F ) == 8, "size missmatch" );

// struct PKTOUT_20 {
//    u8 msgtype;
//    u32 serial;
//    u16 graphic;
//    u8 unk7;
//    u16 color;
//    u8 flag1;
//    u16 x;
//    u16 y;
//    u8 unk15;
//    u8 unk16;
//    u8 dir;     // NOTE: bit 0x80 seems to always be set.
//    u8 z;
//    /*
//    enum {
//        FLAG_NORMAL = 0x00,
//        FLAG_WARMODE = 0x40,
//        FLAG_HIDDEN = 0x80
//    };
//    */
//};
// static_assert( sizeof(PKTOUT_20) == 19, "size missmatch" );

// struct PKTOUT_21 {
//    u8 msgtype;
//    u8 sequence;
//    u16 x;
//    u16 y;
//    u8 facing;
//    u8 z;
//};
// static_assert( sizeof(PKTOUT_21) == 8, "size missmatch" );

struct PKTOUT_23
{
  u8 msgtype;
  u16 itemid;
  u8 zero1;
  u16 zero2;
  u16 amount;  // item amount
  u32 src_serial;
  u16 src_x;
  u16 src_y;
  s8 src_z;
  u32 tgt_serial;
  u16 tgt_x;
  u16 tgt_y;
  s8 tgt_z;
};
static_assert( sizeof( PKTOUT_23 ) == 26, "size missmatch" );

// NOTE: Msg Type 24 followed by message type 3C.
// (for opening containers)
// struct PKTOUT_24 {
//    u8 msgtype;
//    u32 serial;
//    u16 gump; // ugh. 0x0009 is the corpse gump...
//};
// static_assert( sizeof(PKTOUT_24) == 7, "size missmatch" );

// struct PKTOUT_25_V1 {
//    u8 msgtype;
//    u32 serial;
//    u16 graphic;
//    u8 unk7; // layer?
//    u16 amount; // ??
//    u16 x;
//    u16 y;
//    u32 container_serial;
//    u16 color;
//};
// static_assert( sizeof( PKTOUT_25_V1 ) == 20, "size missmatch" );
//
// struct PKTOUT_25_V2 {
//    u8 msgtype;
//    u32 serial;
//    u16 graphic;
//    u8 unk7; // layer?
//    u16 amount; // ??
//    u16 x;
//    u16 y;
//  u8 slotindex;
//    u32 container_serial;
//    u16 color;
//};
// static_assert( sizeof( PKTOUT_25_V2 ) == 21, "size missmatch" );

struct PKTOUT_26
{
  u8 msgtype;
  u32 gmserial;
};
static_assert( sizeof( PKTOUT_26 ) == 5, "size missmatch" );

// struct PKTOUT_27 {
//    u8 msgtype;
//    u8 reason;
//};
// static_assert( sizeof(PKTOUT_27) == 2, "size missmatch" );

struct PKTOUT_28
{
  u8 msgtype;
  u32 serial;
};
static_assert( sizeof( PKTOUT_28 ) == 5, "size missmatch" );

// struct PKTOUT_29 {
//  u8 msgtype;
//};
// static_assert( sizeof(PKTOUT_29) == 1, "size missmatch" );

struct PKTOUT_2A
{
  u8 msgtype;
  u32 serial;
};
static_assert( sizeof( PKTOUT_2A ) == 5, "size missmatch" );

struct PKTOUT_2B
{
  u8 msgtype;
  u8 mode;  // on/off. 0/1?
};
static_assert( sizeof( PKTOUT_2B ) == 2, "size missmatch" );

// struct PKTOUT_2D {
//  u8 msgtype;
//  u32 serial;
//  u16 hitsmax;
//  u16 hitscurrent;
//  u16 manamax;
//  u16 manacurrent;
//  u16 stammax;
//  u16 stamcurrent;
//};
// static_assert( sizeof(PKTOUT_2D) == 17, "size missmatch" );

// struct PKTOUT_2E {
//    u8 msgtype;
//    u32 serial;
//    u16 graphic;
//    u8 unk7;
//    u8 layer;
//    u32 worn_by;
//    u16 color;
//};
// static_assert( sizeof(PKTOUT_2E) == 15, "size missmatch" );

// struct PKTOUT_2F {
//    u8 msgtype;
//    u8 zero1;
//    u32 attacker_serial;
//    u32 defender_serial;
//};
// static_assert( sizeof(PKTOUT_2F) == 10, "size missmatch" );

struct PKTOUT_30
{
  u8 msgtype;
  u32 unk1;
};
static_assert( sizeof( PKTOUT_30 ) == 5, "size missmatch" );

struct PKTOUT_31
{
  u8 msgtype;
};
static_assert( sizeof( PKTOUT_31 ) == 1, "size missmatch" );

struct PKTOUT_32
{
  u8 msgtype;
  u8 unk1;
};
static_assert( sizeof( PKTOUT_32 ) == 2, "size missmatch" );

struct PKTOUT_33
{
  u8 msgtype;
  u8 flow;
};

struct PKTOUT_36
{
  u8 msgtype;
  u16 msglen;
  u8 unk1[1];
};

// msg 3C: be sure to not send the whole structure, just as many 'items' as you insert
// struct PKTOUT_3C {
//    u8 msgtype;
//    u16 msglen;
//    u16 count;
//    struct {
//        u32 serial;
//        u16 graphic;
//        u8 unk6_00;
//        u16 amount;
//        u16 x;
//        u16 y;
//        u32 container_serial;
//        u16 color;
//    } items[ MAX_CONTAINER_ITEMS ];
//};
// static_assert( sizeof(PKTOUT_3C) == 5 + MAX_CONTAINER_ITEMS * 19, "size missmatch" );
//
// struct PKTOUT_3C_6017 {
//    u8 msgtype;
//    u16 msglen;
//    u16 count;
//    struct {
//        u32 serial;
//        u16 graphic;
//        u8 unk6_00;
//        u16 amount;
//        u16 x;
//        u16 y;
//    u8 slot_index;
//        u32 container_serial;
//        u16 color;
//    } items[ MAX_CONTAINER_ITEMS ];
//};
// static_assert( sizeof(PKTOUT_3C_6017) == 5 + MAX_CONTAINER_ITEMS * 20, "size missmatch" );

struct PKTOUT_3E
{
  u8 msgtype;
  u8 unk1_x24[0x24];
};
static_assert( sizeof( PKTOUT_3E ) == 0x25, "size missmatch" );

struct PKTOUT_3F
{
  u8 msgtype;
  u16 msglen;
  u8 data[1];
};

// Server->Client
struct PKTOUT_41
{
  u8 msgtype;
  u16 msglen;
  u16 tileid;
  u8 tiledata[37];
  u16 unk;  // F777
};

// Server->Client
struct PKTOUT_42
{
  u8 msgtype;
  u16 msglen;
  u8 data[1];
};

// Server->Client
struct PKTOUT_43
{
  u8 msgtype;
  u8 unk1_x28[0x28];
};
static_assert( sizeof( PKTOUT_43 ) == 0x29, "size missmatch" );

// Server->Client
struct PKTOUT_44
{
  u8 msgtype;
  u8 unk1_xC8[0xC8];
};
static_assert( sizeof( PKTOUT_44 ) == 0xC9, "size missmatch" );

struct PKTOUT_4E
{
  u8 msgtype;
  u32 mobile_serial;
  u8 lightlevel;
};
static_assert( sizeof( PKTOUT_4E ) == 6, "size missmatch" );

// struct PKTOUT_4F {
//    u8 msgtype;
//    u8 lightlevel;
//};
// static_assert( sizeof(PKTOUT_4F) == 2, "size missmatch" );

// struct PKTOUT_53
//{
//    u8 msgtype;
//    u8 warning;
//
//  enum {
//    WARN_NO_CHARACTER = 1,
//    WARN_CHARACTER_EXISTS = 2,
//    WARN_CANT_CONNECT_1 = 3,
//    WARN_CANT_CONNECT_2 = 4,
//    WARN_CHARACTER_IN_WORLD = 5,
//    WARN_LOGIN_PROBLEM = 6,
//    WARN_CHARACTER_IDLE = 7,
//    WARN_CANT_CONNECT_3 = 8
//  };
//};

// struct PKTOUT_54 {
//    u8 msgtype;
//    u8 flags; // 0 = repeating, 1 = single play
//    u16 effect; // SOUND_EFFECT_XX, see sfx.h
//    u16 volume;
//    u16 x;
//    u16 y;
//    s16 z;
//  enum { FLAG_REPEAT = 0, FLAG_SINGLEPLAY = 1 };
//};
// static_assert( sizeof(PKTOUT_54) == 12, "size missmatch" );

// struct PKTOUT_55 {
//  u8 msgtype;
//};
// static_assert( sizeof(PKTOUT_55) == 1, "size missmatch" );

struct PKTOUT_5B
{
  u8 msgtype;
  u8 hours;
  u8 minutes;
  u8 seconds;
};
static_assert( sizeof( PKTOUT_5B ) == 4, "size missmatch" );

// struct PKTOUT_65 {
//    u8 msgtype;
//    u8 type;
//    u8 severity;
//    u8 aux;
//    enum { TYPE_NONE = 0, TYPE_RAIN = 1, TYPE_SNOW = 2 };
//  enum { //TODO:FIX//
//    WTYPE_START_TO_RAIN = 0x00,
//    WTYPE_FIERCE_STORM = 0x01,
//    WTYPE_BEGIN_TO_SNOW = 0x02,
//    WTYPE_STORM_BREWING = 0x03,
//    WTYPE_STOP_WEATHER = 0xFF
//  };
//};
// static_assert( sizeof(PKTOUT_65) == 4, "size missmatch" );

// struct PKTOUT_6D
//{
//    u8 msgtype;
//    u16 midi;
//};
// static_assert( sizeof(PKTOUT_6D) == 3, "size missmatch" );

// struct PKTOUT_6E {
//    u8 msgtype;
//    u32 serial;
//    u16 action;
//    u16 framecount;
//    u16 repeatcount;
//    u8 backward;
//    u8 repeatflag;
//    u8 delay;
//    enum DIRECTION_FLAG { BACKWARD = 1,
//                       FORWARD = 0 };
//    enum REPEAT_FLAG { REPEAT = 1,
//                       NOREPEAT = 0 };
//};
// static_assert( sizeof(PKTOUT_6E) == 14, "size missmatch" );

/*
Okay, this just sucks.  If, in a client, you click on someone with
the targetting cursor, and at that same moment (or even slightly thereafter!)
that person moves, the client seems to regularly get the "move" before
registering your selection.  The end result is that you target whatever is
NOW under your cursor, rather than what WAS under it.
On the plus side, it seems you can target an effect from an x/y/z coord
to another x/y/z coordinate.
*/
// struct PKTOUT_70 {
//    u8 msgtype;
//    u8 effect_type;     // EFFECT_TYPE_xx
//    u32 source_serial;
//    u32 target_serial;
//    u16 effect;
//    u16 srcx;
//    u16 srcy;
//    u8 srcz;
//    u16 dstx;
//    u16 dsty;
//    u8 dstz;
//    u8 speed; /* was: 07 */
//    u8 loop;    /* 1=shortest, 0=very long */
//    u8 unk24;
//    u8 unk25;
//    u8 unk26;
//    u8 explode;/* not sure what to put here */
//};
// static_assert( sizeof(PKTOUT_70) == 28, "size missmatch" );

// struct PKTOUT_74 {
//  struct HEADER {
//    u8  msgtype;
//    u16 msglen;
//    u32 container;  /* container serial # */
//    u8  num_items;
//  };
//  struct ELEMENT {
//    u32 price;
//    u8 desc_len;      /* including null terminator */
//    char description[1];  /* No idea what max len should be null term string len includes null*/
//  };
//};
// static_assert( sizeof(PKTOUT_74::HEADER) == 8, "size missmatch" );
// static_assert( sizeof(PKTOUT_74::ELEMENT) == 6, "size missmatch" );

// struct PKTOUT_76
//{
//  u8 msgtype;
//  u16 xloc;
//  u16 yloc;
//  u16 zloc;
//  u8 unk0;
//  u16 x1;
//  u16 y1;
//  u16 x2;
//  u16 y2;
//};
// static_assert( sizeof( PKTOUT_76 ) == 16, "size missmatch" );

// struct PKTOUT_77 {
//    u8 msgtype;
//    u32 serial;
//    u16 graphic;
//    u16 x;
//    u16 y;
//    u8 z;
//    u8 dir;
//    u16 skin;
//    u8 flag1; /* bit 0x80 seems to be invis (and self hidden), bit 0x40 is war mode */
//    u8 hilite; /* only 1 to 7 seem to be valid. any other value in the entire byte causes white
//    highlighting */
//};
// static_assert( sizeof(PKTOUT_77) == 17, "size missmatch" );

// struct PKTOUT_78 {
//    u8 msgtype;
//    u16 msglen;
//    u32 serial;
//    u16 graphic;
//    u16 x;
//    u16 y;
//    u8  z;
//    u8  facing;     /* WALK_DIR_x */
//    u16 skin;       /* skin color? */
//    u8 flag1;
//    u8 hilite;
//    u8 wornitems[ 9 * HIGHEST_LAYER + 4 ];
//};
//
// struct PKTOUT_78_NOCOLOR {
//    u32 serial;
//    u16 graphic;
//    u8 layer;
//};
// static_assert( sizeof( PKTOUT_78_NOCOLOR ) == 7, "size missmatch" );
//
// struct PKTOUT_78_COLOR {
//    u32 serial;
//    u16 graphic;
//    u8 layer;
//    u16 color;
//};
// static_assert( sizeof( PKTOUT_78_COLOR ) == 9, "size missmatch" );
/*
  FIXME: msg type 78 is incompletely understood.  I believe item/clothing data can be sent in it.
  NOTE on wornitems: each layer is sent.  The format seems to be:
  SERIAL OBJTYPE LAYER {Optional Color}
  4      2        1       2
  when there are no more, inject 4 zeros into wornitems.
  I think the high bit of OBJTYPE indicates whether or not color is present.
  For our purposes, if the item has a nonzero color, we will send it.
  I'm not sure if the tiledata has a bit to indicate whether color goes along or not.

  */

// struct PKTOUT_7C {
//    u8 msgtype;
//    u16 msglen;
//    u32 used_item_serial;
//    u16 menu_id;
//};
//
// struct PKTOUT_7C_TITLE {
//    u8 titlelen;
//    char title[ 80 ]; /* don't know what this should be */
//};
//
// struct PKTOUT_7C_COUNT {
//    u8 item_count;
//};
//
// struct PKTOUT_7C_ELEM {
//    u16 graphic;
//    u16 color; // ?? they were all grey.
//    u8 desclen;
//    char desc[ 80 ]; // also not sure what the max is
//};

// struct PKTOUT_82 {
//    u8 msgtype;
//    u8 error;
//};
// static_assert( sizeof(PKTOUT_82) == 2, "size missmatch" );

struct PKTOUT_86
{
  u8 msgtype;
  u16 msglen;
  u8 numchars;
};
static_assert( sizeof( PKTOUT_86 ) == 4, "size missmatch" );

struct PKTOUT_86_CHARACTERS
{
  char name[30];
  char password[30];
};
static_assert( sizeof( PKTOUT_86_CHARACTERS ) == 60, "size missmatch" );

// struct PKTOUT_88 {
//    u8 msgtype;
//    u32 serial;
//    u8 text[ 60 ];
//    u8 flag1;
//};
// static_assert( sizeof(PKTOUT_88) == 66, "size missmatch" );

// after all layers inserted, set the 'layer' value
// in the next array elem to 0.  length should include this byte.
// Example, for an empty corpse: 89 00 08 40 75 13 f9 00
// struct PKTOUT_89 {
//    u8 msgtype;
//    u16 msglen;
//    u32 serial;
//    struct {
//        u8 layer;
//        u32 serial;
//    } layers[ NUM_LAYERS +1 ];
//};

// struct PKTOUT_8C {
//    u8 msgtype;
//    u8 ip[4];
//    u16 port;
//    u8 unk7_00;
//    u8 unk8_03;
//    u16 unk9_10_ClientType;
//};
// static_assert( sizeof(PKTOUT_8C) == 11, "size missmatch" );

// struct PKTOUT_90
//{
//    u8 msgtype;
//    u32 serial;
//    u8 unk5_13;
//    u8 unk6_9d;
//    u16 x1;
//    u16 y1;
//    u16 x2;
//    u16 y2;
//    u16 xsize;
//    u16 ysize;
//};
// static_assert( sizeof(PKTOUT_90) == 19, "size missmatch" );

struct PKTOUT_97
{
  u8 msgtype;
  u8 direction;
};
static_assert( sizeof( PKTOUT_97 ) == 2, "size missmatch" );

struct PKTOUT_9C
{
  u8 msgtype;
  char response[52];  // Unknown at this time.
};

// struct PKTOUT_9E {
//  struct HEADER {
//    u8 msgtype;
//    u16 msglen;
//    u32 vendor_serial;
//    u16 num_items;
//  };
//  struct ELEMENT {
//    u32 serial;
//    u16 graphic;
//    u16 color;
//    u16 amount;
//    u16 price;
//    u16 desc_len;
//    char description[1];
//  };
//};
// static_assert( sizeof(PKTOUT_9E::HEADER) == 9, "size missmatch" );
// static_assert( sizeof(PKTOUT_9E::ELEMENT) == 15, "size missmatch" );

// struct PKTOUT_A1
//{
//    u8 msgtype;
//    u32 serial;
//    u16 max_hits; // str
//    u16 hits;
//};
// static_assert( sizeof(PKTOUT_A1) == 9, "size missmatch" );

// struct PKTOUT_A2 {
//    u8 msgtype;
//    u32 serial;
//    u16 max_mana; // intel;
//    u16 mana;
//};
// static_assert( sizeof(PKTOUT_A2) == 9, "size missmatch" );

// struct PKTOUT_A3 {
//    u8 msgtype;
//    u32 serial;
//    u16 max_stamina; // dex;
//    u16 stamina;
//};
// static_assert( sizeof(PKTOUT_A3) == 9, "size missmatch" );

// struct PKTOUT_A5 {
//    u8 msgtype;
//    u16 msglen;
//  char address[ URL_MAX_LEN + 1 ];
//  u8 null_term;
//};

// struct PKTOUT_A6 {
//    u8 msgtype;
//    u16 msglen;
//    u8 type;
//    u8 unk4;
//    u8 unk5;
//    u16 tipnum;
//    u16 textlen;
//    char text[10000];
//    enum { TYPE_TIP = 0,
//           TYPE_UPDATE = 1 };
//};
// static_assert( sizeof(PKTOUT_A6) == 10010, "size missmatch" );

// struct PKTOUT_A8 {
//    u8 msgtype;
//    u16 msglen;
//    u8 unk3_FF;
//    u16 servcount;
//};
// static_assert( sizeof(PKTOUT_A8) == 6, "size missmatch" );
//
// struct PKTOUT_A8_SERVER {
//    u16 servernum;
//    char servername[30]; // think the last two bytes are the sorting amounts.
//    u16 servernum2;
//    u8 percentfull;
//    s8 timezone;
//    char ip[4];
//};
// static_assert( sizeof(PKTOUT_A8_SERVER) == 40, "size missmatch" );

// struct PKTOUT_A9 {
//    u8 msgtype;
//    u16 msglen;
//    u8 numchars;
//};
// static_assert( sizeof(PKTOUT_A9) == 4, "size missmatch" );
//
//// I don't think all of this is the name.  At on point, some garbage was
//// being left in the structure, and a password was prompted for - seems
//// one of these bytes is a password flag.
// struct PKTOUT_A9_CHARACTERS {
//    char name[60];
//};
// static_assert( sizeof(PKTOUT_A9_CHARACTERS) == 60, "size missmatch" );
//
// struct PKTOUT_A9_START_LEN {
//    u8 startcount;
//};
// static_assert( sizeof(PKTOUT_A9_START_LEN) == 1, "size missmatch" );
//
// struct PKTOUT_A9_START_ELEM {
//    u8 startnum;
//    char city[31];
//    char desc[31];
//};
// static_assert( sizeof(PKTOUT_A9_START_ELEM) == 63, "size missmatch" );
//
// struct PKTOUT_A9_START_FLAGS {
//  u32 flags;
//  enum {
//    FLAG_UNK_0x0001             = 0x0001, // ?
//    FLAG_SEND_CONFIG_REQ_LOGOUT = 0x0002, // Sends Configuration to Server
//    FLAG_SINGLE_CHARACTER_SIEGE = 0x0004, // ?
//    FLAG_ENABLE_NPC_POPUP_MENUS = 0x0008, // Enable NPC PopUp Menus
//    FLAG_SINGLE_CHARACTER       = 0x0010, // Use 1 Character only
//    FLAG_AOS_FEATURES           = 0x0020, // Age of Shadows
//    FLAG_UPTO_SIX_CHARACTERS    = 0x0040, // Use up to 6 Characters (not only 5)
//    FLAG_SE_FEATURES            = 0x0080, // Samurai Empire
//    FLAG_ML_FEATURES            = 0x0100, // Mondain's Legacy
//    FLAG_SEND_UO3D_TYPE         = 0x0400, // KR and UO:SA will send 0xE1 packet
//    FLAG_UNK_0x0800             = 0x0800, // ?
//    FLAG_UPTO_SEVEN_CHARACTERS  = 0x1000, // Use up to 7 Characters
//    FLAG_UNK_0x2000             = 0x2000  // ?
//  };
//};

/* AA message - Sets Opponent - the always-highlighted being.
    Setting to 0 clears.
    Sent to you when you die (serial=0), or when others target you if you
    have noone targetted.
    */
// struct PKTOUT_AA {
//    u8 msgtype;
//    u32 serial;
//};
// static_assert( sizeof(PKTOUT_AA) == 5, "size missmatch" );

// struct PKTOUT_AB
//{
//    struct HEADER {
//        u8 msgtype;
//        u16 msglen;
//        u32 serial;
//        u8 type;
//        u8 index;
//    };
//    struct TEXTLINE1 {
//        u16 numbytes;
//        char text[256]; // null-terminated
//    };
//    struct TEXTLINE2 {
//        u8 cancel; enum { CANCEL_DISABLE, CANCEL_ENABLE };
//        u8 style; enum { STYLE_DISABLE, STYLE_NORMAL, STYLE_NUMERICAL };
//        u32 mask;
//        u16 numbytes;
//        char text[256]; // null-terminated
//    };
//
//};
// static_assert( sizeof(PKTOUT_AB::HEADER) == 9, "size missmatch" );
// static_assert( sizeof(PKTOUT_AB::TEXTLINE1) == 258, "size missmatch" );
// static_assert( sizeof(PKTOUT_AB::TEXTLINE2) == 264, "size missmatch" );

// struct PKTOUT_AE
//{
//    u8 msgtype;
//    u16 msglen;
//    u32 source_serial;
//    u16 source_graphic;
//    u8 type;
//    u16 color;
//    u16 font;
//    char lang[4]; // "enu" - US english
//    char speaker_name[ 30 ];
//    u16 wtext[(SPEECH_MAX_LEN) + 1]; // wide-character, double-null terminated
//};
// static_assert( sizeof( PKTOUT_AE ) == 450, "size missmatch" );

// struct PKTOUT_AF {
//    u8 msgtype;
//    u32 player_id;
//    u32 corpse_id;
//    u32 unk4_zero; // 0x00000000
//};
// static_assert( sizeof(PKTOUT_AF) == 13, "size missmatch" );

// struct PKTOUT_B0
//{
//// one of these:
//    struct HEADER {
//        u8 msgtype;
//        u16 msglen;
//        u32 serial;
//        u32 dialogid;
//        u32 x;
//        u32 y;
//    };
//// followed by one of these (variable length):
//    struct LAYOUT {
//        u16 len;
//        char text[ 1 ]; // null-terminated
//    };
//// followed by one of these:
//    struct DATA_HEADER {
//        u16 numlines;
//    };
//// followed by zero or more (numlines, above) of these:
//    struct DATA {
//        u16 len;
//        char text[ 1 ];
//    };
//};
// static_assert( sizeof(PKTOUT_B0::HEADER) == 19, "size missmatch" );
// static_assert( sizeof(PKTOUT_B0::LAYOUT) == 3, "size missmatch" );
// static_assert( sizeof(PKTOUT_B0::DATA_HEADER) == 2, "size missmatch" );
// static_assert( sizeof(PKTOUT_B0::DATA) == 3, "size missmatch" );

// struct PKTOUT_B7 {
//    u8 msgtype;
//    u16 msglen;
//    u32 serial;
//    enum { MAX_CHARACTERS=256 };
//    char text[MAX_CHARACTERS*2+2]; // null-terminated unicode string
//};
// static_assert( sizeof(PKTOUT_B7) == 521, "size missmatch" );

// struct PKTOUT_B9 {
//    u8 msgtype;
//    u16 enable;
//    /*
//    if (MSB not set)
//      Bit 0 T2A upgrade, enables chatbutton,
//      Bit 1 enables LBR update.  (of course LBR installation is required)
//             (plays MP3 instead of midis, 2D LBR client shows new LBR monsters,)
//    if (MSB set)
//      Bit 2 T2A upgrade, enables chatbutton,
//      Bit 3 enables LBR update.
//      Bit 4 enables Age of Shadows update (AoS monsters/map (AOS installation required for that),
//              AOS skills/necro/paladin/fight book stuff works for ALL clients 4.0 and above)
//      Bit 5 enables use of up to 6 Characters (not only 5)
//      Bit 6 enables Samurai Empire update (SE map (SE installation required for that))
//    Bit 7 enables Mondain's Legacy update (Elves (ML/Gold installation required for that))
//    */
//};
// static_assert( sizeof(PKTOUT_B9) == 3, "size missmatch" );
//
// struct PKTOUT_B9_V2 {
//    u8 msgtype;
//  u32 enable;
//};
// static_assert( sizeof(PKTOUT_B9_V2) == 5, "size missmatch" );

// struct PKTOUT_BA {
//  u8 msgtype;
//  u8 active; enum { ARROW_OFF = 0, ARROW_ON = 1 };
//  u16 x_tgt;
//  u16 y_tgt;
//};
// static_assert( sizeof(PKTOUT_BA) == 6, "size missmatch" );

// struct PKTOUT_BC {
//    u8 msgtype;
//  u8 season; enum { SEASON_SPRING = 0, SEASON_SUMMER, SEASON_FALL, SEASON_WINTER,
// SEASON_DESOLATION };
//  u8 playsound; enum { PLAYSOUND_NO = 0, PLAYSOUND_YES = 1 };
//};
// static_assert( sizeof(PKTOUT_BC) == 3, "size missmatch" );

// Big-Endian for u16/32 unless otherwise noted.
// struct PKTOUT_C0
//{
//  u8 msgtype;
//  u8 type;
//  u32 source_serial;
//  u32 target_serial;
//  u16 itemid;
//  u16 x_source;
//  u16 y_source;
//  u8 z_source;
//  u16 x_target;
//  u16 y_target;
//  u8 z_target;
//  u8 speed;
//  u8 duration;
//  u16 unk; //On OSI, flamestrikes are 0x0100
//  u8 fixed_direction;
//  u8 explodes;
//  u32 hue;
//  u32 render_mode;
//  enum {
//    EFFECT_MOVING  = 0x00,
//    EFFECT_LIGHTNING = 0x01,
//    EFFECT_FIXEDXYZ = 0x02,
//    EFFECT_FIXEDFROM = 0x03
//  };
//};

// Argument example:
// take number 1042762:
//"Only ~1_AMOUNT~ gold could be deposited. A check for ~2_CHECK_AMOUNT~ gold was returned to you."
// the arguments string may have "100 thousand\t25 hundred", which in turn would modify the string:
//"Only 100 thousand gold could be deposited. A check for 25 hundred gold was returned to you."
// Big-Endian for u16/32 unless otherwise noted.
// struct PKTOUT_C1 {
//  u8 msgtype;
//  u16 msglen;
//  u32 serial; //0xFFffFFff for system message
//  u16 body; //0xFFff for system message
//  u8 type; //6 lower left, 7 on player
//  u16 hue;
//  u16 font;
//  u32 msgnumber;
//  char name[30];
//  u16 arguments[(SPEECH_MAX_LEN) + 1]; // _little-endian_ Unicode string, tabs ('\t') separate the
// arguments
//};
// static_assert( sizeof( PKTOUT_C1 ) == 450, "size missmatch" );

// not used:
// Big-Endian for u16/32 unless otherwise noted.
struct PKTOUT_C4
{
  u8 msgtype;
  u32 serial;
  u8 intensity;
};

// Big-Endian for u16/32 unless otherwise noted.
// struct PKTOUT_C7
//{
//  PKTOUT_C0 header;
//  u16 effect_num; //see particleffect subdir
//  u16 explode_effect_num; //0 if no explosion
//  u16 explode_sound_num; //for moving effects, 0 otherwise
//  u32 itemid; //if target is item (type 2), 0 otherwise
//  u8 layer; //(of the character, e.g left hand, right hand,  0-5,7, 0xff: moving effect or target
// is no char)
//  u16 unk_effect; //for moving effect, 0 otherwise
//};

// Big-Endian for u16/32 unless otherwise noted.
// struct PKTOUT_CC
//{
//  u8 msgtype;
//  u16 msglen;
//  u32 serial; //0xFFffFFff for system message
//  u16 body; //0xFFff for system message
//  u8 type; //6-lowerleft, 7-on player
//  u16 hue;
//  u16 font;
//  u32 msgnumber;
//  u8 flags; //0x2 unk, 0x4 message doesn't move,
//  //(flags & 0x1) == 0 signals affix is appended to the localization string, (flags & 0x1) == 1
// signals to prepend.
//  char name[30]; //u8 name[32];
//  char affix[1]; //nullterminated, uh not just 1 byte
//  u16 arguments[1]; // _big-endian_ unicode string, tabs ('\t') seperate arguments, see 0xC1 for
// argument example
//};

struct CUSTOM_HOUSE_PLANE
{
  u32 header;  // bitpacked: TTTTZZZZ UUUUUUUU LLLLLLLL UUUULLLL
  // T : numeric type value from 0-2, determines how to process 'data'
  // Z : encoded Z offset value used in type 1 and 2, actual Z value translated from table in
  // customhouses.cpp
  // U : uncompressed length of 'data'
  // L : compressed length of 'data'
  u8 data[1];
};

struct CUSTOM_HOUSE_PLANE_BUFFER
{
  u8 planecount;
  CUSTOM_HOUSE_PLANE planes[1];
};

struct PKTOUT_D8
{
  u8 msgtype;
  u16 msglen;          // flipped
  u8 compressiontype;  // 0x03 currently supported for zlib compression type
  u8 unk;              // usually 0
  u32 serial;
  u32 revision;  // incremented whenever content changes
  u16 numtiles;
  u16 planebuffer_len;  // length of remaining packet data
  CUSTOM_HOUSE_PLANE_BUFFER buffer[1];
};

// If send clients request with 0xd6 list
// struct PKTOUT_DC {
//  u8 msgtype;
//  u32 serial;
//  u32 revision;
//};
// static_assert( sizeof(PKTOUT_DC) == 9, "size missmatch" );

// struct PKTOUT_DD {
//   struct HEADER {
//      u8  msgtype;
//      u16 msglen;
//      u32 serial;
//      u32 dialog_id;
//      u32 dialog_x;
//      u32 dialog_y;
//   };
//   struct LAYOUT {
//      u32 layout_clen;
//      u32 layout_dlen;
//      u8  layout_cdata;
//   };
//   struct TEXT {
//      u32 lines;
//      u32 text_clen;
//      u32 text_dlen;
//      u8  text_cdata;
//   };
//};
// static_assert( sizeof(PKTOUT_DD::HEADER) == 19, "size missmatch" );
// static_assert( sizeof(PKTOUT_DD::LAYOUT) == 9, "size missmatch" );
// static_assert( sizeof(PKTOUT_DD::TEXT) == 13, "size missmatch" );

// struct PKTOUT_E3 {
//  u8  msgtype;
//  u16 msglen;
//  u32 unknown_A_length; //  3 Bytes
//  u8  unknown_A[3];
//  u32 unknown_B_length; // 19 Bytes
//  u8  unknown_B[19];
//  u32 unknown_C_length; // 16 Bytes
//  u8  unknown_C[16];
//  u32 unknown_D;
//  u32 unknown_E_length; // 16 Bytes
//  u8  unknown_E[16];
//};
// static_assert( sizeof(PKTOUT_E3) == 77, "size missmatch" );

// struct PKTOUT_F3 {
//    u8 msgtype;
//    u16 unknown; // always 0x1
//  u8  datatype; // 0x00 for item and 0x02 for multi
//    u32 serial;
//    u16 graphic;
//  u8  facing; // 0x00 if multi
//    u16 amount; // 0x1 if multi
//  u16 amount_2; // 0x1 if multi
//    u16 x;
//    u16 y;
//    u8  z;
//  u8  layer; // 0x00 if multi
//    u16 color; // 0x00 if multi
//    u8  flags; // 0x00 if multi
//};
// static_assert( sizeof(PKTOUT_F3) == 24, "size missmatch" );

#pragma pack( pop )
}  // namespace Pol::Core


#endif
