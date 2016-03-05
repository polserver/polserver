/** @file
 *
 * @par History
 */


#ifndef PLIB_MAPTILE_H
#define PLIB_MAPTILE_H
namespace Pol
{
namespace Plib
{
const unsigned MAPTILE_CHUNK = 64;
const unsigned MAPTILE_SHIFT = 6;
const unsigned MAPTILE_CELLMASK = 0x3f;

#ifdef _MSC_VER     /* Visual C++ 4.0 and above */
#pragma pack( push, 1 )
#else  // GCC most likely
#pragma pack(1)
#endif

struct MAPTILE_CELL
{
  unsigned short landtile;
  signed char z;
};

struct MAPTILE_BLOCK
{
  MAPTILE_CELL cell[MAPTILE_CHUNK][MAPTILE_CHUNK];
};

#ifdef _MSC_VER     /* Visual C++ 4.0 + */
#pragma pack( pop )
#else
#pragma pack()
#endif
}
}
#endif
