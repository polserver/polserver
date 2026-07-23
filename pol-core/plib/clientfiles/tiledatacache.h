#ifndef PLIB_TILEDATACACHE_H
#define PLIB_TILEDATACACHE_H

#include <array>
#include <cstdio>
#include <map>
#include <vector>

#include "../clidata.h"
#include "../ustruct.h"
#include "clib/UniqueFile.h"
#include "clib/rawtypes.h"


namespace Pol::Plib
{
// One tiledata.mul entry's cached fields.
struct TileData
{
  u8 height;
  u8 layer;
  u32 flags;
};

// Per-file verdata.mul patch index: which blocks of a client file have a newer
// version stored in verdata.
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

// The tiledata/verdata cache: owns the tiledata.mul + verdata.mul file handles,
// the in-memory tiledata table + landtile-flag table, and the verdata patch
// index, and answers every land/item-tile query from them. HSA format + the
// client's max tile id are auto-detected at open(). A collaborator of
// UoClientFiles (which delegates its tile queries here); see uoclientfiles.h for
// the load-then-query lifecycle. Queries are const and assert the cache is
// loaded rather than loading lazily.
class TileDataCache
{
public:
  TileDataCache() = default;
  ~TileDataCache() = default;
  TileDataCache( const TileDataCache& ) = delete;
  TileDataCache& operator=( const TileDataCache& ) = delete;

  // Open verdata.mul (if present) then tiledata.mul; auto-detects the HSA format
  // and sets systemstate.config.max_tile_id.
  void open();
  // Fill the caches: verdata index, then the tiledata + landtile-flag tables.
  void load();
  // Release the tiledata table early; otherwise it lives until destruction.
  void clear();

  // Auto-detected at open(): whether tiledata.mul uses the newer HSA layout.
  bool use_new_hsa_format() const { return use_new_hsa_format_; }
  // The open verdata.mul handle, for the two callers that scan it directly
  // (uotool's verdata dump, uoconvert's multi writer). Null if absent.
  FILE* verdata_file() const { return verfile_.get(); }

  void readtile( unsigned short tilenum, USTRUCT_TILE* tile ) const;
  void readtile( unsigned short tilenum, USTRUCT_TILE_HSA* tile ) const;
  void readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE* landtile ) const;
  void readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE_HSA* landtile ) const;
  void read_objinfo( u16 graphic, struct USTRUCT_TILE& objinfo ) const;
  void read_objinfo( u16 graphic, struct USTRUCT_TILE_HSA& objinfo ) const;
  unsigned int landtile_uoflags_read( unsigned short landtile ) const;
  char tileheight_read( unsigned short tilenum ) const;
  unsigned int tile_uoflags_read( unsigned short tilenum ) const;
  bool check_verdata( unsigned int file, unsigned int block, const USTRUCT_VERSION*& vrec ) const;

private:
  bool seekto_newer_version( unsigned int file, unsigned int block ) const;
  void read_veridx();
  void read_tiledata();
  void read_landtiledata();

  Clib::UniqueFile tilefile_;
  Clib::UniqueFile verfile_;
  bool use_new_hsa_format_ = false;

  std::vector<TileData> tiledata_;
  std::array<unsigned int, LANDTILE_COUNT> landtile_flags_arr_ = {};
  static constexpr unsigned int vidx_count = 32;
  std::array<VerdataIndexes, vidx_count> vidx_;
};
}  // namespace Pol::Plib

#endif
