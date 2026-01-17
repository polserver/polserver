/** @file
 *
 * @par History
 */

#ifndef __CTABLE_H
#define __CTABLE_H

namespace Pol::Core
{
struct SVR_KEYDESC
{
  unsigned char nbits;
  unsigned short bits;
  unsigned short bits_reversed; /* LSB .. MSB, MSBit is in bit 0x1 */
};

// last one is a terminator
extern SVR_KEYDESC keydesc[257];
}  // namespace Pol::Core

#endif
