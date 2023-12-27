/** @file
 *
 * @par History
 * - 2009/07/26 MuadDib:   Initial creation. This file is for packets sent by client only.
 * - 2009/08/14 Turley:    fixed definition of PKTIN_5D
 * - 2009/08/14 Turley:    fixed definition of PKTIN_00
 * - 2009/09/06 Turley:    Added PKTIN_E1
 * - 2009/12/02 Turley:    fixed definition of PKTIN_8D
 * - 2011/10/26 Tomi:      Added PKTIN_F8
 *
 * @note This file is for packet structs who sent by client.
 */


#ifndef __PKTINH
#define __PKTINH

#include "../clib/rawtypes.h"
#include "../plib/uconst.h"
#include "layers.h"
#include "pktdef.h"
#include "pktinid.h"

namespace Pol
{
namespace Core
{
#pragma pack( push, 1 )

struct PKTIN_00
{
  u8 msgtype;  // Byte 0
  u32 serial;  // Bytes 1-4
  u8 unk5;
  u8 unk6;
  u8 unk7;
  u8 unk8;
  u8 unk9;
  char Name[30];  // Bytes 10-40
  u16 unk0;
  u32 clientflag;
  u32 unk1;
  u32 logincount;
  u8 profession;
  char unk2[15];
  u8 Sex;           // Byte 70 (0 if Human Male, 1 if Human Female, 2 if Elf Male, 3 if Elf Female)
  u8 Strength;      // Byte 71
  u8 Dexterity;     // Byte 72
  u8 Intelligence;  // Byte 73
  u8 SkillNumber1;  // Byte 74
  u8 SkillValue1;   // Byte 75
  u8 SkillNumber2;  // Byte 76
  u8 SkillValue2;   // Byte 77
  u8 SkillNumber3;  // Byte 78
  u8 SkillValue3;   // Byte 79
  u16 SkinColor;    // Byte 80-81
  u16 HairStyle;    // Byte 82-83
  u16 HairColor;    // Byte 84-85
  u16 BeardStyle;   // Byte 86-87
  u16 BeardColor;   // Byte 88-89
  u8 unk90;
  u8 StartIndex;  // Byte 91 (actually a <word> with unk90)
  u8 unk92;
  u8 unk93;  // <word> with unk92
  u8 unk94;
  u8 CharNumber;  // I think (yup! <word> with unk94 -TJ)
  u8 clientip[4];
  u16 shirtcolor;
  u16 pantscolor;
};
static_assert( sizeof( PKTIN_00 ) == 104, "size missmatch" );

struct PKTIN_01
{
  u8 msgtype;
  u32 unk;  // 0xffffffff
};
static_assert( sizeof( PKTIN_01 ) == 5, "size missmatch" );

struct PKTIN_1E
{
  u8 msgtype;
  u8 unk1_3[3];
};
static_assert( sizeof( PKTIN_1E ) == 4, "size missmatch" );

/* Message Type 02, WALK
dir: low three bits are direction.
bit 0x80 is "run" bit.
*/
struct PKTIN_02
{
  u8 msgtype;
  u8 dir;
  u8 movenum;  // Sequence Number (used with fastwalk prevention!) -TJ
  u32 codes;
};

/* Message Type 03, Speech Message.  Received from client when a
character talks.
Note text[1].  Text length should be determinable from msglen:
msg->msglen - offsetof( PKTIN_03, text )
includes the length of the text as well as the null terminator
What should the valid range for textlen be?  I don't know.

SPEECH_MAX_LEN: max length, not including terminator:
A line of dots is 120 characters.  I haven't found anything
yielding more characters, but 200 characters should be safe.
*/

struct PKTIN_03
{
  u8 msgtype;
  u16 msglen;
  u8 type;
  u16 color;
  u16 font;
  char text[SPEECH_MAX_LEN + 1]; /* one or more characters, null-terminated. */
};
static_assert( sizeof( PKTIN_03 ) < MAXBUFFER, "size missmatch" );

struct PKTIN_04
{
  u8 msgtype;
  u8 mode;  // on/off. 0/1?
};
static_assert( sizeof( PKTIN_04 ) == 2, "size missmatch" );

struct PKTIN_05
{
  u8 msgtype;
  u32 serial;
};
static_assert( sizeof( PKTIN_05 ) == 5, "size missmatch" );

struct PKTIN_06
{
  u8 msgtype;
  u32 serial;  // 0x80000000 bit seems not to count.
};
static_assert( sizeof( PKTIN_06 ) == 5, "size missmatch" );


struct PKTIN_07
{
  u8 msgtype;
  u32 serial;
  u16 amount;
};
static_assert( sizeof( PKTIN_07 ) == 7, "size missmatch" );

struct PKTIN_08_V1
{
  u8 msgtype;
  u32 item_serial;
  u16 x;  // only if x & 0xFF00 = 0xFF?
  u16 y;
  s8 z;
  u32 target_serial;
};
static_assert( sizeof( PKTIN_08_V1 ) == 14, "size missmatch" );

struct PKTIN_08_V2
{
  u8 msgtype;
  u32 item_serial;
  u16 x;  // only if x & 0xFF00 = 0xFF?
  u16 y;
  s8 z;
  u8 slotindex;
  u32 target_serial;
};
static_assert( sizeof( PKTIN_08_V2 ) == 15, "size missmatch" );

struct PKTIN_09
{
  u8 msgtype;
  u32 serial;
};
static_assert( sizeof( PKTIN_09 ) == 05, "size missmatch" );

struct PKTIN_12
{
  u8 msgtype;
  u16 msglen;
  u8 submsgtype;
  u8 data[300];
};
static_assert( sizeof( PKTIN_12 ) == 304, "size missmatch" );

struct PKTIN_13
{
  u8 msgtype;
  u32 serial;
  u8 layer;
  u32 equipped_on;
};
static_assert( sizeof( PKTIN_13 ) == 10, "size missmatch" );

struct PKTIN_34
{
  u8 msgtype;
  u32 serial1;
  u8 stattype;
  u32 serial2;
};
static_assert( sizeof( PKTIN_34 ) == 10, "size missmatch" );

struct PKTIN_5D
{
  u8 msgtype;
  u32 pattern_EDEDEDED;
  char charname[30];
  u16 unk_00_1;
  u32 clientflags;
  u32 unk_00_2;
  u32 unk_login_count;
  u32 unk_00_3;
  u32 unk_00_4;
  u32 unk_00_5;
  u32 unk_00_6;
  u32 charidx;
  u32 clientip;
  enum
  {  // clientflag
    FLAG_T2A = 0x00,
    FLAG_RENAISSANCE = 0x01,
    FLAG_THIRD_DAWN = 0x02,
    FLAG_LBR = 0x04,
    FLAG_AOS = 0x08,
    FLAG_SE = 0x10,
    FLAG_SA = 0x20,
    FLAG_UO3D = 0x40,  // ?
    FLAG_RESERVED = 0x80,
    FLAG_3DCLIENT = 0x100  // old 3D client
  };
};
static_assert( sizeof( PKTIN_5D ) == 0x49, "size missmatch" );

struct PKTIN_75
{
  u8 msgtype;
  u32 serial;
  char name[30];  // always null-terminated, or at least the client is supposed to.
};
static_assert( sizeof( PKTIN_75 ) == 35, "size missmatch" );

struct PKTIN_7D
{
  u8 msgtype;
  u32 used_item_serial;
  u16 menu_id;
  u16 choice;
  u16 graphic;
  u16 color;  // ?? maybe, has always been 0
};

struct PKTIN_80
{
  u8 msgtype;
  char name[30];
  char password[30];
  u8 unk;
};
static_assert( sizeof( PKTIN_80 ) == 62, "size missmatch" );

struct PKTIN_83
{
  u8 msgtype;
  u8 password[30];
  u32 charidx;
  u32 clientip;
};
static_assert( sizeof( PKTIN_83 ) == 39, "size missmatch" );

// FIXME DO NOT USE YET. This is untested, not validated. Must find out
// correct packet structure before moving forward. Different emus and diff documents
// show different structures. wtf?
// Turley 09/12/02 should work
struct PKTIN_8D
{
  u8 msgtype;  // 0
  u16 msglen;
  u32 unk_1;          // 3-6
  u32 char_slot;      // 7-10
  char name[30];      // 11-40
  char password[30];  // 41-70
  u8 profession;      // 71
  u8 flags;           // 72
  u8 gender;          // 73
  u8 race;            // 74   0=human/1=elf/2=gargoyle
  u8 strength;        // 75
  u8 dexterity;       // 76
  u8 intelligence;    // 77
  u16 skin_color;     // 78-79
  u32 unk_2;          // 80-83 0x00000000
  u32 unk_3;          // 84-87 0x00000000
  u8 skillnumber1;    // 88
  u8 skillvalue1;     // 89
  u8 skillnumber2;    // 90
  u8 skillvalue2;     // 91
  u8 skillnumber3;    // 92
  u8 skillvalue3;     // 93
  u8 skillnumber4;    // 94
  u8 skillvalue4;     // 95
  char unk_4[25];     // 96-121 all 00
  u8 unk_5;           // 122 Always 0x0B?
  u16 haircolor;      // 133-134
  u16 hairstyle;      // 135-136
  u8 unk_6;           // 137 Always 0x0C?
  u32 unk_7;          // 138-141 0x00000000
  u8 unk_8;           // 142 Always 0x0D?
  u16 shirtcolor;     // 143-144
  u16 pantscolor;     // 145-146
  u8 unk_9;           // 147 Always 0x0F?
  u16 face_color;     // 148-149
  u16 face_id;        // 150-151
  u8 unk_10;          // 152
  u16 beardcolor;     // 153
  u16 beardstyle;     // 154
};
static_assert( sizeof( PKTIN_8D ) == 146, "size missmatch" );

struct PKTIN_91
{
  u8 msgtype;
  u8 unk1, unk2;
  u16 unk3_4_ClientType;
  char name[30];
  char password[30];
};
static_assert( sizeof( PKTIN_91 ) == 65, "size missmatch" );

struct PKTIN_9B
{
  u8 msgtype;
  u8 unk[0x101];
};
static_assert( sizeof( PKTIN_9B ) == 258, "size missmatch" );

struct PKTIN_9F
{
  u8 msgtype;
  u16 msglen;
  u32 vendor_serial;
  u16 num_items;
  struct
  {
    u32 serial;
    u16 amount;
  } items[( MAXBUFFER - 9 ) / 6];
};
static_assert( sizeof( PKTIN_9F ) == 2559, "size missmatch" );
static_assert( sizeof( PKTIN_9F ) < MAXBUFFER, "size missmatch" );

struct PKTIN_A0
{
  u8 msgtype;
  u16 servernum;
};
static_assert( sizeof( PKTIN_A0 ) == 3, "size missmatch" );

struct PKTIN_A4
{
  u8 msgtype;
  u8 unknown[0x94];
};
static_assert( sizeof( PKTIN_A4 ) == 0x95, "size missmatch" );

struct PKTIN_A7
{
  u8 msgtype;
  u16 lasttip;
  u8 prevnext;  // 0=prev 1=next
};
static_assert( sizeof( PKTIN_A7 ) == 4, "size missmatch" );

struct PKTIN_AC
{
  u8 msgtype;
  u16 msglen;
  u32 serial;
  u8 type;
  u8 index;
  u8 retcode;
  enum
  {
    RETCODE_CANCELED,
    RETCODE_OKAY
  };
  u16 datalen;
  char data[MAXBUFFER - 12];
};
static_assert( sizeof( PKTIN_AC ) == MAXBUFFER, "size missmatch" );

struct PKTIN_AD
{
  u8 msgtype;
  u16 msglen;
  u8 type;
  u16 color;
  u16 font;
  char lang[4];                       // "enu" - US English
  u16 wtext[( MAXBUFFER - 12 ) / 2];  // wide-character, double-null terminated
};
static_assert( sizeof( PKTIN_AD ) == MAXBUFFER, "size missmatch" );

struct PKTIN_B1
{
  struct HEADER
  {
    u8 msgtype;
    u16 msglen;
    u32 serial;
    u32 dialogid;  // Alias for gumpid
    u32 buttonid;
  };
  struct INTS_HEADER
  {
    u32 count;
  };
  struct INT_ENTRY  // FIXME bad name
  {
    u32 value;
  };
  struct STRINGS_HEADER
  {
    u32 count;
  };
  struct STRING_ENTRY
  {
    u16 tag;
    u16 length;
    u16 data[1];
  };
};
static_assert( sizeof( PKTIN_B1::HEADER ) == 15, "size missmatch" );
static_assert( sizeof( PKTIN_B1::INTS_HEADER ) == 4, "size missmatch" );
static_assert( sizeof( PKTIN_B1::INT_ENTRY ) == 4, "size missmatch" );
static_assert( sizeof( PKTIN_B1::STRINGS_HEADER ) == 4, "size missmatch" );
static_assert( sizeof( PKTIN_B1::STRING_ENTRY ) == 6, "size missmatch" );

struct PKTIN_B5
{
  u8 msgtype;
  char chatname[63];  // in unicode
};
static_assert( sizeof( PKTIN_B5 ) == 64, "size missmatch" );

struct PKTIN_B6
{
  u8 msgtype;
  u32 serial;
  u8 langid;
  char lang[3];
};
static_assert( sizeof( PKTIN_B6 ) == 9, "size missmatch" );

struct PKTIN_D9
{
  u8 msgtype;
  u8 unknown1;                // Unknown - allways 0x02
  u32 instance;               // Unique Instance ID of UO
  u32 os_major;               // OS Major
  u32 os_minor;               // OS Minor
  u32 os_revision;            // OS Revision
  u8 cpu_manufacturer;        // CPU Manufacturer
  u32 cpu_family;             // CPU Family
  u32 cpu_model;              // CPU Model
  u32 cpu_clockspeed;         // CPU Clock Speed [Mhz]
  u8 cpu_quantity;            // CPU Quantity
  u32 memory;                 // Memory [MB]
  u32 screen_width;           // Screen Width
  u32 screen_height;          // Screen Height
  u32 screen_depth;           // Screen Depth [Bit]
  u16 directx_major;          // DirectX Major
  u16 directx_minor;          // DirectX Minor
  u16 video_description[64];  // Video Card Description [wide-character]
  u32 video_vendor;           // Video Card Vendor ID
  u32 video_device;           // Video Card Device ID
  u32 video_memory;           // Video Card Memory [MB]
  u8 distribution;            // Distribution
  u8 clients_running;         // Clients Running
  u8 clients_installed;       // Clients Installed
  u8 partial_installed;       // Partial Insstalled
  u16 langcode[4];            // Language Code [wide-character]
  u8 unknown2[64];            // Unknown
};
static_assert( sizeof( PKTIN_D9 ) == 0x10C, "size missmatch" );

struct PKTIN_E1
{
  u8 msgtype;
  u16 msglen;
  u16 unknown_0x01;
  u32 clienttype;
  enum
  {
    CLIENTTYPE_KR = 0x2,
    CLIENTTYPE_SA = 0x3
  };
};
static_assert( sizeof( PKTIN_E1 ) == 0x09, "size missmatch" );

struct PKTIN_E4
{
  u8 msgtype;
  u16 msglen;
  u32 unknown_length;  // 16 Bytes
  u8 unknown[RESPONSE_MAX_LEN];
};
static_assert( sizeof( PKTIN_E4 ) == 207, "size missmatch" );

struct PKTIN_EF
{
  u8 msgtype;
  u32 seed;
  u32 ver_Major;
  u32 ver_Minor;
  u32 ver_Revision;
  u32 ver_Patch;
};
static_assert( sizeof( PKTIN_EF ) == 0x15, "size missmatch" );

struct PKTIN_F8
{
  u8 msgtype;  // Byte 0
  u32 serial;  // Bytes 1-4
  u8 unk5;
  u8 unk6;
  u8 unk7;
  u8 unk8;
  u8 unk9;
  char Name[30];  // Bytes 10-40
  u16 unk0;
  u32 clientflag;
  u32 unk1;
  u32 logincount;
  u8 profession;
  char unk2[15];
  u8 Sex;           // Byte 70 (0 if Human Male, 1 if Human Female, 2 if Elf Male, 3 if Elf Female)
  u8 Strength;      // Byte 71
  u8 Dexterity;     // Byte 72
  u8 Intelligence;  // Byte 73
  u8 SkillNumber1;  // Byte 74
  u8 SkillValue1;   // Byte 75
  u8 SkillNumber2;  // Byte 76
  u8 SkillValue2;   // Byte 77
  u8 SkillNumber3;  // Byte 78
  u8 SkillValue3;   // Byte 79
  u8 SkillNumber4;  // Byte 80
  u8 SkillValue4;   // Byte8 1
  u16 SkinColor;    // Byte 82-83
  u16 HairStyle;    // Byte 84-85
  u16 HairColor;    // Byte 86-87
  u16 BeardStyle;   // Byte 88-89
  u16 BeardColor;   // Byte 90-91
  u8 unk90;
  u8 StartIndex;  // Byte 93 (actually a <word> with unk90)
  u8 unk92;
  u8 unk93;  // <word> with unk92
  u8 unk94;
  u8 CharNumber;  // I think (yup! <word> with unk94 -TJ)
  u8 clientip[4];
  u16 shirtcolor;
  u16 pantscolor;
};
static_assert( sizeof( PKTIN_F8 ) == 106, "size missmatch" );

struct PKTIN_FA
{
  u8 msgtype;  // Byte 0
};
static_assert( sizeof( PKTIN_FA ) == 1, "size missmatch" );

struct PKTIN_FB
{
  u8 msgtype;  // Byte 0
  u8 show;     // Bytes 1
};
static_assert( sizeof( PKTIN_FB ) == 2, "size missmatch" );

#pragma pack( pop )
}  // namespace Core
}  // namespace Pol
#endif
