/** @file
 *
 * @par History
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#ifndef TILES_H
#define TILES_H

#include <string>

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Core
{
class Tile
{
public:
  std::string desc;
  unsigned int uoflags;  // USTRUCT_TILE::*
  unsigned int flags;    // FLAG::*
  u8 layer;
  u8 height;
  u8 weight;  // todo mult, div
};
}
}
#endif
