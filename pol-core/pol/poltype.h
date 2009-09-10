/*
History
=======


Notes
=======

*/

#ifndef POLTYPE_H
#define POLTYPE_H

#include "clib/rawtypes.h"

// X, Y minimums will always be zero.
const short XCOORD_MIN = 0;
const short XCOORD_MAX = 6143;

const short YCOORD_MIN = 0;
const short YCOORD_MAX = 4095;

const short ZCOORD_MIN = -128;
const short ZCOORD_MAX = +127;

// ?coord: validated to be within range.
typedef unsigned short xcoord;
typedef unsigned short ycoord;
typedef short zcoord;

typedef u32 pol_serial_t;
typedef u16 pol_objtype_t;

#endif
