/** @file
 *
 * @par History
 *
 * @note a map block is a 64x64 cell area of a map.
 * @note Since a mapcell is 2 bytes, each map block is 8K.
 */


#ifndef PLIB_MAPBLOCK_H
#define PLIB_MAPBLOCK_H

#include "mapcell.h"
namespace Pol
{
namespace Plib
{
const unsigned MAPBLOCK_CHUNK = 64;
const unsigned MAPBLOCK_SHIFT = 6; // 6 bits
const unsigned MAPBLOCK_CELLMASK = 0x3F;

struct MAPBLOCK
{
  MAPCELL cell[MAPBLOCK_CHUNK][MAPBLOCK_CHUNK]; // [x][y]
};
}
}
#endif
