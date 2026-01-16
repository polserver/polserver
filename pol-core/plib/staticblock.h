/** @file
 *
 * @par History
 * - 2005/06/01 Shinigami: added StaticEntryList for use with getstatics - to fill a list with
 * statics
 */


#ifndef PLIB_STATICBLOCK_H
#define PLIB_STATICBLOCK_H

#include "../clib/rawtypes.h"

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
}  // namespace Pol::Plib

#endif
