#pragma once

#include <cstdio>
#include <iosfwd>
#include <map>
#include <vector>

#include "ustruct.h"


namespace Pol::Plib
{
class RawMap
{
private:
  bool is_init;

  std::vector<USTRUCT_MAPINFO_BLOCK> m_mapinfo_vec;

  // Patch indices for map files (mapdifl[N].mul)
  using MapBlockIndex = std::map<unsigned int, unsigned int>;
  MapBlockIndex mapdifl;

  unsigned short m_mapwidth;
  unsigned short m_mapheight;

public:
  // Gets information about a single tile within a 8x8 cell
  USTRUCT_MAPINFO get_cell( unsigned int blockidx, unsigned int x_offset, unsigned int y_offset );

  // Returns information about a given coordinate in the world
  signed char rawinfo( unsigned short x, unsigned short y, USTRUCT_MAPINFO* gi );

  // Inserts another block of 8x8 tiles
  void add_block( const USTRUCT_MAPINFO_BLOCK& block );

  // Sets map dimensions
  void set_bounds( unsigned short width, unsigned short height );

  // Read map difference indices (mapdifl[N].mul)
  // returns the number of patches
  unsigned int load_map_difflist( FILE* mapdifl_file );

  // Read full map (old version with MUL)
  // returns the number of blocks
  unsigned int load_full_map( FILE* mapfile, FILE* mapdif_file );

  unsigned int load_full_map( int uo_mapid, std::istream& uopfile );

  RawMap();
  ~RawMap() = default;
};

}  // namespace Pol::Plib
