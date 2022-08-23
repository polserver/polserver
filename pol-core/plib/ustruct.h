/** @file
 *
 * @par History
 */


#ifndef __USTRUCT_H
#define __USTRUCT_H

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Plib
{
#pragma pack( push, 1 )

struct USTRUCT_VERSION
{
  u32 file;
  u32 block;
  u32 filepos;
  u32 length;
  u32 unknown;
};
static_assert( sizeof( USTRUCT_VERSION ) == 20, "size missmatch" );

struct USTRUCT_STATIC
{
  u16 graphic;
  s8 x_offset;
  s8 y_offset;
  s8 z;
  u16 hue;
};
static_assert( sizeof( USTRUCT_STATIC ) == 7, "size missmatch" );

struct USTRUCT_IDX
{
  u32 offset;
  u32 length;
  u32 unknown;
};
static_assert( sizeof( USTRUCT_IDX ) == 12, "size missmatch" );

const unsigned STAIDX_CHUNK = 8;

struct USTRUCT_TILE
{
  u32 flags;
  u8 weight;
  u8 layer;
  u8 unk6;
  u8 unk7;
  u8 unk8;
  u8 unk9;
  u32 anim;
  u8 unk14;
  u8 unk15;
  u8 height;
  char name[20];

  enum
  {
    FLAG_FLOOR = 0x00000001,
    FLAG_WEAPON = 0x00000002,
    FLAG_TRANSPARENT = 0x00000004,
    FLAG_TRANSLUCENT = 0x00000008,
    FLAG_WALL = 0x00000010,
    FLAG__DAMAGING = 0x00000020,
    FLAG_BLOCKING = 0x00000040,
    FLAG_LIQUID = 0x00000080,
    // 0x00000100
    FLAG_PLATFORM = 0x00000200,
    FLAG_HALF_HEIGHT = 0x00000400,
    FLAG_STACKABLE = 0x00000800,
    FLAG_WINDOW = 0x00001000,
    FLAG_NO_SHOOT = 0x00002000,
    FLAG_DESC_NEEDS_A = 0x00004000,
    FLAG_DESC_NEEDS_AN = 0x00008000,
    FLAG__DESCRIPTIONAL = 0x00010000,
    FLAG__TRANSPARENT = 0x00020000,
    // 0x00040000,
    // 0x00080000
    FLAG__MAP = 0x00100000,
    FLAG__CONTAINER = 0x00200000,
    FLAG_EQUIPPABLE = 0x00400000,
    FLAG__LIGHTSOURCE = 0x00800000,
    FLAG_ANIMATED = 0x01000000,
    FLAG_HOVEROVER = 0x02000000,
    FLAG__WALK = 0x04000000,
    FLAG__WHOLE_BODY = 0x08000000,
    FLAG_ROOF = 0x10000000,
    FLAG_DOOR = 0x20000000,
    // 0x40000000
    // 0x80000000
    FLAG_WALKBLOCK = FLAG_BLOCKING | FLAG_PLATFORM | FLAG_HALF_HEIGHT,
    FLAG_PASSABLE_WATER = FLAG_FLOOR | FLAG_LIQUID
  };
};
static_assert( sizeof( USTRUCT_TILE ) == 37, "size missmatch" );

struct USTRUCT_TILE_HSA
{
  u32 flags;
  u32 unk;
  u8 weight;
  u8 layer;
  u8 unk6;
  u8 unk7;
  u8 unk8;
  u8 unk9;
  u32 anim;
  u8 unk14;
  u8 unk15;
  u8 height;
  char name[20];
};
static_assert( sizeof( USTRUCT_TILE_HSA ) == 41, "size missmatch" );
/* notes:
  flag1:
  bit 0x80: liquid? wet? (blood, water, water troughs, swamp,
  bit 0x40: block? (tileblock)
  bit 0x20: damaging?. palisade, bees, muck, cactus, lava, fire, spell fields, brambles, campfire,
  forged metal
  bit 0x10: unknown.
  bit 0x08: unknown.
  bit 0x04:
  bit 0x02:
  bit 0x01: floor?
  */
/*
  flag2:
  bit 0x80: should be preceded by 'an ' in description
  bit 0x40: should be preceded by 'a ' in description
  bit 0x20: unknown
  bit 0x10: unknown. windows, arches, arrow loops, curtains, barred metal doors.
  bit 0x08: stackable bit? (not sure)
  bit 0x04: divide height by 2 (ladders/stairs?)
  bit 0x02: unknown
  bit 0x01: nothing has this set!
  */
//#define UOTILE_FLAG2_HALF_HEIGHT 0x4
/*
  flag3:
  bit 0x80: lightsource?
  bit 0x40: equippable?
  bit 0x20: container?
  bit 0x10: map?
  bit 0x08:
  bit 0x04:
  bit 0x02: transparent?
  bit 0x01: descriptional?
  flag4:
  bit 0x80:
  bit 0x40:
  bit 0x20: door?
  bit 0x10:
  bit 0x08: whole-body item?
  bit 0x04: walk? (tilewalk)
  bit 0x02:
  bit 0x01:
  weight: 255 is unmoveable
  */

//#define UOBJ_FLAG2_DESC_NEEDS_AN  0x80
//#define UOBJ_FLAG2_DESC_NEEDS_A   0x40

struct USTRUCT_LAND_TILE
{
  u32 flags;
  u16 unk;  // texid
  char name[20];
};

struct USTRUCT_LAND_TILE_HSA
{
  u32 flags;
  u32 unk1;
  u16 unk;  // texid
  char name[20];
};

struct USTRUCT_MULTI_HEADER
{
  u16 elemcount;
};

struct USTRUCT_MULTI_ELEMENT
{
  u16 graphic;
  s16 x;
  s16 y;
  s16 z;
  u32 flags;
};
static_assert( sizeof( USTRUCT_MULTI_ELEMENT ) == 12, "size missmatch" );

struct USTRUCT_MULTI_ELEMENT_HSA
{
  u16 graphic;
  s16 x;
  s16 y;
  s16 z;
  u32 flags;
  u32 unk;
};
static_assert( sizeof( USTRUCT_MULTI_ELEMENT_HSA ) == 16, "size missmatch" );

struct USTRUCT_MAPINFO
{
  u16 landtile;
  s8 z;

  enum
  {
    WATER__LOWEST = 0xA8,
    WATER__HIGHEST = 0xAB
  };
};
static_assert( sizeof( USTRUCT_MAPINFO ) == 3, "size missmatch" );

struct USTRUCT_MAPINFO_BLOCK
{
  unsigned int hdr;
  USTRUCT_MAPINFO cell[8][8];
};
static_assert( sizeof( USTRUCT_MAPINFO_BLOCK ) == 196, "size missmatch" );

struct USTRUCT_POL_MAPINFO_BLOCK
{
  signed char z[8][8];
  unsigned char walkok[8];  // x=index, y=bit
  unsigned char water[8];
};
static_assert( sizeof( USTRUCT_POL_MAPINFO_BLOCK ) == 8 * 8 + 16, "size missmatch" );

#pragma pack( pop )
}  // namespace Plib
}  // namespace Pol
#endif /* __USTRUCT_H */
