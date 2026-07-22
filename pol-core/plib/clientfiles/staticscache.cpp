/** @file
 * Statics cache: rawstaticfullread fills a dif-merged staidx/statics cache,
 * getstaticblock serves per-block record vectors from it, and the per-tile
 * queries (readstatics / readstatics_block / readallstatics) + the staidx
 * staticsmax diagnostic answer from that cache.
 *
 * @par History
 * - 2005/01/17 Shinigami: readstaticblock - modified passert reason
 * - 2005/01/23 Shinigami: readstaticblock - used constant WORLD_X/Y... bad idea
 * - 2005/07/05 Shinigami: added uoconvert.cfg option *StaticsPerBlock (hard limit is set to 10000)
 */


#include <cstdio>
#include <cstring>
#include <fmt/format.h>
#include <span>

#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/passert.h"
#include "clib/rawtypes.h"
#include "clib/strutil.h"
#include "staticblock.h"
#include "uoclientfiles.h"
#include "ustruct.h"


namespace Pol::Plib
{
void UoClientFiles::read_static_diffs()
{
  unsigned index = 0;
  if ( stadifl_file != nullptr )
  {
    u32 blockid;
    while ( fread( &blockid, sizeof blockid, 1, stadifl_file ) == 1 )
    {
      stadifl[blockid] = index;
      ++index;
    }
  }
  num_static_patches = index;
}

const std::vector<USTRUCT_STATIC>& UoClientFiles::getstaticblock( unsigned short x,
                                                                  unsigned short y ) const
{
  passert_always( rawstatic_init );  // caller must rawstaticfullread() first
  if ( x >= uo_map_width || y >= uo_map_height )
  {
    ERROR_PRINTLN( "getstaticblock: x={},y={}", x, y );
  }
  auto block = staticblock_from_coords( x, y, uo_map_height );
  return rawstatic_buffer_vec.at( block ).statics;
}

bool UoClientFiles::rawstatics_loaded() const
{
  return rawstatic_init;
}

void UoClientFiles::rawstaticfullread()
{
  // Bulk-read both input files once instead of one fseek+fread pair per block
  // (~393k blocks for map0). The dif-file path below stays fseek/fread since
  // it's rare and small.
  std::vector<std::byte> idx_buf = Clib::ReadEntireFile( sidxfile );
  std::vector<std::byte> stat_buf = Clib::ReadEntireFile( statfile );
  std::span<const USTRUCT_IDX> idx_records( reinterpret_cast<const USTRUCT_IDX*>( idx_buf.data() ),
                                            idx_buf.size() / sizeof( USTRUCT_IDX ) );

  rawstatic_buffer_vec.reserve( idx_records.size() );

  for ( unsigned int block = 0; block < idx_records.size(); ++block )
  {
    USTRUCT_IDX idx = idx_records[block];
    USTRUCT_STATIC_BUFFER buf;
    auto citr = stadifl.find( block );
    if ( citr == stadifl.end() ) [[likely]]
    {
      if ( idx.length != 0xFFffFFffLu && idx.offset != 0xFFffFFffLu )
      {
        int srec_count = idx.length / sizeof( USTRUCT_STATIC );
        auto [x, y] = staticblock_to_coords( block, uo_map_height );
        passert_always_r(
            srec_count <= cfg_max_statics_per_block,
            fmt::format(
                "too many static items in area {},{} - {},{} - maybe double items... you've "
                "to reduce amount of {} items below {} items ",
                x, y, x + 7, y + 7, srec_count, cfg_max_statics_per_block ) );

        // dave 9/8/3, Austin's statics had a normal offset but a length of 0. badly written tool?
        if ( idx.length != 0 )
        {
          if ( static_cast<u64>( idx.offset ) + idx.length > stat_buf.size() )
          {
            throw std::runtime_error(
                "rawstaticfullread: statics0.mul read out of range for block " +
                Clib::tostring( block ) );
          }
          buf.statics.resize( srec_count );
          memcpy( buf.statics.data(), stat_buf.data() + idx.offset, idx.length );
        }

        if ( srec_count > cfg_warning_statics_per_block )
          INFO_PRINTLN( " Warning: {} items found in area {},{} - {},{}", srec_count, x, y,
                        ( x + 7 ), ( y + 7 ) );

        buf.count = srec_count;
      }
      else
      {
        buf.count = 0;
      }
    }
    else [[unlikely]]
    {
      // it's in the dif file.. get it from there.
      unsigned dif_index = ( *citr ).second;
      int offset = dif_index * sizeof idx;
      if ( fseek( stadifi_file, offset, SEEK_SET ) != 0 )
      {
        throw std::runtime_error( "rawstaticfullread: fseek(stadifi) to " +
                                  Clib::tostring( offset ) + " failed." );
      }

      if ( fread( &idx, sizeof idx, 1, stadifi_file ) != 1 )
      {
        throw std::runtime_error( "rawstaticfullread: fread(stadifi) failed." );
      }

      if ( idx.length != 0xFFffFFffLu )
      {
        if ( fseek( stadif_file, idx.offset, SEEK_SET ) != 0 )
        {
          throw std::runtime_error( "rawstaticfullread: fseek(stadif) to " +
                                    Clib::tostring( idx.offset ) + " failed." );
        }

        int srec_count = idx.length / sizeof( USTRUCT_STATIC );
        auto [x, y] = staticblock_to_coords( block, uo_map_height );
        passert_always_r(
            srec_count <= cfg_max_statics_per_block,
            fmt::format(
                "too many static items in area {},{} - {},{} - maybe double items... you've "
                "to reduce amount of {} items below {} items ",
                x, y, x + 7, y + 7, srec_count, cfg_max_statics_per_block ) );

        buf.statics.resize( srec_count );
        if ( fread( buf.statics.data(), idx.length, 1, stadif_file ) != 1 )
        {
          throw std::runtime_error( "rawstaticfullread: fread(stadif) failed." );
        }

        if ( srec_count > cfg_warning_statics_per_block )
          INFO_PRINTLN( " Warning: {} items found in dif-area {},{} - {},{}", srec_count, x, y,
                        ( x + 7 ), ( y + 7 ) );

        buf.count = srec_count;
      }
      else
      {
        buf.count = 0;
      }
    }
    rawstatic_buffer_vec.push_back( std::move( buf ) );
  }
  rawstatic_init = true;
}

// --- Statics-block query used by uoconvert (formerly uofile07.cpp) ---
void UoClientFiles::readstatics_block( StaticBuckets& buckets, unsigned short x, unsigned short y,
                                       unsigned int flags ) const
{
  for ( auto& bucket : buckets )
    bucket.clear();

  const std::vector<USTRUCT_STATIC>& srecarr = getstaticblock( x, y );

  for ( const auto& srec : srecarr )
  {
    // Records with offsets outside the 8x8 block never match any tile in the
    // per-tile readstatics() scan; drop them here the same way.
    if ( srec.x_offset < 0 || srec.x_offset >= static_cast<int>( STATICBLOCK_CHUNK ) ||
         srec.y_offset < 0 || srec.y_offset >= static_cast<int>( STATICBLOCK_CHUNK ) )
      continue;

    const unsigned int uoflags = tile_uoflags_read( srec.graphic );
    if ( uoflags & flags )
      buckets[srec.x_offset * STATICBLOCK_CHUNK + srec.y_offset].emplace_back(
          srec.graphic, srec.z, uoflags, tileheight_read( srec.graphic ) );
  }
}
}  // namespace Pol::Plib
