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

typedef u32 USERIAL;
typedef u16 UOBJTYPE;
typedef u16 UCOLOR;
typedef u16 UXCOORD;
typedef u16 UYCOORD;
typedef s8 UZCOORD;


#endif
