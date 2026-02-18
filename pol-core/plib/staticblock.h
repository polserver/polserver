/** @file
 *
 * @par History
 * - 2005/06/01 Shinigami: added StaticEntryList for use with getstatics - to fill a list with
 * statics
 */


#ifndef PLIB_STATICBLOCK_H
#define PLIB_STATICBLOCK_H

#include "../clib/rawtypes.h"

#include <utility>
#include <vector>


namespace Pol::Plib
{
struct STATIC_INDEX
{
  u32 index;
};

struct STATIC_ENTRY
{
  u16 objtype;
  u8 xy;  // high nibble: x, low nibble: y (0x80, 0x40 bits unused)
  s8 z;
  u16 hue;
};

class StaticEntryList : public std::vector<STATIC_ENTRY>
{
};

const unsigned STATICBLOCK_CHUNK = 8;
const unsigned STATICBLOCK_SHIFT = 3;
const unsigned STATICCELL_MASK = 0x7;

constexpr size_t staticblock_from_coords( u16 x, u16 y, u16 map_height )
{
  return ( x / STATICBLOCK_CHUNK ) * ( map_height / STATICBLOCK_CHUNK ) + ( y / STATICBLOCK_CHUNK );
}

constexpr std::pair<u16, u16> staticblock_to_coords( size_t block, u16 map_height )
{
  return { block / ( map_height / STATICBLOCK_CHUNK ) * STATICBLOCK_CHUNK,
           ( block % ( map_height / STATICBLOCK_CHUNK ) ) * STATICBLOCK_CHUNK };
}
static_assert( staticblock_to_coords( staticblock_from_coords( 96, 200, 4096 ), 4096 ) ==
               std::make_pair( 96_u16, 200_u16 ) );

}  // namespace Pol::Plib

#endif
