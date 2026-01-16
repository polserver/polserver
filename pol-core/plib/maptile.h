/** @file
 *
 * @par History
 */


#ifndef PLIB_MAPTILE_H
#define PLIB_MAPTILE_H

namespace Pol::Plib
{
const unsigned MAPTILE_CHUNK = 64;
const unsigned MAPTILE_SHIFT = 6;
const unsigned MAPTILE_CELLMASK = 0x3f;

#pragma pack( push, 1 )

struct MAPTILE_CELL
{
  unsigned short landtile;
  signed char z;
};

struct MAPTILE_BLOCK
{
  MAPTILE_CELL cell[MAPTILE_CHUNK][MAPTILE_CHUNK];
};

#pragma pack( pop )
}  // namespace Pol::Plib

#endif
