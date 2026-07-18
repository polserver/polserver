/** @file
 * Reader API over the raw UO client data files (map/statics/tiledata/verdata),
 * used by uoconvert and uotool -- pol reads converted realm files instead.
 *
 * Implementation is spread over uofile00-08.cpp; by declaration group:
 * - open_uo_data_files/read_uo_data ......... uofile00.cpp (open, UOP probing)
 * - readtile/readlandtile/read_objinfo ...... uofile01.cpp (tiledata + verdata cache)
 * - getstaticblock/rawstaticfullread ........ uofile02.cpp (statics full-read cache)
 * - readwater/iswater ....................... uofile04.cpp (water-tile table)
 * - staticsmax .............................. uofile05.cpp
 * - readstatics* (udatfile.h) ............... uofile07.cpp (per-tile static queries)
 * - getmapinfo/safe_getmapinfo/rawmap* ...... uofile08.cpp (raw map cache accessors)
 * (uofile06.cpp: standheight_read, declared in uofilei.h)
 *
 * @par History
 * - 2005/07/05 Shinigami: added uoconvert.cfg option *StaticsPerBlock (hard limit is set to 10000)
 * - 2005/07/16 Shinigami: added uoconvert.cfg flag ShowIllegalGraphicWarning
 * - 2006/04/09 Shinigami: added uoconvert.cfg flag ShowRoofAndPlatformWarning
 */


#ifndef UOFILE_H
#define UOFILE_H

#include "clidata.h"
#include "ustruct.h"

#include <vector>


#define MAX_STATICS_PER_BLOCK 10000

namespace Pol::Plib
{
extern void safe_getmapinfo( unsigned short x, unsigned short y, short* z, USTRUCT_MAPINFO* mi );
void rawmapfullread();
// Bulk-copy the raw map into caller-provided row-major arrays (idx = y*uo_map_width + x),
// each uo_map_width*uo_map_height in size. Triggers a lazy rawmapfullread() if needed.
void rawmap_extract_planes( u16* landtile_out, s8* z_out );
void getmapinfo( unsigned short x, unsigned short y, short* z, USTRUCT_MAPINFO* mi );
void readtile( unsigned short tilenum, USTRUCT_TILE* tile );
void readtile( unsigned short tilenum, USTRUCT_TILE_HSA* tile );
void clear_tiledata();
const std::vector<USTRUCT_STATIC>& getstaticblock( unsigned short x, unsigned short y );
void rawstaticfullread();

// True once rawmapfullread()/rawstaticfullread() have populated the raw buffers. Used to
// assert the buffers are already loaded before entering a parallel region, so getstaticblock/
// safe_getmapinfo's lazy first-touch read can never fire concurrently (which would race).
bool rawmap_loaded();
bool rawstatics_loaded();


void read_objinfo( u16 graphic, struct USTRUCT_TILE& objinfo );
void read_objinfo( u16 graphic, struct USTRUCT_TILE_HSA& objinfo );
void readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE* landtile );
void readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE_HSA* landtile );

void open_uo_data_files();
void read_uo_data();
void readwater();

void staticsmax();
bool iswater( u16 objtype );

// Mutable global UO-file state, set once per run before any reader/conversion
// function is called and then treated as read-only. uoconvert's main() fills
// these from the command-line/realm descriptor for the chosen command (map
// dimensions, mapid, dif usage) *before* dispatching to create_map/create_maptile/
// statics/etc.; pol sets them when loading a realm. Every getmapinfo/readstatics/
// block-index call silently depends on uo_map_width/uo_map_height already holding
// the current realm's dimensions -- change them mid-run and the block math is wrong.
extern int uo_mapid;
extern int uo_usedif;
extern bool uo_readuop;
extern unsigned short uo_map_width;
extern unsigned short uo_map_height;
extern size_t uo_map_size;

extern int cfg_max_statics_per_block;
extern int cfg_warning_statics_per_block;
extern bool cfg_show_illegal_graphic_warning;
extern bool cfg_show_roof_and_platform_warning;
extern bool cfg_use_new_hsa_format;
}  // namespace Pol::Plib

#endif
