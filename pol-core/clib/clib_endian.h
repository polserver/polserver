/** @file
 *
 * @par History
 * - 2009/09/10 MuadDib:   FlipEndian32 Update for compilers. Submitted by Grin.
 */


#ifndef __CLIB_ENDIAN_H
#define __CLIB_ENDIAN_H

#include "clib/rawtypes.h"

#include <cstring>

#ifndef U_BIG_ENDIAN
#define U_LITTLE_ENDIAN
#endif

#define flipEndian32( x )                                                   \
  ( ( (unsigned)( x ) >> 24 ) | ( ( (unsigned)( x ) >> 8 ) & 0x0000FF00 ) | \
    ( ( ( x ) << 8 ) & 0x00FF0000 ) | ( ( x ) << 24 ) )
#define flipEndian16( x ) ( ( ( ( x ) >> 8 ) & 0x00FF ) | ( ( ( x ) << 8 ) & 0xFF00 ) )

#ifdef U_BIG_ENDIAN
/* big endian */

#define cfLEu32( x ) flipEndian32( x )
#define cfLEu16( x ) flipEndian16( x )
#define ctLEu32( x ) flipEndian32( x )
#define ctLEu16( x ) flipEndian16( x )

#define cfBEu32( x ) ( x )
#define cfBEu16( x ) ( x )
#define ctBEu32( x ) ( x )
#define ctBEu16( x ) ( x )

#else
/* little endian */

#define cfLEu32( x ) ( x )
#define cfLEu16( x ) ( x )
#define ctLEu32( x ) ( x )
#define ctLEu16( x ) ( x )

#define cfBEu32( x ) flipEndian32( x )
#define cfBEu16( x ) flipEndian16( x )
#define ctBEu32( x ) flipEndian32( x )
#define ctBEu16( x ) flipEndian16( x )

#endif

namespace Pol::Clib
{
// Packet buffers hold multi-byte fields at any offset. Dereferencing a cast pointer there is
// undefined behaviour on a misaligned address; memcpy is not, and compiles to a plain load.
template <typename T>
T load_unaligned( const void* p )
{
  T value;
  std::memcpy( &value, p, sizeof( T ) );
  return value;
}

template <typename T>
void store_unaligned( void* p, T value )
{
  std::memcpy( p, &value, sizeof( T ) );
}
}  // namespace Pol::Clib

#endif
