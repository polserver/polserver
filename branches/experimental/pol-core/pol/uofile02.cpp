/*
History
=======
2005/01/17 Shinigami: readstaticblock - modified passert reason
2005/01/23 Shinigami: readstaticblock - used constant WORLD_X/Y... bad idea
2005/07/05 Shinigami: added uoconvert.cfg option *StaticsPerBlock (hard limit is set to 10000)

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <stdio.h>
#include <string.h>

#include "../clib/passert.h"
#include "../clib/stlutil.h"

#include "polcfg.h"
#include "polfile.h"
#include "udatfile.h"
#include "ustruct.h"

#include "uofile.h"
#include "uofilei.h"
#include "wrldsize.h"

bool static_debug_on = false;
unsigned int num_static_patches = 0;
// I'd put these in an anonymous namespace, but the debugger can't see 'em...at least not easily.

    unsigned int last_block = ~0u;
    USTRUCT_IDX idxrec;
    USTRUCT_STATIC srecs[MAX_STATICS_PER_BLOCK];
    int srec_count;

    int cfg_max_statics_per_block = 1000;
    int cfg_warning_statics_per_block = 1000;

    typedef std::map< unsigned int, unsigned int > StaticDifBlockIndex;
    StaticDifBlockIndex stadifl;

void read_static_diffs()
{
    unsigned index = 0;
    if (stadifl_file != NULL)
    {
        u32 blockid;
        while (fread( &blockid, sizeof blockid, 1, stadifl_file ) == 1)
        {
            stadifl[ blockid ] = index;
            ++index;
        }
    }
	num_static_patches = index;
}

void readstaticblock( USTRUCT_STATIC** ppst, int* pnum, unsigned short x, unsigned short y )
{
    if (x >= uo_map_width || y >= uo_map_height)
    {
        cerr << "readstaticblock: x=" << x << ",y=" << y << endl;
    }
    unsigned int x_block, y_block;

    x_block = x / 8;
    y_block = y / 8;
    unsigned int block = (x_block * (uo_map_height/8) + y_block);
    if (block != last_block)
    {
        last_block = block;

        StaticDifBlockIndex::const_iterator citr = stadifl.find( block );
        if (citr == stadifl.end())
        {
            int offset = block * sizeof idxrec;
            if (fseek( sidxfile, offset, SEEK_SET ) != 0)
            {
                throw runtime_error( "readstaticblock: fseek(sidxfile) to " + tostring(offset) + " failed." );
            }

            if (fread( &idxrec, sizeof idxrec, 1, sidxfile ) != 1)
            {
                throw runtime_error( "readstaticblock: fread(sidxfile) failed." );
            }

            if (idxrec.length != 0xFFffFFffLu && idxrec.offset != 0xFFffFFffLu)
            {
                if (fseek( statfile, idxrec.offset, SEEK_SET ) != 0)
                {
                    throw runtime_error( "readstaticblock: fseek(statfile) to " + tostring(idxrec.offset) + " failed." );
                }
                //dave 9/8/3, Austin's statics had a normal offset but a length of 0. badly written tool?
                if (idxrec.length != 0 && fread( srecs, idxrec.length, 1, statfile ) != 1)
                {
                    throw runtime_error( "readstaticblock: fread(statfile) failed." );
                }
                srec_count = idxrec.length / sizeof srecs[0];

                passert_always_r( srec_count <= cfg_max_statics_per_block,
                  "to many static items in area " + tostring(x) + " " + tostring(y) + " " + tostring(x + 7) +
                  " " + tostring(y + 7) + " - maybe double items... you've to reduce amount of " + 
                  tostring(srec_count) + " items below " + tostring(cfg_max_statics_per_block) + " items");

                if (srec_count > cfg_warning_statics_per_block)
                    cout << " Warning: " << srec_count << " items found in area "
                         << x << " " << y << " " << (x + 7) << " " << (y + 7) << endl;
            }
            else
            {
                srec_count = 0;
            }
        }
        else
        {
            // it's in the dif file.. get it from there.
            unsigned dif_index = (*citr).second;
            int offset = dif_index * sizeof idxrec;
            if (fseek( stadifi_file, offset, SEEK_SET ) != 0)
            {
                throw runtime_error( "readstaticblock: fseek(stadifi) to " + tostring(offset) + " failed." );
            }

            if (fread( &idxrec, sizeof idxrec, 1, stadifi_file ) != 1)
            {
                throw runtime_error( "readstaticblock: fread(stadifi) failed." );
            }

            if (idxrec.length != 0xFFffFFffLu)
            {
                if (fseek( stadif_file, idxrec.offset, SEEK_SET ) != 0)
                {
                    throw runtime_error( "readstaticblock: fseek(stadif) to " + tostring(idxrec.offset) + " failed." );
                }
                if (fread( srecs, idxrec.length, 1, stadif_file ) != 1)
                {
                    throw runtime_error( "readstaticblock: fread(stadif) failed." );
                }
                srec_count = idxrec.length / sizeof srecs[0];

                passert_always_r( srec_count <= cfg_max_statics_per_block,
                  "to many static items in dif-area " + tostring(x) + " " + tostring(y) + " " + tostring(x + 7) +
                  " " + tostring(y + 7) + " - maybe double items... you've to reduce amount of " + 
                  tostring(srec_count) + " items below " + tostring(cfg_max_statics_per_block) + " items");

                if (srec_count > cfg_warning_statics_per_block)
                    cout << " Warning: " << srec_count << " items found in dif-area "
                         << x << " " << y << " " << (x + 7) << " " << (y + 7) << endl;
            }
            else
            {
                srec_count = 0;
            }
        }
    }

    *ppst = srecs;
    *pnum = srec_count;
}

