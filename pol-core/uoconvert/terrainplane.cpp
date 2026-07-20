#include "terrainplane.h"

#include <cstddef>
#include <cstdlib>

#include "../plib/clidata.h"
#include "../plib/uofile.h"
#include "../plib/ustruct.h"
#include "parallel.h"

namespace Pol::UoConvert
{
void TerrainPlane::build( int w, int h, bool need_low_z, unsigned threads )
{
  width = w;
  height = h;
  const std::size_t n = static_cast<std::size_t>( w ) * static_cast<std::size_t>( h );
  landtile.resize( n );
  avg_z.resize( n );
  eff_z.resize( n );
  if ( need_low_z )
    low_z.resize( n );

  // Bulk-extract the raw landtile ids and raw cell z's in one sequential pass, instead of
  // four block-indexed rawmapinfo lookups per corner per tile.
  std::vector<s8> raw_z( n );
  Plib::rawmap_extract_planes( landtile, raw_z );

  // Pass 1: avg_z + eff_z from the flat raw arrays. This replicates safe_getmapinfo
  // exactly -- its x+1/y+1 edge clamping, the min-differential diagonal choice, and the
  // round-toward-negative-infinity halving -- but reads the four corners from flat memory
  // rather than recomputing UO block-index math and copying a USTRUCT_MAPINFO each time.
  parallel_for(
      static_cast<std::size_t>( h ),
      [&]( std::size_t yy )
      {
        const int y = static_cast<int>( yy );
        const int yp = ( y + 1 < h ) ? y + 1 : h - 1;
        const std::size_t row = static_cast<std::size_t>( y ) * static_cast<std::size_t>( w );
        const std::size_t rowp = static_cast<std::size_t>( yp ) * static_cast<std::size_t>( w );
        for ( int x = 0; x < w; ++x )
        {
          const int xp = ( x + 1 < w ) ? x + 1 : w - 1;
          const short z1 = raw_z[row + xp];
          const short z2 = raw_z[row + x];
          const short z3 = raw_z[rowp + x];
          const short z4 = raw_z[rowp + xp];
          const short zsum = ( std::abs( z1 - z3 ) < std::abs( z2 - z4 ) )
                                 ? static_cast<short>( z1 + z3 )
                                 : static_cast<short>( z2 + z4 );
          const s8 avg = static_cast<s8>( zsum >= 0 ? zsum / 2 : ( zsum - 1 ) / 2 );

          const std::size_t i = row + x;
          avg_z[i] = avg;

          // Caller-side liquid override (UoConvertMain ProcessSolidBlock / create_maptile):
          // for water, don't average with surrounding tiles -- use the raw cell z. This lands
          // in eff_z only; avg_z stays the plain average because get_lowestadjacentz reads
          // neighbors' *un-overridden* averages.
          eff_z[i] =
              ( Plib::landtile_uoflags_read( landtile[i] ) & Plib::USTRUCT_TILE::FLAG_LIQUID )
                  ? raw_z[i]
                  : avg;
        }
      },
      threads );

  if ( !need_low_z )
    return;

  // Pass 2: get_lowestadjacentz over the pass-1 arrays. Mirrors the original exactly:
  // start from the center's effective z, then min-reduce the eight in-bounds neighbors'
  // averages, substituting the center's z for cave-shadow/cave-exit neighbors and
  // returning the center's z outright if any neighbor is a no-draw tile.
  parallel_for(
      static_cast<std::size_t>( h ),
      [&]( std::size_t yy )
      {
        const int y = static_cast<int>( yy );
        for ( int x = 0; x < w; ++x )
        {
          const std::size_t ci = static_cast<std::size_t>( y ) * static_cast<std::size_t>( w ) + x;
          const short z_center = eff_z[ci];
          short lowest = z_center;
          bool cave_override = false;

          auto consider = [&]( int nx, int ny )
          {
            const std::size_t ni =
                static_cast<std::size_t>( ny ) * static_cast<std::size_t>( w ) + nx;
            short z0 = avg_z[ni];
            const u16 lt = landtile[ni];
            // raw mul data can hold ids past the landtile range (the conversion warns
            // about them later); out-of-range ids classify as plain terrain, exactly
            // like the == chains they replace.
            const u8 cls = lt < landtile_class.size() ? landtile_class[lt] : 0;
            if ( cls & LandtileClass::CAVE )
              z0 = z_center;
            if ( cls & LandtileClass::NO_DRAW )
              cave_override = true;
            if ( z0 < lowest )
              lowest = z0;
          };

          if ( x - 1 >= 0 && y - 1 >= 0 )
            consider( x - 1, y - 1 );
          if ( x - 1 >= 0 )
            consider( x - 1, y );
          if ( x - 1 >= 0 && y + 1 < h )
            consider( x - 1, y + 1 );
          if ( y - 1 >= 0 )
            consider( x, y - 1 );
          if ( y - 1 >= 0 && x + 1 < w )
            consider( x + 1, y - 1 );
          if ( x + 1 < w )
            consider( x + 1, y );
          if ( x + 1 < w && y + 1 < h )
            consider( x + 1, y + 1 );
          if ( y + 1 < h )
            consider( x, y + 1 );

          low_z[ci] = static_cast<s8>( cave_override ? z_center : lowest );
        }
      },
      threads );
}

}  // namespace Pol::UoConvert
