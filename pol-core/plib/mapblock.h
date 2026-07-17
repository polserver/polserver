/** @file
 *
 * @par History
 *
 * @note a map block is a 64x64 cell area of a map.
 * @note Since a mapcell is 2 bytes, each map block is 8K.
 */


#ifndef PLIB_MAPBLOCK_H
#define PLIB_MAPBLOCK_H

#include <cstddef>

#include "mapcell.h"

namespace Pol::Plib
{
const unsigned MAPBLOCK_CHUNK = 8;
const unsigned MAPBLOCK_SHIFT = 3;
const unsigned MAPBLOCK_CELLMASK = 7;

// Flat index of the 8x8 block containing (x,y) in POL's realm block grids: a full
// row of blocks, then the next row (y-major). This one layout is shared by
// base.dat map blocks, the solidx1 solid-block index, and statics.dat static
// blocks -- `width` is the realm width in tiles (always a multiple of 8). The
// cell within the block is (x & MAPBLOCK_CELLMASK, y & MAPBLOCK_CELLMASK).
// Note the client's own mul files use a different, column-major block layout
// (staticblock_from_coords in staticblock.h), and maptile.dat uses 64x64 blocks
// with a rounded-up stride (maptile_index in maptile.h).
constexpr size_t realm_block_index( unsigned short x, unsigned short y, unsigned short width )
{
  return static_cast<size_t>( y >> MAPBLOCK_SHIFT ) * ( width >> MAPBLOCK_SHIFT ) +
         ( x >> MAPBLOCK_SHIFT );
}

struct MAPBLOCK
{
  MAPCELL cell[MAPBLOCK_CHUNK][MAPBLOCK_CHUNK];  // [x][y]
};
}  // namespace Pol::Plib

#endif
