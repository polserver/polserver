/*
History
=======

Notes
=======
SOLIDX2_FILLER_SIZE bytes are at the beginning of the solidx2.dat file,
so that something referencing the first element won't reference offset 0.

*/

#ifndef PLIB_MAPBLOB_H
#define PLIB_MAPBLOB_H

#include "../clib/compileassert.h"

#define SOLIDX_X_SIZE 16
#define SOLIDX_X_SHIFT 4
#define SOLIDX_X_CELLMASK 0xF

#define SOLIDX_Y_SIZE 16
#define SOLIDX_Y_SHIFT 4
#define SOLIDX_Y_CELLMASK 0xF


struct SOLIDX1_ELEM
{
    unsigned int offset;
};

const unsigned SOLIDX2_FILLER_SIZE = 4;

struct SOLIDX2_ELEM
{
    unsigned int baseindex;
    unsigned short addindex[SOLIDX_X_SIZE][SOLIDX_Y_SIZE];
};
assertsize( SOLIDX2_ELEM, 516 );

struct SOLIDS_ELEM
{
    signed char z;
    unsigned char height;
    unsigned char flags;
};
assertsize( SOLIDS_ELEM, 3 );

#endif
