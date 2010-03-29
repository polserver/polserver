/*
History
=======
2005/01/13 Shinigami: safe_getmapinfo - add missing checks for x and y
2005/01/23 Shinigami: rawmapinfo - used constant WORLD_X/Y... bad idea

Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif


#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../clib/passert.h"

#include "../plib/mapfunc.h"

#include "clidata.h"
#include "objtype.h"
#include "polcfg.h"
#include "polfile.h"
#include "profile.h"
#include "udatfile.h"
#include "ustruct.h"
#include "wrldsize.h"

#include "uofile.h"
#include "uofilei.h"

typedef std::map< unsigned int, unsigned int > MapBlockIndex;
MapBlockIndex mapdifl;
unsigned int num_map_patches = 0;

void read_map_difs()
{
	unsigned index = 0;
    if (mapdifl_file != NULL)
    {
        u32 blockid;
        while (fread( &blockid, sizeof blockid, 1, mapdifl_file ) == 1)
        {
            mapdifl[ blockid ] = index;
            ++index;
        }
    }
	num_map_patches = index;
}

static USTRUCT_MAPINFO_BLOCK rawmap_buffer;
static unsigned int last_block = ~0Lu;

signed char rawmapinfo( unsigned short x, unsigned short y, USTRUCT_MAPINFO* gi )
{
    passert( x < uo_map_width && y < uo_map_height );

    unsigned int x_block = x / 8;
    unsigned int y_block = y / 8;
    unsigned int block = (x_block * (uo_map_height/8) + y_block);

    if (block != last_block)
    {
        MapBlockIndex::const_iterator citr = mapdifl.find( block );
        if (citr == mapdifl.end())
        {
            unsigned int file_offset = block * sizeof(USTRUCT_MAPINFO_BLOCK);
            if (fseek( mapfile, file_offset, SEEK_SET ) != 0)
                throw runtime_error( "rawmapinfo: fseek(mapfile) failure" );
            if (fread( &rawmap_buffer, sizeof rawmap_buffer, 1, mapfile ) != 1)
                throw runtime_error( "rawmapinfo: fread(mapfile) failure" );
        }
        else
        {
            // it's in the dif file.. get it from there.
            unsigned dif_index = (*citr).second;
            unsigned int file_offset = dif_index * sizeof(USTRUCT_MAPINFO_BLOCK);
            if (fseek( mapdif_file, file_offset, SEEK_SET ) != 0)
                throw runtime_error( "rawmapinfo: fseek(mapdif_file) failure" );

            if (fread( &rawmap_buffer, sizeof rawmap_buffer, 1, mapdif_file ) != 1)
                throw runtime_error( "rawmapinfo: fread(mapdif_file) failure" );
        }

        last_block = block;
        ++mapcache_misses;
    }
    else
    {
        ++mapcache_hits;
    }

    unsigned int x_offset = x & 0x7;
    unsigned int y_offset = y & 0x7;

    *gi = rawmap_buffer.cell[y_offset][x_offset];
    return gi->z;
}

/* 

    MAP(x,y)        MAP
      ^  
      \--CHARACTER(x,y)

    MAP             MAP

    We use the diagonal with the smallest differential..

    Simple averaging doesn't seem to work near cliffs and such.
    poltest has a unit test for this.
*/
bool groundheight( unsigned short x, unsigned short y, short* z )
{
    USTRUCT_MAPINFO md, mi;
    short z1, z2, z3, z4; // quadrants
    
    z1 = rawmapinfo( x+1, y,   &md   );
    z2 = rawmapinfo( x,   y, &mi   );
    z3 = rawmapinfo( x,   y+1, &md );
    z4 = rawmapinfo( x+1, y+1, &md );

    if (abs(z1-z3)<abs(z2-z4))
        *z = (z1+z3)/2;
    else
        *z = (z2+z4)/2;
    
    if (mi.landtile == 0x2) // it's a nodraw tile
        *z = -128;
    
    return ( (mi.landtile<0x4000) && 
             ((landtile_uoflags(mi.landtile) & USTRUCT_TILE::FLAG_BLOCKING) == 0) );
}

void getmapinfo( unsigned short x, unsigned short y, short* z, USTRUCT_MAPINFO* mi )
{
    USTRUCT_MAPINFO md;
    short z1, z2, z3, z4; // quadrants
    
    z1 = rawmapinfo( x+1, y, &md   );
    z2 = rawmapinfo( x,   y, mi   );
    z3 = rawmapinfo( x,   y+1, &md );
    z4 = rawmapinfo( x+1, y+1, &md );

    short zsum;

    if (abs(z1-z3)<abs(z2-z4))
    {
        zsum = (z1+z3);
    }
    else
    {
        zsum = (z2+z4);
    }
    if (zsum >= 0)
    {
        *z = zsum / 2;
    }
    else
    {
        *z = (zsum-1) / 2;
    }
}

void safe_getmapinfo( unsigned short x, unsigned short y, short* z, USTRUCT_MAPINFO* mi )
{
    USTRUCT_MAPINFO md;
    short z1, z2, z3, z4; // quadrants

    if (x>=uo_map_width)
        x=uo_map_width-1;
    if (y>=uo_map_height)
        y=uo_map_height-1;
    unsigned short xp=x+1, yp=y+1;
    if (xp>=uo_map_width)
        xp=uo_map_width-1;
    if (yp>=uo_map_height)
        yp=uo_map_height-1;
    z1 = rawmapinfo( xp, y, &md   );
    z2 = rawmapinfo( x, y, mi   );
    z3 = rawmapinfo( x, yp, &md );
    z4 = rawmapinfo( xp, yp, &md );

    short zsum;

    if (abs(z1-z3)<abs(z2-z4))
    {
        zsum = (z1+z3);
    }
    else
    {
        zsum = (z2+z4);
    }
    if (zsum >= 0)
    {
        *z = zsum / 2;
    }
    else
    {
        *z = (zsum-1) / 2;
    }
}

