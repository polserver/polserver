/** @file
 *
 * @par History
 * - 2009/09/10 MuadDib:   FlipEndian32 Update for compilers. Submitted by Grin.
 */


#ifndef __CLIB_ENDIAN_H
#define __CLIB_ENDIAN_H

#include "rawtypes.h"

#ifndef U_BIG_ENDIAN
#	define U_LITTLE_ENDIAN
#endif

#define flipEndian32(x) (((unsigned)(x) >> 24) | (((unsigned)(x) >> 8) & 0x0000FF00) | (((x) << 8) & 0x00FF0000) | ((x) << 24))
#define flipEndian16(x) ((((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00))

#ifdef U_BIG_ENDIAN
    /* big endian */

#	define cfLEu32(x) flipEndian32(x)
#	define cfLEu16(x) flipEndian16(x)
#	define ctLEu32(x) flipEndian32(x)
#	define ctLEu16(x) flipEndian16(x)

#	define cfBEu32(x) (x)
#	define cfBEu16(x) (x)
#	define ctBEu32(x) (x)
#	define ctBEu16(x) (x)

#else
   /* little endian */

#	define cfLEu32(x) (x)
#	define cfLEu16(x) (x)
#	define ctLEu32(x) (x)
#	define ctLEu16(x) (x)

#	define cfBEu32(x) flipEndian32(x)
#	define cfBEu16(x) flipEndian16(x)
#	define ctBEu32(x) flipEndian32(x)
#	define ctBEu16(x) flipEndian16(x)

#endif

#endif
