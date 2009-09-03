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
const long XCOORD_MIN = 0;
const long XCOORD_MAX = 6143;

const long YCOORD_MIN = 0;
const long YCOORD_MAX = 4095;

const long ZCOORD_MIN = -128;
const long ZCOORD_MAX = +127;

// ?coord: validated to be within range.
typedef long xcoord;
typedef long ycoord;
typedef long zcoord;

typedef u32 pol_serial_t;
typedef u16 pol_objtype_t;

#endif
