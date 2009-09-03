/*
History
=======


Notes
=======

*/

#ifndef TILES_H
#define TILES_H

#include "clib/rawtypes.h"

class Tile
{
public:
    string desc;
    unsigned long uoflags; // USTRUCT_TILE::*
    unsigned long flags; // FLAG::*
    u8 layer;
    u8 height;
    u8 weight; // todo mult, div
};

const unsigned TILE_COUNT = 0x4000;
extern Tile tile[ TILE_COUNT ];

#endif
