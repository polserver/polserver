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

// number of MAPTILE_CHUNK-sized block columns/rows needed to cover a
// dimension of size w; unlike w >> MAPTILE_SHIFT this rounds up, so it's
// correct for map dimensions that aren't a multiple of MAPTILE_CHUNK.
constexpr unsigned maptile_blocks_across( unsigned w )
{
  return ( w + MAPTILE_CHUNK - 1 ) / MAPTILE_CHUNK;
}

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
