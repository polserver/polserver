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
	bool cfg_use_new_hsa_format = 0;

    typedef std::map< unsigned int, unsigned int > StaticDifBlockIndex;
    StaticDifBlockIndex stadifl;

struct USTRUCT_STATIC_BUFFER
{
	vector<USTRUCT_STATIC> statics;
	int count;
};
static vector<USTRUCT_STATIC_BUFFER> rawstatic_buffer_vec;
static bool rawstatic_init=false;

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

void readstaticblock( std::vector<USTRUCT_STATIC>* ppst, int* pnum, unsigned short x, unsigned short y )
{
	if (!rawstatic_init) // FIXME just for safety cause I'm lazy
		rawstaticfullread();
    if (x >= uo_map_width || y >= uo_map_height)
    {
        cerr << "readstaticblock: x=" << x << ",y=" << y << endl;
    }
    unsigned int x_block, y_block;
    x_block = x / 8;
    y_block = y / 8;
    unsigned int block = (x_block * (uo_map_height/8) + y_block);
	*ppst = rawstatic_buffer_vec.at(block).statics;
	*pnum = rawstatic_buffer_vec.at(block).count;
}

void rawstaticfullread()
{
	unsigned int block=0;
	USTRUCT_IDX idx;
	while (fread( &idx, sizeof idx, 1, sidxfile ) == 1)
	{
		USTRUCT_STATIC_BUFFER buf;
		StaticDifBlockIndex::const_iterator citr = stadifl.find( block );
		if (citr == stadifl.end())
		{
			if (idx.length != 0xFFffFFffLu && idx.offset != 0xFFffFFffLu)
			{
				if (fseek( statfile, idx.offset, SEEK_SET ) != 0)
				{
					throw runtime_error( "readstaticblock: fseek(statfile) to " + tostring(idx.offset) + " failed." );
				}
				srec_count = idx.length / sizeof srecs[0];
				int x = block / uo_map_width; // FIXME is this correct?
				int y = (block % uo_map_width) / 8;
				passert_always_r( srec_count <= cfg_max_statics_per_block,
					"to many static items in area " + tostring(x) + " " + tostring(y) + " " + tostring(x + 7) +
					" " + tostring(y + 7) + " - maybe double items... you've to reduce amount of " + 
					tostring(srec_count) + " items below " + tostring(cfg_max_statics_per_block) + " items");

				//dave 9/8/3, Austin's statics had a normal offset but a length of 0. badly written tool?
				if (idx.length != 0 && fread( srecs, idx.length, 1, statfile ) != 1)
				{
					throw runtime_error( "readstaticblock: fread(statfile) failed." + tostring(block));
				}
		

				if (srec_count > cfg_warning_statics_per_block)
					cout << " Warning: " << srec_count << " items found in area "
					<< x << " " << y << " " << (x + 7) << " " << (y + 7) << endl;

				buf.count=srec_count;
			}
			else
			{
				buf.count=0;
			}
		}
		else
		{
			// it's in the dif file.. get it from there.
			unsigned dif_index = (*citr).second;
			int offset = dif_index * sizeof idx;
			if (fseek( stadifi_file, offset, SEEK_SET ) != 0)
			{
				throw runtime_error( "readstaticblock: fseek(stadifi) to " + tostring(offset) + " failed." );
			}

			if (fread( &idx, sizeof idx, 1, stadifi_file ) != 1)
			{
				throw runtime_error( "readstaticblock: fread(stadifi) failed." );
			}

			if (idx.length != 0xFFffFFffLu)
			{
				if (fseek( stadif_file, idx.offset, SEEK_SET ) != 0)
				{
					throw runtime_error( "readstaticblock: fseek(stadif) to " + tostring(idx.offset) + " failed." );
				}

				srec_count = idx.length / sizeof srecs[0];
				int x = block / uo_map_width;
				int y = (block % uo_map_width) / (uo_map_height/8);
				passert_always_r( srec_count <= cfg_max_statics_per_block,
					"to many static items in area " + tostring(x) + " " + tostring(y) + " " + tostring(x + 7) +
					" " + tostring(y + 7) + " - maybe double items... you've to reduce amount of " + 
					tostring(srec_count) + " items below " + tostring(cfg_max_statics_per_block) + " items");

				if (fread( srecs, idx.length, 1, stadif_file ) != 1)
				{
					throw runtime_error( "readstaticblock: fread(stadif) failed." );
				}

				if (srec_count > cfg_warning_statics_per_block)
					cout << " Warning: " << srec_count << " items found in dif-area "
					<< x << " " << y << " " << (x + 7) << " " << (y + 7) << endl;

				buf.count=srec_count;
			}
			else
			{
				buf.count=0;
			}
		}
		if (buf.count>0)
		{
			buf.statics.assign(&srecs[0],&srecs[buf.count]);
		}
		rawstatic_buffer_vec.push_back(buf);
		++block;
	}
	rawstatic_init=true;
}
