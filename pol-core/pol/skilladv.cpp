/** @file
 *
 * @par History
 */


#include "skilladv.h"


namespace Pol::Core
{
/* skill advancement:
    (essentially, a base-2 logarithm..)
    disp     raw              bits  posn
    10.0    2048    1000 0000 0000    11
    20.0    4096  1 0000 0000 0000    12
    30.0      8K                      13
    ...
    70.0    128K                      17
    ...
    100.0  1024K                      20
    200.0     1M                     ...
    220.0     4M                     ...
    We care about the high bit, and the next 8 bits of
    lesser significance to it.
    So, we want to normalize to: 1 bbbb bbbb (n removed)
    "bbbb bbbb" is used to generate the "ones.decimal" part of
    a "tens ones . decimal" skill value.

    2047 (  111 1111 1111 )
    shift 2 bits:
    1 1111 1111
    Not greater than 1FFh, so done
    skill += (255 * 100) / 256
    2048: (1000 0000 0000 )
    shift 2 bits:
    10 0000 0000
    greater than 1FF, so skill += 100, raw >>= 1
    1 0000 0000
    not greater than 1FF, so stop

    Note, this function is faster for lower skill values than high ones.
    */
unsigned short raw_to_base( unsigned int raw )
{
  unsigned short skill = 0;

  // what we're really doing here is skipping up the power of two chain,
  // accumulating tens.
  while ( raw & 0xFFFFF000Lu )
  {
    raw >>= 1;
    skill += 100;  // go from 0.0 to 10.0, 10.0 to 20.0, and so forth
  }

  if ( raw & 0x800 )
  {
    raw &= 0x7FF;
    skill += 100;
  }

  // now, what's left is the "1 bbbb bbbb" part.
  //             (or "0 bbbb bbbb" if less than 1 0000 0000)
  // calculate (linearly) the ones digit and decimal point part, using 8 bits

  skill += static_cast<unsigned short>( raw ) * 100 / 2048;

  return skill;
}

unsigned int base_to_raw( unsigned short base )
{
  if ( base < 100 )
  {
    return base * 2048L / 100L;
  }
  if ( base > 2100 )
  {
    base = 2100;
  }

  unsigned int raw = 1024;
  unsigned short tmpbase = base;
  int rawadd = 10;
  while ( tmpbase >= 100 )
  {
    tmpbase -= 100;
    raw *= 2;
    rawadd *= 2;
  }
  raw += ( tmpbase * raw / 100L );

  int diff;
  while ( ( diff = base - raw_to_base( raw ) ) > 0 )
  {
    if ( diff > 1 )
      --diff;
    raw += rawadd * diff;
  }

  return raw;
}
}  // namespace Pol::Core
