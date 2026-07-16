#include "UoConvertMain.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#include "clib/Program/ProgramMain.h"
#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/passert.h"
#include "clib/rawtypes.h"
#include "clib/stlutil.h"
#include "clib/timer.h"
#include "plib/clidata.h"
#include "plib/mapcell.h"
#include "plib/mapfunc.h"
#include "plib/mapshape.h"
#include "plib/mapsolid.h"
#include "plib/maptile.h"
#include "plib/mapwriter.h"
#include "plib/mul/map.h"
#include "plib/objtype.h"
#include "plib/polfile.h"
#include "plib/realmdescriptor.h"
#include "plib/systemstate.h"
#include "plib/udatfile.h"
#include "plib/uofile.h"
#include "plib/uofilei.h"
#include "plib/uoinstallfinder.h"
#include "plib/uopreader/uop.h"
#include "plib/uopreader/uophash.h"
#include "plib/ustruct.h"
#include "pol/landtile.h"

#include "terrainplane.h"


namespace Pol::UoConvert
{
using namespace std;
using namespace Pol::Core;
using namespace Pol::Plib;

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

UoConvertMain::UoConvertMain()
    : Pol::Clib::ProgramMain(), cfg_use_no_shoot( false ), cfg_LOS_through_windows( false )
{
}
UoConvertMain::~UoConvertMain() = default;
///////////////////////////////////////////////////////////////////////////////

void UoConvertMain::showHelp()
{
  ERROR_PRINTLN( std::string{
      "Usage:\n"
      "    \n"
      "  UOCONVERT command [options ...]\n"
      "    \n"
      "  Commands: \n"
      "    map {uodata=Dir} {realm=realmname} {width=Width}        {height=Height} {mapid=0} "
      "{readuop=1} {x=X} {y=Y} {profile=0}\n"
      "    statics {uodata=Dir} {realm=realmname}\n"
      "    maptile {uodata=Dir} {realm=realmname}\n"
      "    multis {uodata=Dir} {outdir=dir}\n"
      "    tiles {uodata=Dir} {outdir=dir}\n"
      "    landtiles {uodata=Dir} {outdir=dir}" } );
}

using namespace Core;
using namespace Plib;

void UoConvertMain::display_flags()
{
  for ( unsigned blocking = 0; blocking <= 1; ++blocking )
  {
    for ( unsigned platform = 0; platform <= 1; ++platform )
    {
      for ( unsigned walk = 0; walk <= 1; ++walk )
      {
        for ( unsigned wall = 0; wall <= 1; ++wall )
        {
          for ( unsigned half = 0; half <= 1; ++half )
          {
            for ( unsigned floor = 0; floor <= 1; ++floor )
            {
              unsigned flags = 0;
              if ( blocking )
                flags |= USTRUCT_TILE::FLAG_BLOCKING;
              if ( platform )
                flags |= USTRUCT_TILE::FLAG_PLATFORM;
              if ( walk )
                flags |= USTRUCT_TILE::FLAG__WALK;
              if ( wall )
                flags |= USTRUCT_TILE::FLAG_WALL;
              if ( half )
                flags |= USTRUCT_TILE::FLAG_HALF_HEIGHT;
              if ( floor )
                flags |= USTRUCT_TILE::FLAG_FLOOR;

              unsigned int polflags = Plib::polflags_from_tileflags(
                  0x4000, flags, cfg_use_no_shoot, cfg_LOS_through_windows );
              unsigned moveland = ( polflags & Plib::FLAG::MOVELAND ) ? 1 : 0;
              INFO_PRINTLN( "{} {} {} {} {} {}: {}", blocking, platform, walk, wall, half, floor,
                            moveland );
            }
          }
        }
      }
    }
  }
}

void UoConvertMain::create_maptile( const std::string& realmname )
{
  Plib::RealmDescriptor descriptor = Plib::RealmDescriptor::Load( realmname );
  uo_map_height = static_cast<unsigned short>( descriptor.height );
  uo_map_width = static_cast<unsigned short>( descriptor.width );

  INFO_PRINTLN(
      "Creating maptile file.\n"
      "  Realm: {}\n"
      "  Map ID: {}\n"
      "  Use Dif files: {}\n"
      "  Size: {}x{}",
      realmname, descriptor.uomapid, ( descriptor.uodif ? "Yes" : "No" ), uo_map_width,
      uo_map_height );

  Plib::MapWriter writer;
  writer.OpenExistingFiles( realmname );

  // Same precomputed terrain as create_map; maptile only needs the landtile id and the
  // effective (liquid-overridden) z per tile, so skip the lowest-adjacent-z pass.
  rawmapfullread();
  TerrainPlane plane;
  plane.build( uo_map_width, uo_map_height, /*need_low_z=*/false );

  for ( unsigned short y_base = 0; y_base < uo_map_height; y_base += Plib::MAPTILE_CHUNK )
  {
    for ( unsigned short x_base = 0; x_base < uo_map_width; x_base += Plib::MAPTILE_CHUNK )
    {
      unsigned short x_add_max = Plib::MAPTILE_CHUNK, y_add_max = Plib::MAPTILE_CHUNK;
      if ( x_base + x_add_max > uo_map_width )
        x_add_max = uo_map_width - x_base;
      if ( y_base + y_add_max > uo_map_height )
        y_add_max = uo_map_height - y_base;

      for ( unsigned short x_add = 0; x_add < x_add_max; ++x_add )
      {
        for ( unsigned short y_add = 0; y_add < y_add_max; ++y_add )
        {
          unsigned short x = x_base + x_add;
          unsigned short y = y_base + y_add;

          const std::size_t plane_idx = plane.index( x, y );
          const u16 landtile = plane.landtile[plane_idx];
          const s8 z = plane.eff_z[plane_idx];  // effective z (liquid override applied)

          if ( landtile > 0x3FFF )
            INFO_PRINTLN( "Tile {:#x} at ({},{},{}) is an invalid ID!", landtile, x, y, z );

          Plib::MAPTILE_CELL cell;
          cell.landtile = landtile;
          cell.z = z;
          writer.SetMapTile( x, y, cell );
        }
      }
    }
    INFO_PRINT( "\rConverting: {}%", y_base * 100 / uo_map_height );
  }
  writer.Flush();
  INFO_PRINTLN( "\rConversion complete." );
}

class StaticsByZ
{
public:
  bool operator()( const StaticRec& a, const StaticRec& b )
  {
    return ( a.z < b.z ) || ( ( a.z == b.z && a.height < b.height ) );
  }
};


bool flags_match( unsigned int f1, unsigned int f2, unsigned char bits_compare )
{
  return ( f1 & bits_compare ) == ( f2 & bits_compare );
}

/*
bool otherflags_match( unsigned char f1, unsigned char f2, unsigned char bits_exclude )
{
  return ( f1 & ~bits_exclude ) == ( f2 & ~bits_exclude );
}
bool differby_exactly( unsigned char f1, unsigned char f2, unsigned char bits )
{
  return ( ( f1 ^ f2 ) == bits );
}*/

void UoConvertMain::update_map( const std::string& realm, unsigned short x, unsigned short y )
{
  MapWriter mapwriter;
  mapwriter.OpenExistingFiles( realm );
  rawmapfullread();
  rawstaticfullread();
  unsigned short x_base = x / SOLIDX_X_SIZE * SOLIDX_X_SIZE;
  unsigned short y_base = y / SOLIDX_Y_SIZE * SOLIDX_Y_SIZE;

  // ProcessSolidBlock reads the smoothed-terrain plane; build the full plane. This is a
  // rarely-used single-block debug path (x=/y= args), so the full build is acceptable.
  TerrainPlane plane;
  plane.build( uo_map_width, uo_map_height );

  BlockResult result = ComputeSolidBlock( x_base, y_base, plane );
  StitchBlock( mapwriter, x_base, y_base, result );
  INFO_PRINTLN( "empty={}, nonempty={}\nwith more_solids: {}\ntotal statics={}", empty, nonempty,
                with_more_solids, total_statics );
}

void UoConvertMain::create_map( const std::string& realm, unsigned short width,
                                unsigned short height )
{
  MapWriter mapwriter;
  INFO_PRINT(
      "Creating map base and solids files.\n"
      "  Realm: {}\n"
      "  Map ID: {}\n"
      "  Reading UOP file: {}\n"
      "  Use Dif files: {}\n"
      "  Size: {}x{}\n"
      "Initializing files: ",
      realm, uo_mapid, ( uo_readuop ? "Yes" : "No" ), ( uo_usedif ? "Yes" : "No" ), uo_map_width,
      uo_map_height );
  // Reset the opt-in per-tile profiling accumulators for this run.
  prof_mapinfo_ns = prof_statics_ns = prof_shape_ns = prof_writer_ns = 0;

  // Coarse, always-on phase timers so a plain run shows where the wall-clock goes
  // (file init vs map read vs static read vs the hot loop vs the final flush).
  Tools::Timer<> init_timer;
  mapwriter.CreateNewFiles( realm, width, height );
  init_timer.stop();
  INFO_PRINTLN( "Done." );

  Tools::Timer<> mapread_timer;
  rawmapfullread();
  mapread_timer.stop();

  Tools::Timer<> staticread_timer;
  rawstaticfullread();
  staticread_timer.stop();

  // Precompute the smoothed-terrain plane once (safe_getmapinfo + lowest-adjacent-z for
  // every tile) so the block loop below is flat array reads instead of ~36 raw cell
  // fetches per tile. The plane is immutable during the loop.
  Tools::Timer<> plane_timer;
  TerrainPlane plane;
  plane.build( width, height );
  plane_timer.stop();

  Tools::Timer<> loop_timer;
  for ( unsigned short y_base = 0; y_base < height; y_base += SOLIDX_Y_SIZE )
  {
    for ( unsigned short x_base = 0; x_base < width; x_base += SOLIDX_X_SIZE )
    {
      // Phase A/B split: compute the block from immutable inputs, then stitch it into
      // the writer in this fixed block order so the output stays byte-identical.
      BlockResult result = ComputeSolidBlock( x_base, y_base, plane );
      StitchBlock( mapwriter, x_base, y_base, result );
    }
    INFO_PRINT( "\rConverting: {}%", y_base * 100 / height );
  }
  loop_timer.stop();

  Tools::Timer<> flush_timer;
  mapwriter.WriteConfigFile();
  mapwriter.Flush();  // surface any write errors before reporting success
  flush_timer.stop();

  long long total_ms = init_timer.ellapsed() + mapread_timer.ellapsed() +
                       staticread_timer.ellapsed() + plane_timer.ellapsed() +
                       loop_timer.ellapsed() + flush_timer.ellapsed();

  INFO_PRINTLN(
      "\rConversion complete.\n"
      "Conversion details:\n"
      "  Total blocks: {}\n"
      "  Blocks with solids: {} ({}%)\n"
      "  Blocks without solids: {} ({}%)\n"
      "  Locations with solids: {}\n"
      "  Total number of solids: {}\n"
      "Timing:\n"
      "  Init files:   {} ms\n"
      "  Map read:     {} ms\n"
      "  Static read:  {} ms\n"
      "  Terrain plane:{} ms\n"
      "  Process loop: {} ms\n"
      "  Config+flush: {} ms\n"
      "  Total:        {} ms",
      empty + nonempty, nonempty, ( nonempty * 100 / ( empty + nonempty ) ), empty,
      ( empty * 100 / ( empty + nonempty ) ), with_more_solids, total_statics,
      init_timer.ellapsed(), mapread_timer.ellapsed(), staticread_timer.ellapsed(),
      plane_timer.ellapsed(), loop_timer.ellapsed(), flush_timer.ellapsed(), total_ms );

  if ( cfg_profile )
  {
    // Sub-totals of the process loop (see prof_* members). Their sum is slightly below the
    // loop time; the remainder is block bookkeeping plus the chrono-read overhead itself.
    auto to_ms = []( long long ns ) { return ns / 1'000'000; };
    INFO_PRINTLN(
        "Process loop breakdown (profile=1):\n"
        "    Map-info:    {} ms\n"
        "    Statics:     {} ms\n"
        "    Shape-build: {} ms\n"
        "    Writer:      {} ms\n"
        "    Sum:         {} ms",
        to_ms( prof_mapinfo_ns ), to_ms( prof_statics_ns ), to_ms( prof_shape_ns ),
        to_ms( prof_writer_ns ),
        to_ms( prof_mapinfo_ns + prof_statics_ns + prof_shape_ns + prof_writer_ns ) );
  }
}

// Phase A (pure compute): produce this block's cells, solids, and block-local index
// data from immutable inputs only. Deliberately touches no MapWriter and no
// UoConvertMain counters -- stats and warnings accumulate into the returned
// BlockResult so this can later run concurrently across blocks. StitchBlock() folds
// the result into the MapWriter in block order.
BlockResult UoConvertMain::ComputeSolidBlock( unsigned short x_base, unsigned short y_base,
                                              const TerrainPlane& plane ) const
{
  // Raw UO tile flags fetched for every tile's statics below. Any static reaching
  // `statics` is guaranteed to have at least one of these bits set, so nothing
  // downstream needs to re-check srec.flags against this same mask.
  static constexpr unsigned int kSolidStaticFlags =
      USTRUCT_TILE::FLAG_BLOCKING | USTRUCT_TILE::FLAG_PLATFORM |
      USTRUCT_TILE::FLAG_HALF_HEIGHT | USTRUCT_TILE::FLAG_LIQUID | USTRUCT_TILE::FLAG_HOVEROVER;

  BlockResult result;

  // Block-local, 0-based count of solid elements appended so far in this block. Replaces
  // the old `NextSolidIndex() - baseindex` math (which was already block-local); the
  // stitch rebases it against the live solids buffer.
  unsigned int local_elems = 0;

  unsigned short x_add_max = SOLIDX_X_SIZE, y_add_max = SOLIDX_Y_SIZE;
  if ( x_base + x_add_max > uo_map_width )
    x_add_max = uo_map_width - x_base;
  if ( y_base + y_add_max > uo_map_height )
    y_add_max = uo_map_height - y_base;
  result.x_add_max = x_add_max;
  result.y_add_max = y_add_max;

  // Reused across tiles (cleared, not reconstructed) to avoid a per-tile
  // allocation across the ~25M tiles a full map conversion visits.
  StaticList statics;
  std::vector<MapShape> shapes;

  // Opt-in per-tile profiling: `t` is a rolling cursor and lap() folds the elapsed time
  // since the last cursor into an accumulator, advancing the cursor. Guarded by cfg_profile
  // at every call site so a normal run does no chrono reads at all.
  using ProfClock = std::chrono::high_resolution_clock;
  ProfClock::time_point t;
  auto lap = [&]( long long& acc )
  {
    auto now = ProfClock::now();
    acc += std::chrono::duration_cast<std::chrono::nanoseconds>( now - t ).count();
    t = now;
  };

  for ( unsigned short x_add = 0; x_add < x_add_max; ++x_add )
  {
    for ( unsigned short y_add = 0; y_add < y_add_max; ++y_add )
    {
      unsigned short x = x_base + x_add;
      unsigned short y = y_base + y_add;

      statics.clear();

      if ( cfg_profile )
        t = ProfClock::now();

      // read the precomputed smoothed terrain, and treat it like a static.
      const std::size_t plane_idx = plane.index( x, y );
      const u16 landtile = plane.landtile[plane_idx];
      short z = plane.eff_z[plane_idx];  // effective z: the liquid override is already folded in

      if ( landtile > 0x3FFF )
        result.warnings.push_back(
            fmt::format( "Tile {:#x} at ({},{},{}) is an invalid ID!", landtile, x, y, z ) );

      short low_z = plane.low_z[plane_idx];

      short lt_height = z - low_z;
      z = low_z;

      if ( landtile > 0x3FFF )
        result.warnings.push_back(
            fmt::format( "Tile {:#x} at ({},{},{}) is an invalid ID!", landtile, x, y, z ) );

      unsigned int lt_flags = Plib::landtile_uoflags_read( landtile );
      if ( ~lt_flags & USTRUCT_TILE::FLAG_BLOCKING )
      {  // this seems to be the default.
        lt_flags |= USTRUCT_TILE::FLAG_PLATFORM;
      }
      lt_flags |=
          USTRUCT_TILE::FLAG_NO_SHOOT;  // added to make sure people using noshoot will have shapes
      // generated by this tile in future block LOS, shouldn't
      // affect people using old LOS method one way or another.
      lt_flags |= USTRUCT_TILE::FLAG_FLOOR;
      lt_flags |= USTRUCT_TILE::FLAG_HALF_HEIGHT;  // the entire map is this way

      if ( lt_flags & USTRUCT_TILE::FLAG_WALL )
        lt_height = 20;

      if ( cfg_profile )
        lap( result.prof_mapinfo_ns );

      readstatics( statics, x, y, kSolidStaticFlags );

      std::erase_if( statics,
                     [this]( const StaticRec& srec )
                     {
                       unsigned int polflags = polflags_from_tileflags(
                           srec.graphic, srec.flags, cfg_use_no_shoot, cfg_LOS_through_windows );
                       return ( ~polflags & FLAG::MOVELAND ) && ( ~polflags & FLAG::MOVESEA ) &&
                              ( ~polflags & FLAG::BLOCKSIGHT ) && ( ~polflags & FLAG::BLOCKING ) &&
                              ( ~polflags & FLAG::OVERFLIGHT );
                     } );

      if ( cfg_profile )
        lap( result.prof_statics_ns );

      bool addMap = true;

      for ( const auto& srec : statics )
      {
        // Look for water tiles. If there are any, discard the map (which is usually at -15 anyway)
        if ( z + lt_height <= srec.z &&
             // only where the map is below or same Z as the static
             ( ( srec.z - ( z + lt_height ) ) <= 10 ) && DiscardedWaterTypes.count( srec.graphic ) )
        {
          // arr, there be water here
          addMap = false;
        }

        // if there's a static on top of one of these "wall" landtiles, make it override.
        if ( ( lt_flags & USTRUCT_TILE::FLAG_WALL ) &&  // wall?
             z <= srec.z && srec.z - z <= lt_height )
        {
          lt_height = srec.z - z;
        }
      }
      // shadows above caves
      if ( is_cave_shadow( landtile ) && !statics.empty() )
      {
        addMap = false;
      }


      // If the map is a NODRAW tile, and there are statics, discard the map tile
      if ( landtile == 2 && !statics.empty() )
        addMap = false;

      if ( addMap )
        statics.emplace_back( 0, static_cast<signed char>( z ), lt_flags,
                              static_cast<char>( lt_height ) );

      sort( statics.begin(), statics.end(), StaticsByZ() );
      reverse( statics.begin(), statics.end() );

      shapes.clear();

      // try to consolidate like shapes, and discard ones we don't care about.
      while ( !statics.empty() )
      {
        StaticRec srec = statics.back();
        statics.pop_back();

        unsigned int polflags = polflags_from_tileflags( srec.graphic, srec.flags, cfg_use_no_shoot,
                                                         cfg_LOS_through_windows );
        if ( ( ~polflags & FLAG::MOVELAND ) && ( ~polflags & FLAG::MOVESEA ) &&
             ( ~polflags & FLAG::BLOCKSIGHT ) && ( ~polflags & FLAG::BLOCKING ) &&
             ( ~polflags & FLAG::OVERFLIGHT ) )
        {
          // Invariant from the filter above: every element still in `statics` has
          // at least one of these bits set, so this can never fire.
          passert_always( 0 );
          continue;
        }

        if ( shapes.empty() )
        {
          // this, whatever it is, is the map base.
          // TODO: look for water statics and use THOSE as the map.
          // these will be converted below to make the map "solid"; the lowest
          // level is always gradual no matter what.
          MapShape shape{ .z = srec.z, .height = 0, .flags = ( polflags & 0xFFu ) | FLAG::GRADUAL };
          shapes.push_back( shape );

          // for wall flag - map tile always height 0, at bottom. if map tile has height, add it as
          // a static
          if ( srec.height != 0 )
          {
            MapShape _shape{ .z = srec.z, .height = srec.height, .flags = polflags };
            shapes.push_back( _shape );
          }
          continue;
        }

        MapShape& prev = shapes.back();
        // we're adding it.
        MapShape shape{ .z = srec.z, .height = srec.height, .flags = polflags };

        // always add the map shape seperately
        if ( shapes.size() == 1 )
        {
          shapes.push_back( shape );
          continue;
        }

        if ( shape.z < prev.z + prev.height )
        {
          // things can't exist in the same place.
          // shrink the bottom part of this shape.
          // if that would give it negative height, then skip it.
          short height_remove = prev.z + prev.height - shape.z;
          if ( height_remove <= shape.height )
          {
            shape.z += height_remove;
            shape.height -= height_remove;
          }
          else
          {  // example: 5530, 14
            continue;
          }
        }

        // sometimes water has "sand" a couple z-coords above it.
        // We'll try to detect this (really, anything that is up to 4 dist from water)
        // and extend the thing above downward.
        if ( ( prev.flags & FLAG::MOVESEA ) && ( shape.z > prev.z + prev.height ) &&
             ( shape.z <= prev.z + prev.height + 4 ) )
        {
          short height_add = shape.z - prev.z - prev.height;
          shape.z -= height_add;
          shape.height += height_add;
        }
        if ( ( prev.flags & FLAG::MOVESEA ) && ( prev.z + prev.height == -5 ) &&
             ( shape.flags & FLAG::MOVESEA ) && ( shape.z == 25 ) )
        {
          // oddly, there are some water tiles at z=25 in some places...I don't get it
          continue;
        }

        // string prevflags_s = flagstr(prev.flags);
        // const char* prevflags = prevflags_s.c_str();
        // string shapeflags_s = flagstr(shape.flags);
        // const char* shapeflags = shapeflags_s.c_str();

        if ( shape.z > prev.z + prev.height )
        {
          //
          // elevated above what's below, must include separately
          //

          shapes.push_back( shape );
          continue;
        }

        passert_always( shape.z == prev.z + prev.height );

        if ( shape.z == prev.z + prev.height )
        {
          //
          // sitting right on top of the previous solid
          //

          // standable atop non-standable: standable
          // nonstandable atop standable: nonstandable
          // etc
          bool can_combine =
              flags_match( prev.flags, shape.flags, FLAG::BLOCKSIGHT | FLAG::BLOCKING );
          if ( prev.flags & FLAG::MOVELAND && ~shape.flags & FLAG::BLOCKING &&
               ~shape.flags & FLAG::MOVELAND )
          {
            can_combine = false;
          }

          if ( can_combine )
          {
            prev.flags = shape.flags;
            prev.height += shape.height;
          }
          else  // if one blocks LOS, but not the other, they can't be combined this way.
          {
            shapes.push_back( shape );
            continue;
          }
        }
      }

      // the first StaticShape is the map base.
      MapShape base = shapes[0];
      shapes.erase( shapes.begin() );
      MAPCELL cell;
      passert_always( base.height == 0 );
      cell.z = static_cast<signed char>(
          base.z );  // assume now map has height=1. a static was already added if it was >0
      cell.flags = static_cast<u8>( base.flags );
      if ( !shapes.empty() )
        cell.flags |= FLAG::MORE_SOLIDS;

      if ( cfg_profile )
        lap( result.prof_shape_ns );

      result.cells[x_add][y_add] = cell;

      if ( !shapes.empty() )
      {
        ++result.nonempty_locations;
        result.total_statics += static_cast<unsigned int>( shapes.size() );
        result.has_solids = true;

        // Block-local element offset of this cell's first run. Captured before the
        // appends below, so it equals the count of runs emitted by earlier cells of
        // this block -- identical to the old `NextSolidIndex() - baseindex`.
        unsigned int addindex = local_elems;
        if ( addindex > std::numeric_limits<unsigned short>::max() )
          throw std::runtime_error( "addoffset overflow" );
        result.addindex[x_add][y_add] = static_cast<unsigned short>( addindex );
        int count = static_cast<int>( shapes.size() );
        for ( int j = 0; j < count; ++j )
        {
          MapShape shape = shapes[j];
          char _z, height, flags;
          _z = static_cast<char>( shapes[j].z );
          height = static_cast<char>( shape.height );
          flags = static_cast<u8>( shape.flags );
          if ( !height )  // make 0 height solid
          {
            --_z;
            ++height;
          }

          if ( j != count - 1 )
            flags |= FLAG::MORE_SOLIDS;
          SOLIDS_ELEM solid;
          solid.z = _z;
          solid.height = height;
          solid.flags = flags;
          result.solids.push_back( solid );
          ++local_elems;
        }
      }
    }
  }
  return result;
}

// Phase B (stitch): fold one block's compute result into the MapWriter. Runs in the
// exact block order create_map used to run ProcessSolidBlock, so every solids/solidx2
// append offset -- and thus every byte of solidx1/solidx2 -- is identical to the old
// serial path.
void UoConvertMain::StitchBlock( MapWriter& mapwriter, unsigned short x_base,
                                 unsigned short y_base, const BlockResult& result )
{
  // Reduce the per-block stat sums into the run-wide counters.
  with_more_solids += result.nonempty_locations;
  total_statics += result.total_statics;
  if ( cfg_profile )
  {
    prof_mapinfo_ns += result.prof_mapinfo_ns;
    prof_statics_ns += result.prof_statics_ns;
    prof_shape_ns += result.prof_shape_ns;
  }

  // Replay invalid-ID warnings in the order they were produced.
  for ( const auto& w : result.warnings )
    INFO_PRINTLN( "{}", w );

  // Time the MapWriter mutations as the "writer" slice of the loop breakdown (the
  // per-tile writer laps moved out of ComputeSolidBlock, which no longer writes).
  using ProfClock = std::chrono::high_resolution_clock;
  ProfClock::time_point writer_start;
  if ( cfg_profile )
    writer_start = ProfClock::now();

  // Cell writes are disjoint per block; copy the clamped region into the base buffer.
  for ( unsigned short x_add = 0; x_add < result.x_add_max; ++x_add )
    for ( unsigned short y_add = 0; y_add < result.y_add_max; ++y_add )
      mapwriter.SetMapCell( x_base + x_add, y_base + y_add, result.cells[x_add][y_add] );

  if ( !result.has_solids )
  {
    ++empty;
    mapwriter.SetSolidx2Offset( x_base, y_base, 0 );
  }
  else
  {
    ++nonempty;
    SOLIDX2_ELEM idx2_elem{};
    idx2_elem.baseindex = mapwriter.NextSolidIndex();
    // addindex is already block-local and zero for run-less cells -- emit as-is.
    std::memcpy( idx2_elem.addindex, result.addindex, sizeof( idx2_elem.addindex ) );

    // Byte offset this block's solidx2 elem lands at (before appending it).
    unsigned int idx2_offset = mapwriter.NextSolidx2Offset();
    mapwriter.AppendSolidx2Elem( idx2_elem );
    for ( const auto& solid : result.solids )
      mapwriter.AppendSolid( solid );
    mapwriter.SetSolidx2Offset( x_base, y_base, idx2_offset );
  }

  if ( cfg_profile )
  {
    auto now = ProfClock::now();
    prof_writer_ns +=
        std::chrono::duration_cast<std::chrono::nanoseconds>( now - writer_start ).count();
  }
}

std::string UoConvertMain::resolve_type_from_id( unsigned id ) const
{
  if ( BoatTypes.count( id ) )
    return "Boat";
  return "Multi";
}

void UoConvertMain::write_multi_element( FILE* multis_cfg, const USTRUCT_MULTI_ELEMENT& elem,
                                         const std::string& mytype, bool& first )
{
  if ( elem.graphic == GRAPHIC_NODRAW )
    return;

  std::string type = elem.flags ? "static" : "dynamic";

  if ( mytype == "Boat" && first && elem.graphic != 1 )
    type = "static";

  std::string comment;
  if ( cfg_use_new_hsa_format )
  {
    USTRUCT_TILE_HSA tile;
    readtile( elem.graphic, &tile );
    comment.assign( tile.name, sizeof( tile.name ) );
  }
  else
  {
    USTRUCT_TILE tile;
    readtile( elem.graphic, &tile );
    comment.assign( tile.name, sizeof( tile.name ) );
  }

  fprintf( multis_cfg, "    %-7s 0x%04x %4d %4d %4d   // %s\n", type.c_str(), elem.graphic, elem.x,
           elem.y, elem.z, comment.c_str() );

  first = false;
}

void UoConvertMain::write_multi( FILE* multis_cfg, unsigned id,
                                 std::vector<Plib::USTRUCT_MULTI_ELEMENT>& multi_elems )
{
  std::string mytype = resolve_type_from_id( id );

  fprintf( multis_cfg, "%s 0x%x\n", mytype.c_str(), id );
  fprintf( multis_cfg, "{\n" );

  bool first = true;
  for ( const auto& elem : multi_elems )
  {
    write_multi_element( multis_cfg, elem, mytype, first );
  }

  fprintf( multis_cfg, "}\n\n" );
}

void UoConvertMain::write_multi( FILE* multis_cfg, unsigned id, FILE* multi_mul,
                                 unsigned int offset, unsigned int length )
{
  USTRUCT_MULTI_ELEMENT elem;
  unsigned int count =
      cfg_use_new_hsa_format ? length / sizeof( USTRUCT_MULTI_ELEMENT_HSA ) : length / sizeof elem;

  std::string mytype = resolve_type_from_id( id );

  fprintf( multis_cfg, "%s 0x%x\n", mytype.c_str(), id );
  fprintf( multis_cfg, "{\n" );

  if ( fseek( multi_mul, offset, SEEK_SET ) != 0 )
  {
    throw std::runtime_error( "write_multi(): fseek() failed" );
  }


  bool first = true;
  while ( count-- )
  {
    if ( fread( &elem, sizeof elem, 1, multi_mul ) != 1 )
    {
      throw std::runtime_error( "write_multi(): fread() failed" );
    }

    if ( cfg_use_new_hsa_format )
    {
      if ( fseek( multi_mul, 4, SEEK_CUR ) != 0 )
        throw std::runtime_error( "write_multi(): fseek() failed" );
    }

    write_multi_element( multis_cfg, elem, mytype, first );
  }

  fprintf( multis_cfg, "}\n\n" );
}

void UoConvertMain::create_multis_cfg( FILE* multi_idx, FILE* multi_mul, FILE* multis_cfg )
{
  if ( fseek( multi_idx, 0, SEEK_SET ) != 0 )
    throw std::runtime_error( "create_multis_cfg: fseek failed" );
  unsigned count = 0;
  USTRUCT_IDX idxrec;
  for ( int i = 0; fread( &idxrec, sizeof idxrec, 1, multi_idx ) == 1; ++i )
  {
    const USTRUCT_VERSION* vrec = nullptr;

    if ( check_verdata( VERFILE_MULTI_MUL, i, vrec ) )
    {
      write_multi( multis_cfg, i, verfile, vrec->filepos, vrec->length );
      ++count;
    }
    else
    {
      if ( idxrec.offset == 0xFFffFFffLu )
        continue;

      write_multi( multis_cfg, i, multi_mul, idxrec.offset, idxrec.length );
      ++count;
    }
  }
  INFO_PRINTLN( "{} multi definitions written to multis.cfg", count );
}

void UoConvertMain::create_multis_cfg()
{
  std::map<unsigned int, std::vector<USTRUCT_MULTI_ELEMENT>> multi_map;

  std::string outdir = programArgsFindEquals( "outdir=", "." );
  FILE* multis_cfg = fopen( ( outdir + "/multis.cfg" ).c_str(), "wt" );

  if ( open_uopmulti_file( multi_map ) )
  {
    for ( auto& [id, elems] : multi_map )
    {
      write_multi( multis_cfg, id, elems );
    }

    INFO_PRINTLN( "{} multi definitions written to multis.cfg", multi_map.size() );

    return;
  }

  FILE* multi_idx = open_uo_file( "multi.idx" );
  FILE* multi_mul = open_uo_file( "multi.mul" );

  create_multis_cfg( multi_idx, multi_mul, multis_cfg );

  fclose( multis_cfg );

  fclose( multi_idx );
  fclose( multi_mul );
}
void UoConvertMain::write_flags( FILE* fp, unsigned int flags )
{
  if ( flags & FLAG::MOVELAND )
    fprintf( fp, "    MoveLand 1\n" );
  if ( flags & FLAG::MOVESEA )
    fprintf( fp, "    MoveSea 1\n" );
  if ( flags & FLAG::BLOCKSIGHT )
    fprintf( fp, "    BlockSight 1\n" );
  if ( ~flags & FLAG::OVERFLIGHT )
    fprintf( fp, "    OverFlight 0\n" );
  if ( flags & FLAG::ALLOWDROPON )
    fprintf( fp, "    AllowDropOn 1\n" );
  if ( flags & FLAG::GRADUAL )
    fprintf( fp, "    Gradual 1\n" );
  if ( flags & FLAG::STACKABLE )
    fprintf( fp, "    Stackable 1\n" );
  if ( flags & FLAG::BLOCKING )
    fprintf( fp, "    Blocking 1\n" );
  if ( flags & FLAG::MOVABLE )
    fprintf( fp, "    Movable 1\n" );
  if ( flags & FLAG::EQUIPPABLE )
    fprintf( fp, "    Equippable 1\n" );
  if ( flags & FLAG::DESC_PREPEND_A )
    fprintf( fp, "    DescPrependA 1\n" );
  if ( flags & FLAG::DESC_PREPEND_AN )
    fprintf( fp, "    DescPrependAn 1\n" );
}

void UoConvertMain::create_tiles_cfg()
{
  std::string outdir = programArgsFindEquals( "outdir=", "." );
  FILE* fp = fopen( ( outdir + "/tiles.cfg" ).c_str(), "wt" );
  int mountCount;
  char name[21];

  unsigned count = 0;
  for ( unsigned int graphic_i = 0; graphic_i <= Plib::systemstate.config.max_tile_id; ++graphic_i )
  {
    u16 graphic = static_cast<u16>( graphic_i );
    USTRUCT_TILE tile;
    if ( cfg_use_new_hsa_format )
    {
      USTRUCT_TILE_HSA newtile;
      read_objinfo( graphic, newtile );
      tile.anim = newtile.anim;
      tile.flags = newtile.flags;
      tile.height = newtile.height;
      tile.layer = newtile.layer;
      memcpy( tile.name, newtile.name, sizeof tile.name );
      tile.unk14 = newtile.unk14;
      tile.unk15 = newtile.unk15;
      tile.unk6 = newtile.unk6;
      tile.unk7 = newtile.unk7;
      tile.unk8 = newtile.unk8;
      tile.unk9 = newtile.unk9;
      tile.weight = newtile.weight;
    }
    else
      read_objinfo( graphic, tile );
    mountCount = static_cast<int>( MountTypes.count( graphic ) );

    if ( tile.name[0] == '\0' && tile.flags == 0 && tile.layer == 0 && tile.height == 0 &&
         mountCount == 0 )
    {
      continue;
    }
    unsigned int flags =
        polflags_from_tileflags( graphic, tile.flags, cfg_use_no_shoot, cfg_LOS_through_windows );
    if ( mountCount != 0 )
    {
      tile.layer = 25;
      flags |= FLAG::EQUIPPABLE;
    }

    memset( name, 0, sizeof name );
    memcpy( name, tile.name, sizeof tile.name );

    fprintf( fp, "tile 0x%x\n", graphic );
    fprintf( fp, "{\n" );
    fprintf( fp, "    Desc %s\n", name );
    fprintf( fp, "    UoFlags 0x%08lx\n", static_cast<unsigned long>( tile.flags ) );
    if ( tile.layer )
      fprintf( fp, "    Layer %u\n", tile.layer );
    if ( flags & FLAG::EQUIPPABLE )
      fprintf( fp, "    AnimID %u\n", tile.anim );
    if ( static_cast<unsigned long>( tile.flags ) & USTRUCT_TILE::FLAG_PARTIAL_HUE )
      fprintf( fp, "    PartialHue 1\n" );
    fprintf( fp, "    Height %u\n", tile.height );
    fprintf( fp, "    Weight %u\n", tile.weight );
    write_flags( fp, flags );
    fprintf( fp, "}\n" );
    fprintf( fp, "\n" );
    ++count;
  }
  fclose( fp );

  INFO_PRINTLN( "{} tile definitions written to tiles.cfg", count );
}

void UoConvertMain::create_landtiles_cfg()
{
  std::string outdir = programArgsFindEquals( "outdir=", "." );
  FILE* fp = fopen( ( outdir + "/landtiles.cfg" ).c_str(), "wt" );
  unsigned count = 0;

  for ( u16 i = 0; i <= 0x3FFF; ++i )
  {
    USTRUCT_LAND_TILE landtile;
    if ( cfg_use_new_hsa_format )
    {
      USTRUCT_LAND_TILE_HSA newlandtile;
      readlandtile( i, &newlandtile );
      landtile.flags = newlandtile.flags;
      landtile.unk = newlandtile.unk;
      memcpy( landtile.name, newlandtile.name, sizeof landtile.name );
    }
    else
      readlandtile( i, &landtile );

    if ( landtile.name[0] || landtile.flags )
    {
      fprintf( fp, "landtile 0x%x\n", i );
      fprintf( fp, "{\n" );
      fprintf( fp, "    Name %s\n", landtile.name );
      fprintf( fp, "    UoFlags 0x%08lx\n", static_cast<unsigned long>( landtile.flags ) );

      unsigned int flags = polflags_from_landtileflags( i, landtile.flags );
      flags &= ~FLAG::MOVABLE;  // movable makes no sense for landtiles
      write_flags( fp, flags );
      fprintf( fp, "}\n" );
      fprintf( fp, "\n" );
      ++count;
    }
  }
  fclose( fp );

  INFO_PRINTLN( "{} landtile definitions written to landtiles.cfg", count );
}

int UoConvertMain::main()
{
  const std::vector<std::string>& binArgs = programArgs();

  /**********************************************
   * show help
   **********************************************/
  if ( binArgs.size() == 1 )
  {
    showHelp();
    return 0;  // return "okay"
  }

  // Setups uoconvert by finding the path of uo files and max tiles from pol.cfg or command
  // line arguments. Also loads parameters from uoconvert.cfg.
  setup_uoconvert();

  std::string command = binArgs[1];
  if ( command == "uoptomul" )
  {
    if ( !convert_uop_to_mul() )
      return 1;
  }
  else if ( command == "map" )
  {
    UoConvert::uo_mapid = programArgsFindEquals( "mapid=", 0, false );
    UoConvert::uo_usedif = programArgsFindEquals( "usedif=", 1, false );
    UoConvert::uo_readuop = (bool)programArgsFindEquals( "readuop=", 1, false );
    cfg_profile = (bool)programArgsFindEquals( "profile=", 0, false );

    std::string realm = programArgsFindEquals( "realm=", "britannia" );

    UoConvert::open_uo_data_files();
    UoConvert::read_uo_data();

    // Auto-detects defaults for mapid=0 or 1 based on the map size. All other sizes are fixed based
    // on the mapid.
    Pol::Plib::MUL::MapInfo mapinfo( UoConvert::uo_mapid, UoConvert::uo_map_size );
    int default_width = mapinfo.width();
    int default_height = mapinfo.height();

    if ( mapinfo.guessed() )
      INFO_PRINTLN( "Auto-detected map dimensions: {}x{}", default_width, default_height );

    uo_map_width =
        static_cast<unsigned short>( programArgsFindEquals( "width=", default_width, false ) );
    uo_map_height =
        static_cast<unsigned short>( programArgsFindEquals( "height=", default_height, false ) );

    check_for_errors_in_map_parameters();

    int x = programArgsFindEquals( "x=", -1, false );
    int y = programArgsFindEquals( "y=", -1, false );

    // britannia: realm=main mapid=0 width=6144 height=4096
    // ilshenar: realm=ilshenar mapid=2 width=2304 height=1600
    // malas: realm=malas mapid=3 width=2560 height=2048
    // tokuno: realm=tokuno mapid=4 width=1448 height=1448
    // termur: realm=termur mapid=5 width=1280 height=4096

    if ( x >= 0 && y >= 0 )
    {
      UoConvertMain::update_map( realm, static_cast<u16>( x ), static_cast<u16>( y ) );
    }
    else
    {
      UoConvertMain::create_map( realm, static_cast<unsigned short>( uo_map_width ),
                                 static_cast<unsigned short>( uo_map_height ) );
    }
  }
  else if ( command == "statics" )
  {
    std::string realm = programArgsFindEquals( "realm=", "britannia" );
    Plib::RealmDescriptor descriptor = Plib::RealmDescriptor::Load( realm );

    UoConvert::uo_mapid = descriptor.uomapid;
    UoConvert::uo_usedif = descriptor.uodif;
    UoConvert::uo_map_width = static_cast<unsigned short>( descriptor.width );
    UoConvert::uo_map_height = static_cast<unsigned short>( descriptor.height );

    UoConvert::open_uo_data_files();
    UoConvert::read_uo_data();

    write_pol_static_files( realm );
  }
  else if ( command == "multis" )
  {
    UoConvert::open_uo_data_files();
    UoConvert::read_uo_data();
    UoConvertMain::create_multis_cfg();
  }
  else if ( command == "tiles" )
  {
    UoConvert::open_uo_data_files();
    UoConvert::read_uo_data();
    UoConvertMain::create_tiles_cfg();
  }
  else if ( command == "landtiles" )
  {
    UoConvert::open_uo_data_files();
    UoConvert::read_uo_data();
    UoConvertMain::create_landtiles_cfg();
  }
  else if ( command == "maptile" )
  {
    std::string realm = programArgsFindEquals( "realm=", "britannia" );
    Plib::RealmDescriptor descriptor = Plib::RealmDescriptor::Load( realm );

    UoConvert::uo_mapid = descriptor.uomapid;
    UoConvert::uo_usedif = descriptor.uodif;
    UoConvert::uo_map_width = static_cast<unsigned short>( descriptor.width );
    UoConvert::uo_map_height = static_cast<unsigned short>( descriptor.height );

    UoConvert::open_uo_data_files();
    UoConvert::read_uo_data();

    UoConvertMain::create_maptile( realm );
  }
  else if ( command == "flags" )
  {
    UoConvertMain::display_flags();
  }
  else  // unknown option
  {
    showHelp();
    return 1;
  }
  UoConvert::clear_tiledata();
  return 0;
}
void UoConvertMain::check_for_errors_in_map_parameters()
{
  if ( !MUL::Map::valid_size( UoConvert::uo_map_size, uo_map_width, uo_map_height ) )
  {
    size_t expected_size =
        MUL::Map::blockSize * MUL::Map::expected_blocks( uo_map_width, uo_map_height );

    INFO_PRINTLN( "\nWarning: Width and height do not match the map size ({} bytes, expected {})",
                  UoConvert::uo_map_size, expected_size );

    if ( uo_map_width == 0 || uo_map_height == 0 )
      throw std::runtime_error(
          "Width and height were not identified automatically. Please specify them manually." );


    if ( ( uo_map_width % MUL::Map::blockWidth != 0 ) ||
         ( uo_map_height % MUL::Map::blockHeight != 0 ) )
      throw std::runtime_error( "Width and height must be divisible by 8" );

    if ( uo_map_size < expected_size )
      throw std::runtime_error( "Map size is smaller than the given width and height" );
  }
}
bool UoConvertMain::convert_uop_to_mul()
{
  // this is kludgy and doesn't take into account the UODataPath. Mostly a proof of concept now.
  UoConvert::uo_mapid = programArgsFindEquals( "mapid=", 0, false );

  std::string mul_mapfile = "map" + to_string( uo_mapid ) + ".mul";
  std::string uop_mapfile = "map" + to_string( uo_mapid ) + "LegacyMUL.uop";

  auto maphash = []( int mapid, size_t chunkidx )
  { return HashLittle2( fmt::format( "build/map{}legacymul/{:08d}.dat", mapid, chunkidx ) ); };

  std::ifstream ifs( uop_mapfile, std::ifstream::binary );
  if ( !ifs )
  {
    ERROR_PRINTLN( "Error when opening mapfile: {}", uop_mapfile );
    return false;
  }

  kaitai::kstream ks( &ifs );
  uop_t uopfile( &ks );

  // TODO: read all blocks
  std::map<uint64_t, uop_t::file_t*> filemap;
  uop_t::block_addr_t* currentblock = uopfile.header()->firstblock();
  for ( auto file : *currentblock->block_body()->files() )
  {
    if ( file == nullptr )
      continue;
    if ( file->decompressed_size() == 0 )
      continue;
    filemap[file->filehash()] = file;
  }

  if ( uopfile.header()->nfiles() != filemap.size() )
    INFO_PRINTLN( "Warning: not all chunks read ({}/{})", filemap.size(),
                  uopfile.header()->nfiles() );

  std::ofstream ofs( mul_mapfile, std::ofstream::binary );
  for ( size_t i = 0; i < filemap.size(); i++ )
  {
    auto fileitr = filemap.find( maphash( uo_mapid, i ) );
    if ( fileitr == filemap.end() )
    {
      INFO_PRINTLN( "Couldn't find file hash: {}", maphash( uo_mapid, i ) );
      continue;
    }

    auto file = fileitr->second;
    ofs << file->data()->filebytes();
    INFO_PRINTLN( "Wrote: {}/{}", i + 1, filemap.size() );
  }
  INFO_PRINTLN( "Done converting." );

  return true;
}
void UoConvertMain::setup_uoconvert()
{
  std::string uodata_root = programArgsFindEquals( "uodata=", "" );
  unsigned short max_tile =
      static_cast<unsigned short>( programArgsFindEquals( "maxtileid=", 0x0, true ) );

  if ( max_tile )
  {
    INFO_PRINTLN( "Warning: maxtileid will be ignored and detected from tiledata.mul instead." );
  }

  // read required parameters from pol.cfg
  if ( uodata_root.empty() )
  {
    INFO_PRINTLN( "Reading pol.cfg." );
    Clib::ConfigFile cf( "pol.cfg" );
    Clib::ConfigElem elem;

    cf.readraw( elem );

    if ( uodata_root.empty() )
      uodata_root = Plib::UOInstallFinder::remove_elem( elem );
  }

  // Save the parameters into this ugly global state we have
  Plib::systemstate.config.uo_datafile_root = Clib::normalized_dir_form( uodata_root );

  // Load parameters from uoconvert.cfg (multi types, mounts, etc)
  load_uoconvert_cfg();
}

void parse_graphics_properties( Clib::ConfigElem& elem, const std::string& prop_name,
                                std::set<unsigned int>& dest )
{
  std::string prop_value;
  std::string graphicnum;

  if ( !elem.has_prop( prop_name.c_str() ) )
  {
    elem.throw_prop_not_found( prop_name );
  }

  while ( elem.remove_prop( prop_name.c_str(), &prop_value ) )
  {
    ISTRINGSTREAM is( prop_value );
    while ( is >> graphicnum )
    {
      dest.insert( strtoul( graphicnum.c_str(), nullptr, 0 ) );
    }
  }
}

void notice_deprecated( Clib::ConfigElem& elem, const std::string& prop_name )
{
  if ( elem.has_prop( prop_name.c_str() ) )
  {
    INFO_PRINTLN( "Note: specifying {} in MultiTypes is no longer needed.", prop_name );
  }
}

void UoConvertMain::load_uoconvert_cfg()
{
  std::string main_cfg = "uoconvert.cfg";
  if ( Clib::FileExists( main_cfg ) )
  {
    Clib::ConfigElem elem;
    INFO_PRINTLN( "Reading uoconvert.cfg." );
    Clib::ConfigFile cf_main( main_cfg );
    while ( cf_main.read( elem ) )
    {
      if ( elem.type_is( "MultiTypes" ) )
      {
        parse_graphics_properties( elem, "Boats", BoatTypes );
        notice_deprecated( elem, "Houses" );
        notice_deprecated( elem, "Stairs" );
      }
      else if ( elem.type_is( "LOSOptions" ) )
      {
        if ( elem.has_prop( "UseNoShoot" ) )
          UoConvertMain::cfg_use_no_shoot = elem.remove_bool( "UseNoShoot" );

        if ( elem.has_prop( "LOSThroughWindows" ) )
          UoConvertMain::cfg_LOS_through_windows = elem.remove_bool( "LOSThroughWindows" );
      }
      else if ( elem.type_is( "Mounts" ) )
      {
        parse_graphics_properties( elem, "Tiles", MountTypes );
      }
      else if ( elem.type_is( "StaticOptions" ) )
      {
        if ( elem.has_prop( "MaxStaticsPerBlock" ) )
        {
          UoConvert::cfg_max_statics_per_block = elem.remove_int( "MaxStaticsPerBlock" );

          if ( UoConvert::cfg_max_statics_per_block > MAX_STATICS_PER_BLOCK )
          {
            UoConvert::cfg_max_statics_per_block = MAX_STATICS_PER_BLOCK;
            INFO_PRINTLN( "max. Statics per Block limited to {} Items",
                          UoConvert::cfg_max_statics_per_block );
          }
          else if ( UoConvert::cfg_max_statics_per_block < 0 )
            UoConvert::cfg_max_statics_per_block = 1000;
        }

        if ( elem.has_prop( "WarningStaticsPerBlock" ) )
        {
          UoConvert::cfg_warning_statics_per_block = elem.remove_int( "WarningStaticsPerBlock" );

          if ( UoConvert::cfg_warning_statics_per_block > MAX_STATICS_PER_BLOCK )
          {
            UoConvert::cfg_warning_statics_per_block = MAX_STATICS_PER_BLOCK;
            INFO_PRINTLN( "max. Statics per Block for Warning limited to {} Items",
                          UoConvert::cfg_warning_statics_per_block );
          }
          else if ( UoConvert::cfg_warning_statics_per_block < 0 )
            UoConvert::cfg_warning_statics_per_block = 1000;
        }

        if ( elem.has_prop( "DiscardedWaterTiles" ) )
        {
          parse_graphics_properties( elem, "DiscardedWaterTiles", DiscardedWaterTypes );
        }
        else
          for ( int i = 0x1796; i <= 0x17B2; ++i )
            DiscardedWaterTypes.insert( i );

        if ( elem.has_prop( "ShowIllegalGraphicWarning" ) )
          UoConvert::cfg_show_illegal_graphic_warning =
              elem.remove_bool( "ShowIllegalGraphicWarning" );
      }
      else if ( elem.type_is( "TileOptions" ) )
      {
        if ( elem.has_prop( "ShowRoofAndPlatformWarning" ) )
          cfg_show_roof_and_platform_warning = elem.remove_bool( "ShowRoofAndPlatformWarning" );
      }
      else if ( elem.type_is( "ClientOptions" ) )
      {
        if ( elem.has_prop( "UseNewHSAFormat" ) )
          INFO_PRINTLN( "Warning: UseNewHSAFormat in uoconvert.cfg is no longer needed." );
      }
    }
  }
}
}  // namespace Pol::UoConvert


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
  Pol::UoConvert::UoConvertMain* UoConvertMain = new Pol::UoConvert::UoConvertMain();
  UoConvertMain->start( argc, argv );
}
