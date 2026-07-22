#ifndef PLIB_UOCLIENTFILES_H
#define PLIB_UOCLIENTFILES_H

#include <cstddef>
#include <cstdio>
#include <fstream>
#include <map>
#include <span>
#include <vector>

#include "../clidata.h"
#include "../uconst.h"
#include "../udatfile.h"
#include "../ustruct.h"
#include "RawMap.h"
#include "clib/rawtypes.h"
#include "tiledatacache.h"

namespace Pol::Plib
{
// One statics block's cached records (see staticscache.cpp).
struct USTRUCT_STATIC_BUFFER
{
  std::vector<USTRUCT_STATIC> statics;
  int count = 0;
};

// Reader over the raw UO client data files (map/statics/tiledata/verdata):
// configuration knobs, open file handles, the in-memory caches filled from
// them, and every query that answers from those caches. Used only by
// uoconvert/uotool -- pol reads converted realm files instead.
//
// Lifecycle: construct, set the configuration members (mapid, dif usage, map
// dimensions -- every block-index computation depends on uo_map_width/
// uo_map_height holding the current realm's dimensions), then
// open_uo_data_files() + read_uo_data(), then rawmapfullread()/
// rawstaticfullread() before any raw-map/statics query -- the queries assert the
// cache is loaded rather than loading it themselves, so the caller controls when
// the (non-thread-safe) load happens. Queries are const and safe to share across
// a parallel region once the loads are done. uoconvert owns an instance and
// threads it explicitly; uotool keeps a process-wide instance behind its own
// uofiles() accessor and its own load-on-first-use guards.
//
// Method definitions live alongside this header in clientfiles/, grouped by
// responsibility: open/UOP probing + the load sequence (uoclientfiles.cpp),
// tiledata + verdata (tiledatacache.cpp), statics cache + queries
// (staticscache.cpp), raw-map load (rawmapaccess.cpp). uotool-only diagnostics
// (map z-averaging, walk sim, per-tile statics, water scan) live in
// uotool/clientqueries/ as free functions over this reader.
class UoClientFiles
{
public:
  // --- Configuration: set before open_uo_data_files(), read-only afterwards ---
  int uo_mapid = 0;
  int uo_usedif = 1;
  bool uo_readuop = true;
  unsigned short uo_map_width = 0;
  unsigned short uo_map_height = 0;
  size_t uo_map_size = 0;

  int cfg_max_statics_per_block = 1000;
  int cfg_warning_statics_per_block = 1000;
  bool cfg_show_illegal_graphic_warning = true;
  bool cfg_show_roof_and_platform_warning = true;

  // --- Loads (uoclientfiles/tiledatacache/staticscache/rawmapaccess .cpp) ---
  // Call the relevant load before the matching queries -- the queries assert the
  // cache is present rather than loading lazily (see rawmap_loaded()). Not
  // thread-safe; a parallel region must find the caches already loaded.
  void open_uo_data_files();
  void read_uo_data();       // verdata index, tiledata, landtiles, dif lists
  void rawmapfullread();     // raw map full read (mul or UOP) + difs
  void rawstaticfullread();  // staidx/statics full read + dif merge
  void clear_tiledata() { tiledata_.clear(); }

  bool rawmap_loaded() const;
  bool rawstatics_loaded() const;

  // --- Tiledata / verdata queries: delegate to the tiledata cache ---
  bool use_new_hsa_format() const { return tiledata_.use_new_hsa_format(); }
  FILE* verdata_file() const { return tiledata_.verdata_file(); }
  void readtile( unsigned short tilenum, USTRUCT_TILE* tile ) const
  {
    tiledata_.readtile( tilenum, tile );
  }
  void readtile( unsigned short tilenum, USTRUCT_TILE_HSA* tile ) const
  {
    tiledata_.readtile( tilenum, tile );
  }
  void readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE* landtile ) const
  {
    tiledata_.readlandtile( tilenum, landtile );
  }
  void readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE_HSA* landtile ) const
  {
    tiledata_.readlandtile( tilenum, landtile );
  }
  void read_objinfo( u16 graphic, USTRUCT_TILE& objinfo ) const
  {
    tiledata_.read_objinfo( graphic, objinfo );
  }
  void read_objinfo( u16 graphic, USTRUCT_TILE_HSA& objinfo ) const
  {
    tiledata_.read_objinfo( graphic, objinfo );
  }
  unsigned int landtile_uoflags_read( unsigned short landtile ) const
  {
    return tiledata_.landtile_uoflags_read( landtile );
  }
  char tileheight_read( unsigned short tilenum ) const
  {
    return tiledata_.tileheight_read( tilenum );
  }
  unsigned int tile_uoflags_read( unsigned short tilenum ) const
  {
    return tiledata_.tile_uoflags_read( tilenum );
  }
  bool check_verdata( unsigned int file, unsigned int block, const USTRUCT_VERSION*& vrec ) const
  {
    return tiledata_.check_verdata( file, block, vrec );
  }

  // --- Statics cache queries (staticscache.cpp) ---
  const std::vector<USTRUCT_STATIC>& getstaticblock( unsigned short x, unsigned short y ) const;
  void readstatics_block( StaticBuckets& buckets, unsigned short x, unsigned short y,
                          unsigned int flags ) const;

  // --- Raw map (rawmapaccess.cpp) ---
  // Bulk-copy the raw map into caller-provided row-major arrays (idx = y*uo_map_width + x),
  // each exactly uo_map_width*uo_map_height in size.
  void rawmap_extract_planes( std::span<u16> landtile_out, std::span<s8> z_out ) const;

  // --- Open file handles (public during the migration; will become private) ---
  FILE* mapfile = nullptr;
  FILE* sidxfile = nullptr;
  FILE* statfile = nullptr;
  FILE* stadifl_file = nullptr;
  FILE* stadifi_file = nullptr;
  FILE* stadif_file = nullptr;
  FILE* mapdifl_file = nullptr;
  FILE* mapdif_file = nullptr;
  std::ifstream uopmapfile;

  // --- Caches ---
  TileDataCache tiledata_;

  std::vector<USTRUCT_STATIC_BUFFER> rawstatic_buffer_vec;
  bool rawstatic_init = false;
  std::map<unsigned int, unsigned int> stadifl;  // block -> stadif index
  unsigned int num_static_patches = 0;

  RawMap rawmap;
  bool rawmap_ready = false;
  unsigned int num_map_patches = 0;

private:
  void open_map();
  FILE* open_map_file( const std::string& name, size_t* out_file_size = nullptr );
  bool open_uopmap_file( size_t* out_file_size = nullptr );
  void read_static_diffs();
  void read_map_difs();
};

// Hard ceiling uoconvert clamps cfg_max_statics_per_block / *_warning_* to.
#define MAX_STATICS_PER_BLOCK 10000

// Which client data file a verdata patch applies to (the file key check_verdata
// takes). Only the multi file is queried today (uoconvert create_multis_cfg).
#define VERFILE_MULTI_MUL 0x0E

// Free helpers over the raw client files (defined in uoclientfiles.cpp): open a
// named mul/idx by install path, and load the multi collection from a UOP.
FILE* open_uo_file( const std::string& filename_part, size_t* out_file_size = nullptr );
bool open_uopmulti_file( std::map<unsigned int, std::vector<USTRUCT_MULTI_ELEMENT>>& multi_map );
}  // namespace Pol::Plib

#endif
