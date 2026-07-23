#pragma once

#include <cstddef>
#include <cstdio>
#include <fstream>
#include <map>
#include <span>
#include <vector>

#include "../ustruct.h"
#include "clib/UniqueFile.h"


namespace Pol::Plib
{
// The raw map cache: owns the map<mapid>.mul / map<mapid>LegacyMUL.uop handle plus
// the optional mapdif patch files, the in-memory block cache filled from them, and
// the mapdifl patch index. A collaborator of UoClientFiles, which delegates the
// raw-map queries (rawinfo / extract_planes) to it. Like the other client-file
// caches, open() acquires the handles, read_difflist()/full_read() fill the cache,
// and the queries assert the cache is loaded rather than loading lazily; see
// uoclientfiles.h for the lifecycle.
class RawMap
{
public:
  RawMap();
  ~RawMap() = default;
  RawMap( const RawMap& ) = delete;
  RawMap& operator=( const RawMap& ) = delete;

  // Open map<mapid>.mul, or map<mapid>LegacyMUL.uop when readuop (falling back to
  // the mul when the UOP is absent), plus the mapdif<mapid> patch files when usedif.
  // Returns the (equivalent) mul map size in bytes.
  size_t open( int mapid, bool usedif, bool readuop );
  // Read the mapdifl index (block -> mapdif record); returns / sets num_patches().
  unsigned int read_difflist();
  // Full raw-map read (mul or UOP) into the block cache + dif merge; captures bounds.
  void full_read( int mapid, unsigned short width, unsigned short height );

  bool loaded() const { return is_init; }
  unsigned int num_patches() const { return m_num_patches; }

  // Returns information about a given coordinate in the world
  signed char rawinfo( unsigned short x, unsigned short y, USTRUCT_MAPINFO* gi ) const;

  // Bulk-copy the whole map into caller-provided row-major arrays
  // (idx = y * width + x), each exactly width*height in size. Same cell selection
  // as rawinfo(), but without the per-call bounds-check / struct-copy overhead --
  // for consumers (uoconvert's terrain plane) that need every tile at once.
  void extract_planes( std::span<u16> landtile_out, std::span<s8> z_out ) const;

private:
  // Gets information about a single tile within a 8x8 cell
  USTRUCT_MAPINFO get_cell( unsigned int blockidx, unsigned int x_offset,
                            unsigned int y_offset ) const;

  // Inserts another block of 8x8 tiles
  void add_block( const USTRUCT_MAPINFO_BLOCK& block );

  // Sets map dimensions
  void set_bounds( unsigned short width, unsigned short height );

  // Open map<mapid>LegacyMUL.uop into uopmapfile_ (false when absent).
  bool open_uop( int mapid, size_t* out_file_size );

  // Read map difference indices (mapdifl[N].mul); returns the number of patches.
  unsigned int load_map_difflist( FILE* mapdifl_file );

  // Read full map (old version with MUL); returns the number of blocks.
  unsigned int load_full_map( FILE* mapfile, FILE* mapdif_file );

  unsigned int load_full_map( int uo_mapid, std::istream& uopfile );

  bool is_init;

  std::vector<USTRUCT_MAPINFO_BLOCK> m_mapinfo_vec;

  // Patch indices for map files (mapdifl[N].mul)
  using MapBlockIndex = std::map<unsigned int, unsigned int>;
  MapBlockIndex mapdifl;

  unsigned short m_mapwidth;
  unsigned short m_mapheight;

  // Owned file handles: the map file and its optional dif patch files.
  Clib::UniqueFile mapfile_;
  Clib::UniqueFile mapdifl_file_;
  Clib::UniqueFile mapdif_file_;
  std::ifstream uopmapfile_;
  unsigned int m_num_patches = 0;
};

}  // namespace Pol::Plib
