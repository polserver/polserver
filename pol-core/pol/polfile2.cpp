/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"

#include "clib/passert.h"

#include "plib/mapcell.h"

#include "tiles.h"

unsigned char tilelayer( unsigned short tilenum )
{
    if ( tilenum < TILE_COUNT )
    {
        return tile[ tilenum ].layer;
    }
    else
    {
        return 0;
    }
}

char tileheight(unsigned short tilenum)
{
    if ( tilenum < TILE_COUNT )
    {
        char height = tile[ tilenum ].height;
        if (tile[tilenum].flags & FLAG::GRADUAL)
            height /= 2;
        return height;
    }
    else
    {
        return 0;
    }
}

u32 tile_flags( unsigned short tilenum )
{
    if (tilenum < TILE_COUNT )
    {
        return tile[ tilenum ].flags;
    }
    else
    {
        return 0;
    }
}

u32 tile_uoflags( unsigned short tilenum )
{
    if (tilenum < TILE_COUNT )
    {
        return tile[ tilenum ].uoflags;
    }
    else
    {
        return 0;
    }
}

string tile_desc( unsigned short tilenum )
{
    if (tilenum < TILE_COUNT )
    {
        return tile[ tilenum ].desc;
    }
    else // a multi, probably
    {
        return "multi";
    }
}

unsigned short tileweight( unsigned short tilenum )
{
    if ( tilenum < TILE_COUNT )
    {
        return tile[ tilenum ].weight;
    }
    else
    {
        return 0;
    }
}
