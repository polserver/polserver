/** @file
 *
 * @par History
 * - 2005/01/17 Shinigami: readstaticblock - modified passert reason
 * - 2005/01/23 Shinigami: readstaticblock - used constant WORLD_X/Y... bad idea
 * - 2005/07/05 Shinigami: added uoconvert.cfg option *StaticsPerBlock (hard limit is set to 10000)
 */


#include <cstdio>
#include <fmt/format.h>

#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../clib/strutil.h"
#include "staticblock.h"
#include "uofile.h"
#include "uofilei.h"
#include "ustruct.h"


namespace Pol::Plib
{
unsigned int num_static_patches = 0;
bool static_debug_on = false;

// I'd put these in an anonymous namespace, but the debugger can't see 'em...at least not easily.

unsigned int last_block = ~0u;
USTRUCT_IDX idxrec;
USTRUCT_STATIC srecs[MAX_STATICS_PER_BLOCK];
int srec_count;

int cfg_max_statics_per_block = 1000;
int cfg_warning_statics_per_block = 1000;
bool cfg_use_new_hsa_format = false;

using StaticDifBlockIndex = std::map<unsigned int, unsigned int>;
StaticDifBlockIndex stadifl;

struct USTRUCT_STATIC_BUFFER
{
  std::vector<USTRUCT_STATIC> statics;
  int count;
};
static std::vector<USTRUCT_STATIC_BUFFER> rawstatic_buffer_vec;
static bool rawstatic_init = false;

void read_static_diffs()
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

void readstaticblock( std::vector<USTRUCT_STATIC>* ppst, int* pnum, unsigned short x,
                      unsigned short y )
{
  if ( !rawstatic_init )  // FIXME just for safety cause I'm lazy
    rawstaticfullread();
  if ( x >= uo_map_width || y >= uo_map_height )
  {
    ERROR_PRINTLN( "readstaticblock: x={},y={}", x, y );
  }
  auto block = staticblock_from_coords( x, y, uo_map_height );
  *ppst = rawstatic_buffer_vec.at( block ).statics;
  *pnum = rawstatic_buffer_vec.at( block ).count;
}

void rawstaticfullread()
{
  unsigned int block = 0;
  USTRUCT_IDX idx;
  while ( fread( &idx, sizeof idx, 1, sidxfile ) == 1 )
  {
    USTRUCT_STATIC_BUFFER buf;
    StaticDifBlockIndex::const_iterator citr = stadifl.find( block );
    if ( citr == stadifl.end() )
    {
      if ( idx.length != 0xFFffFFffLu && idx.offset != 0xFFffFFffLu )
      {
        if ( fseek( statfile, idx.offset, SEEK_SET ) != 0 )
        {
          throw std::runtime_error( "readstaticblock: fseek(statfile) to " +
                                    Clib::tostring( idx.offset ) + " failed." );
        }
        srec_count = idx.length / sizeof srecs[0];
        auto [x, y] = staticblock_to_coords( block, uo_map_height );
        passert_always_r(
            srec_count <= cfg_max_statics_per_block,
            fmt::format(
                "to many static items in area {},{} - {},{} - maybe double items... you've "
                "to reduce amount of {} items below {} items ",
                x, y, x + 7, y + 7, srec_count, cfg_max_statics_per_block ) );

        // dave 9/8/3, Austin's statics had a normal offset but a length of 0. badly written tool?
        if ( idx.length != 0 && fread( srecs, idx.length, 1, statfile ) != 1 )
        {
          throw std::runtime_error( "readstaticblock: fread(statfile) failed." +
                                    Clib::tostring( block ) );
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
    else
    {
      // it's in the dif file.. get it from there.
      unsigned dif_index = ( *citr ).second;
      int offset = dif_index * sizeof idx;
      if ( fseek( stadifi_file, offset, SEEK_SET ) != 0 )
      {
        throw std::runtime_error( "readstaticblock: fseek(stadifi) to " + Clib::tostring( offset ) +
                                  " failed." );
      }

      if ( fread( &idx, sizeof idx, 1, stadifi_file ) != 1 )
      {
        throw std::runtime_error( "readstaticblock: fread(stadifi) failed." );
      }

      if ( idx.length != 0xFFffFFffLu )
      {
        if ( fseek( stadif_file, idx.offset, SEEK_SET ) != 0 )
        {
          throw std::runtime_error( "readstaticblock: fseek(stadif) to " +
                                    Clib::tostring( idx.offset ) + " failed." );
        }

        srec_count = idx.length / sizeof srecs[0];
        auto [x, y] = staticblock_to_coords( block, uo_map_height );
        passert_always_r(
            srec_count <= cfg_max_statics_per_block,
            fmt::format(
                "to many static items in area {},{} - {},{} - maybe double items... you've "
                "to reduce amount of {} items below {} items ",
                x, y, x + 7, y + 7, srec_count, cfg_max_statics_per_block ) );

        if ( fread( srecs, idx.length, 1, stadif_file ) != 1 )
        {
          throw std::runtime_error( "readstaticblock: fread(stadif) failed." );
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
    if ( buf.count > 0 )
    {
      buf.statics.assign( &srecs[0], &srecs[buf.count] );
    }
    rawstatic_buffer_vec.push_back( buf );
    ++block;
  }
  rawstatic_init = true;
}
}  // namespace Pol::Plib
