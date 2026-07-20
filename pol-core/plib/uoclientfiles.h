#ifndef PLIB_UOCLIENTFILES_H
#define PLIB_UOCLIENTFILES_H

#include <cstddef>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <span>
#include <vector>

#include "RawMap.h"
#include "clidata.h"
#include "clib/rawtypes.h"
#include "uconst.h"
#include "udatfile.h"
#include "ustruct.h"

namespace Pol::Plib
{
// One tiledata.mul entry's cached fields (see uofile01.cpp).
struct TileData
{
  u8 height;
  u8 layer;
  u32 flags;
};

// Per-file verdata.mul patch index: which blocks of a client file have a newer
// version stored in verdata (see uofile01.cpp).
struct VerdataIndexes
{
  using VRecList = std::map<unsigned int, USTRUCT_VERSION>;
  VRecList vrecs;  // key is the block

  void insert( const USTRUCT_VERSION& vrec ) { vrecs.emplace( vrec.block, vrec ); }
  bool find( unsigned int block, const USTRUCT_VERSION*& vrec ) const
  {
    VRecList::const_iterator itr = vrecs.find( block );
    if ( itr == vrecs.end() )
      return false;
    vrec = &( ( *itr ).second );
    return true;
  }
};

// One statics block's cached records (see uofile02.cpp).
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
// rawstaticfullread() (or rely on the queries' lazy first-touch load -- but
// never from concurrent code; parallel regions must find the caches already
// loaded, see rawmap_loaded()/rawstatics_loaded()). Queries are const.
//
// Method definitions live in uofile00-08.cpp, grouped as before:
// open/UOP probing (00), tiledata + verdata (01), statics cache (02), water
// table (04), staticsmax (05), standheight (06), per-tile static queries (07),
// raw-map accessors (08).
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
  bool cfg_use_new_hsa_format = false;
  bool cfg_show_illegal_graphic_warning = true;
  bool cfg_show_roof_and_platform_warning = true;

  // --- Loads (uofile00.cpp / uofile01.cpp / uofile02.cpp / uofile08.cpp) ---
  void open_uo_data_files();
  void read_uo_data();          // verdata index, tiledata, landtiles, dif lists
  // The raw-map/statics caches are memoized: these fill them and the queries below
  // trigger them lazily on first touch, so both are const (they populate mutable
  // caches without changing any query's observable result). Not thread-safe -- a
  // parallel region must find the caches already loaded (see rawmap_loaded()).
  void rawmapfullread() const;     // raw map full read (mul or UOP) + difs
  void rawstaticfullread() const;  // staidx/statics full read + dif merge
  void readwater();             // water-tile table (uotool)
  void clear_tiledata();

  bool rawmap_loaded() const;
  bool rawstatics_loaded() const;

  // --- Tiledata / verdata queries (uofile01.cpp) ---
  void readtile( unsigned short tilenum, USTRUCT_TILE* tile ) const;
  void readtile( unsigned short tilenum, USTRUCT_TILE_HSA* tile ) const;
  void readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE* landtile ) const;
  void readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE_HSA* landtile ) const;
  void read_objinfo( u16 graphic, struct USTRUCT_TILE& objinfo ) const;
  void read_objinfo( u16 graphic, struct USTRUCT_TILE_HSA& objinfo ) const;
  unsigned int landtile_uoflags_read( unsigned short landtile ) const;
  char tileheight_read( unsigned short tilenum ) const;
  unsigned char tilelayer_read( unsigned short tilenum ) const;
  unsigned short tileweight_read( unsigned short tilenum ) const;
  unsigned int tile_uoflags_read( unsigned short tilenum ) const;
  bool check_verdata( unsigned int file, unsigned int block, const USTRUCT_VERSION*& vrec ) const;

  // --- Statics queries (uofile02.cpp / uofile07.cpp) ---
  const std::vector<USTRUCT_STATIC>& getstaticblock( unsigned short x, unsigned short y ) const;
  void readstatics( StaticList& vec, unsigned short x, unsigned short y ) const;
  void readstatics( StaticList& vec, unsigned short x, unsigned short y,
                    unsigned int flags ) const;
  void readstatics_block( StaticBuckets& buckets, unsigned short x, unsigned short y,
                          unsigned int flags ) const;
  void readallstatics( StaticList& vec, unsigned short x, unsigned short y ) const;
  void staticsmax() const;  // scans staidx for the largest block (diagnostic print)

  // --- Raw map queries (uofile08.cpp) ---
  void getmapinfo( unsigned short x, unsigned short y, short* z, USTRUCT_MAPINFO* mi ) const;
  void safe_getmapinfo( unsigned short x, unsigned short y, short* z, USTRUCT_MAPINFO* mi ) const;
  bool groundheight_read( unsigned short x, unsigned short y, short* z ) const;
  // Bulk-copy the raw map into caller-provided row-major arrays (idx = y*uo_map_width + x),
  // each exactly uo_map_width*uo_map_height in size.
  void rawmap_extract_planes( std::span<u16> landtile_out, std::span<s8> z_out ) const;

  // --- Walk simulation over raw client data (uofile06.cpp, uotool) ---
  void standheight_read( MOVEMODE movemode, StaticList& statics, unsigned short x,
                         unsigned short y, short oldz, bool* result, short* newz ) const;

  // --- Open file handles (public during the migration; will become private) ---
  FILE* mapfile = nullptr;
  FILE* sidxfile = nullptr;
  FILE* statfile = nullptr;
  FILE* verfile = nullptr;
  FILE* tilefile = nullptr;
  FILE* stadifl_file = nullptr;
  FILE* stadifi_file = nullptr;
  FILE* stadif_file = nullptr;
  FILE* mapdifl_file = nullptr;
  FILE* mapdif_file = nullptr;
  mutable std::ifstream uopmapfile;  // mutable: read by rawmapfullread()'s lazy load

  // --- Caches ---
  TileData* tiledata = nullptr;
  unsigned int landtile_flags_arr[LANDTILE_COUNT] = {};
  static constexpr unsigned int vidx_count = 32;
  VerdataIndexes vidx[vidx_count];

  // mutable: memoized cache filled lazily by rawstaticfullread() from const queries.
  mutable std::vector<USTRUCT_STATIC_BUFFER> rawstatic_buffer_vec;
  mutable bool rawstatic_init = false;
  std::map<unsigned int, unsigned int> stadifl;  // block -> stadif index
  unsigned int num_static_patches = 0;

  // mutable: memoized cache filled lazily by rawmapfullread() from const queries.
  mutable RawMap rawmap;
  mutable bool rawmap_ready = false;
  unsigned int num_map_patches = 0;

  std::set<unsigned int> water;

private:
  void open_map();
  void open_tiledata();
  FILE* open_map_file( const std::string& name, size_t* out_file_size = nullptr );
  bool open_uopmap_file( size_t* out_file_size = nullptr );
  void read_veridx();
  void read_tiledata();
  void read_landtiledata();
  void read_static_diffs();
  void read_map_difs();
  bool seekto_newer_version( unsigned int file, unsigned int block ) const;
  signed char rawmapinfo( unsigned short x, unsigned short y, USTRUCT_MAPINFO* gi ) const;
};

// The process-wide instance (defined in uofile00.cpp). See the TRANSITIONAL note
// on the class: the eventual end state is an instance owned by main() and passed
// explicitly through the conversion pipeline; this singleton is scaffolding for
// that migration.
UoClientFiles& uofiles();
}  // namespace Pol::Plib

#endif
