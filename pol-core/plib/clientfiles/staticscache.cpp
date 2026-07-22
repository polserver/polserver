/** @file
 * Statics cache: open() acquires the staidx/statics (+ stadif patch) handles,
 * read_difflist reads the staidx dif index, full_read fills a dif-merged
 * per-block cache, and getstaticblock serves per-block record vectors from it.
 * (The cross-cache readstatics_block query is a UoClientFiles coordinator method
 * -- it also needs the tiledata cache -- and lives in uoclientfiles.h.)
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
#include <string>

#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/passert.h"
#include "clib/rawtypes.h"
#include "clib/strutil.h"
#include "staticblock.h"
#include "staticscache.h"
#include "systemstate.h"
#include "uoclientfiles.h"
#include "ustruct.h"


namespace Pol::Plib
{
void StaticsCache::open( int mapid, bool usedif )
{
  staidx_.reset( open_map_file( "staidx", mapid ) );
  statics_.reset( open_map_file( "statics", mapid ) );

  if ( usedif )
  {
    std::string filename = "stadifl" + Clib::tostring( mapid ) + ".mul";
    if ( Clib::FileExists( systemstate.config.uo_datafile_root + filename ) )
    {
      stadifl_.reset( open_uo_file( filename ) );
      filename = "stadifi" + Clib::tostring( mapid ) + ".mul";
      stadifi_.reset( open_uo_file( filename ) );
      filename = "stadif" + Clib::tostring( mapid ) + ".mul";
      stadif_.reset( open_uo_file( filename ) );
    }
  }
}

void StaticsCache::read_difflist()
{
  unsigned index = 0;
  if ( stadifl_.get() != nullptr )
  {
    u32 blockid;
    while ( fread( &blockid, sizeof blockid, 1, stadifl_.get() ) == 1 )
    {
      stadifl_index_[blockid] = index;
      ++index;
    }
  }
  num_patches_ = index;
}

const std::vector<USTRUCT_STATIC>& StaticsCache::getstaticblock( unsigned short x,
                                                                 unsigned short y ) const
{
  passert_always( loaded_ );  // caller must full_read() first
  if ( x >= width_ || y >= height_ )
  {
    ERROR_PRINTLN( "getstaticblock: x={},y={}", x, y );
  }
  auto block = staticblock_from_coords( x, y, height_ );
  return buffer_vec_.at( block ).statics;
}

void StaticsCache::full_read( unsigned short width, unsigned short height, int max_per_block,
                              int warning_per_block )
{
  width_ = width;
  height_ = height;

  // Bulk-read both input files once instead of one fseek+fread pair per block
  // (~393k blocks for map0). The dif-file path below stays fseek/fread since
  // it's rare and small.
  std::vector<std::byte> idx_buf = Clib::ReadEntireFile( staidx_.get() );
  std::vector<std::byte> stat_buf = Clib::ReadEntireFile( statics_.get() );
  std::span<const USTRUCT_IDX> idx_records( reinterpret_cast<const USTRUCT_IDX*>( idx_buf.data() ),
                                            idx_buf.size() / sizeof( USTRUCT_IDX ) );

  buffer_vec_.reserve( idx_records.size() );

  for ( unsigned int block = 0; block < idx_records.size(); ++block )
  {
    USTRUCT_IDX idx = idx_records[block];
    USTRUCT_STATIC_BUFFER buf;
    auto citr = stadifl_index_.find( block );
    if ( citr == stadifl_index_.end() ) [[likely]]
    {
      if ( idx.length != 0xFFffFFffLu && idx.offset != 0xFFffFFffLu )
      {
        int srec_count = idx.length / sizeof( USTRUCT_STATIC );
        auto [x, y] = staticblock_to_coords( block, height_ );
        passert_always_r(
            srec_count <= max_per_block,
            fmt::format(
                "too many static items in area {},{} - {},{} - maybe double items... you've "
                "to reduce amount of {} items below {} items ",
                x, y, x + 7, y + 7, srec_count, max_per_block ) );

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

        if ( srec_count > warning_per_block )
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
      if ( fseek( stadifi_.get(), offset, SEEK_SET ) != 0 )
      {
        throw std::runtime_error( "rawstaticfullread: fseek(stadifi) to " +
                                  Clib::tostring( offset ) + " failed." );
      }

      if ( fread( &idx, sizeof idx, 1, stadifi_.get() ) != 1 )
      {
        throw std::runtime_error( "rawstaticfullread: fread(stadifi) failed." );
      }

      if ( idx.length != 0xFFffFFffLu )
      {
        if ( fseek( stadif_.get(), idx.offset, SEEK_SET ) != 0 )
        {
          throw std::runtime_error( "rawstaticfullread: fseek(stadif) to " +
                                    Clib::tostring( idx.offset ) + " failed." );
        }

        int srec_count = idx.length / sizeof( USTRUCT_STATIC );
        auto [x, y] = staticblock_to_coords( block, height_ );
        passert_always_r(
            srec_count <= max_per_block,
            fmt::format(
                "too many static items in area {},{} - {},{} - maybe double items... you've "
                "to reduce amount of {} items below {} items ",
                x, y, x + 7, y + 7, srec_count, max_per_block ) );

        buf.statics.resize( srec_count );
        if ( fread( buf.statics.data(), idx.length, 1, stadif_.get() ) != 1 )
        {
          throw std::runtime_error( "rawstaticfullread: fread(stadif) failed." );
        }

        if ( srec_count > warning_per_block )
          INFO_PRINTLN( " Warning: {} items found in dif-area {},{} - {},{}", srec_count, x, y,
                        ( x + 7 ), ( y + 7 ) );

        buf.count = srec_count;
      }
      else
      {
        buf.count = 0;
      }
    }
    buffer_vec_.push_back( std::move( buf ) );
  }
  loaded_ = true;
}
}  // namespace Pol::Plib
