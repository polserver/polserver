/** @file
 *
 * @par History
 */


#ifndef __UTYPE_H
#define __UTYPE_H

#ifndef CLIB_RAWTYPES_H
#include "../clib/rawtypes.h"
#endif

// I'm thinking of using these as parameters to functions, and as the
// member variable types in UObject, Item, Character, and UContainer,
// rather than explictly u32, u16 etc - more self-documenting.
// Probably not in the message types, though - those would stay
// whatever raw types.
// Can't decide between USERIAL, UXCOORD, and USerial, UXCoord, either.

using USERIAL = u32;
using UOBJTYPE = u16;
using UCOLOR = u16;
using UXCOORD = u16;
using UYCOORD = u16;
using UZCOORD = s8;


#endif
