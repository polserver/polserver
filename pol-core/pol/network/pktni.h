/** @file
 *
 * @par History
 * - 2009/08/02 MuadDib: Initial creation.
 *
 * @note This is where all the Not Implemented Packets belong.
 */


#ifndef __PKTNI_H
#define __PKTNI_H

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Core
{
#pragma pack( push, 1 )

struct PKTNI_3D
{
  u8 msgtype;
  u8 unk1;
};
static_assert( sizeof( PKTNI_3D ) == 2, "size missmatch" );

struct PKTNI_40
{
  u8 msgtype;
  u8 unk1_xC8[0xC8];
};
static_assert( sizeof( PKTNI_40 ) == 0xC9, "size missmatch" );

struct PKTNI_45
{
  u8 msgtype;
  u32 unk1;
};
static_assert( sizeof( PKTNI_45 ) == 5, "size missmatch" );

struct PKTNI_46
{
  u8 msgtype;
  u16 msglen;
  u32 tileid;
  u8 artdata[1];
};

struct PKTNI_47
{
  u8 msgtype;
  u16 x;
  u16 y;
  u16 artid;
  u16 width;
  u16 height;
};
static_assert( sizeof( PKTNI_47 ) == 11, "size missmatch" );

struct PKTNI_48
{
  u8 msgtype;
  u32 tileid;
  u8 frames[64];
  u8 unk1;
  u8 framecount;
  u8 frameinterval;
  u8 startinterval;
};
static_assert( sizeof( PKTNI_48 ) == 73, "size missmatch" );

struct PKTNI_49
{
  u8 msgtype;
  u32 hueid;
  u16 huevalues[32];
  u16 start;
  u16 end;
  char name[20];
};
static_assert( sizeof( PKTNI_49 ) == 93, "size missmatch" );

struct PKTNI_4A
{
  u8 msgtype;
  u32 artid;
};
static_assert( sizeof( PKTNI_4A ) == 5, "size missmatch" );

struct PKTNI_4B
{
  u8 msgtype;
  u8 unk1_8[8];
};
static_assert( sizeof( PKTNI_4B ) == 9, "size missmatch" );

struct PKTNI_4C
{
  u8 msgtype;
  u16 msglen;
  u8 data[1];
};

struct PKTNI_4D
{
  u8 msgtype;
  u16 msglen;
  u8 data[1];
};

struct PKTNI_50
{
  u8 msgtype;
  u16 msglen;
  u8 data[1];
};

struct PKTNI_51
{
  u8 msgtype;
  u16 msglen;
  u8 data[1];
};

struct PKTNI_52
{
  u8 msgtype;
  u16 msglen;
  u8 data[1];
};

struct PKTNI_57
{
  u8 msgtype;
  u8 unk1_x6D[109];
};
static_assert( sizeof( PKTNI_57 ) == 110, "size missmatch" );

struct PKTNI_58
{
  u8 msgtype;
  char areaname[40];
  u32 zero1;
  u16 x;
  u16 y;
  u16 width;
  u16 height;
  s16 z1;
  s16 z2;
  char description[40];
  u16 soundfx;
  u16 music;
  u16 soundfx_night;
  u8 dungeon;
  u16 light;
};
static_assert( sizeof( PKTNI_58 ) == 106, "size missmatch" );

struct PKTNI_59
{
  u8 msgtype;
  u16 msglen;
  u8 data[1];
};

struct PKTNI_5A
{
  u8 msgtype;
  u16 msglen;
  u8 data[1];
};

struct PKTNI_5C
{
  u8 msgtype;
  u8 unk1;
};
static_assert( sizeof( PKTNI_5C ) == 2, "size missmatch" );

struct PKTNI_5E
{
  u8 msgtype;
  u16 msglen;
  u8 data[1];
};

struct PKTNI_5F
{
  u8 msgtype;
  u8 unk1_x30[48];
};
static_assert( sizeof( PKTNI_5F ) == 49, "size missmatch" );

struct PKTNI_60
{
  u8 msgtype;
  u8 unk1_4[4];
};
static_assert( sizeof( PKTNI_60 ) == 5, "size missmatch" );

struct PKTNI_61
{
  u8 msgtype;
  u16 x;
  u16 y;
  u16 z;
  u16 id;
};
static_assert( sizeof( PKTNI_61 ) == 9, "size missmatch" );

struct PKTNI_62
{
  u8 msgtype;
  u16 old_x;
  u16 old_y;
  u16 old_z;
  u16 graphic;
  u16 z_offset;
  u16 y_offset;
  u16 x_offset;
};
static_assert( sizeof( PKTNI_62 ) == 15, "size missmatch" );

struct PKTNI_63
{
  u8 msgtype;
  u8 unk1_12[12];
};
static_assert( sizeof( PKTNI_63 ) == 13, "size missmatch" );

struct PKTNI_64
{
  u8 msgtype;
};
static_assert( sizeof( PKTNI_64 ) == 1, "size missmatch" );

#pragma pack( pop )
}  // namespace Core
}  // namespace Pol
#endif  // PKTNI_H
