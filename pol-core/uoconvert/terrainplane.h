#ifndef POL_UOCONVERT_TERRAINPLANE_H
#define POL_UOCONVERT_TERRAINPLANE_H

#include <cstddef>
#include <vector>

#include "../clib/rawtypes.h"

namespace Pol::UoConvert
{
// Precomputed per-tile smoothed-terrain data for a whole realm, built once from the
// already-in-RAM raw map. It replaces the ~36 raw cell fetches ProcessSolidBlock did
// per tile (one safe_getmapinfo directly plus eight through get_lowestadjacentz, each
// four rawmapinfo lookups) with flat array reads, and lets create_maptile share the
// same work. Every tile's smoothed z is now computed exactly once.
//
// All arrays are width*height, row-major (idx = y*width + x). The plane is immutable
// after build(), so concurrent const reads are safe (spec 04 parallelizes the block
// loop over it).
struct TerrainPlane
{
  int width = 0;
  int height = 0;

  std::vector<u16> landtile;  // raw mul landtile id
  std::vector<s8> avg_z;      // safe_getmapinfo 4-corner average, WITHOUT the liquid
                              //   override -- this is the value neighbors contribute to
                              //   the lowest-adjacent-z reduction.
  std::vector<s8> eff_z;      // avg_z with the caller-side FLAG_LIQUID -> raw cell z
                              //   override applied: the effective tile z used by all
                              //   consumers and as the *center* value in the reduction.
  std::vector<s8> low_z;      // get_lowestadjacentz result.

  // Build from the loaded raw map (call rawmapfullread() first). w/h are the realm
  // dimensions (== uo_map_width / uo_map_height). Pass need_low_z=false to skip the
  // lowest-adjacent-z reduction (Pass 2) when only landtile/eff_z are needed, e.g.
  // create_maptile -- it leaves low_z empty.
  //
  // Both passes are pure indexed writes (each tile writes only its own index), so
  // they run in parallel over row bands: threads==0 uses hardware_concurrency,
  // threads==1 forces serial. Output is byte-identical regardless of thread count.
  void build( int w, int h, bool need_low_z = true, unsigned threads = 0 );

  std::size_t index( int x, int y ) const { return static_cast<std::size_t>( y ) * width + x; }
};

// Landtile classifiers used by the plane build and by the solid-block conversion.
// The id lists are hardcoded client landtile ids (see tiledata.mul / landtiles.cfg:
// 0x2 = NODRAW; 0x7ec-0x7f1, 0x834-0x839, 0x1d3-0x1da = cave entrance floors/edges;
// 0x1ae-0x1b5, 0x1db = the black "shadow" tiles above cave interiors). They have
// carried through from the original UOConvert; audit against the client's tiledata
// if a newer client ever renumbers them.
constexpr bool is_no_draw( u16 landtile )
{
  return ( landtile == 0x2 );
}

constexpr bool is_cave_exit( u16 landtile )
{
  return ( landtile == 0x7ec || landtile == 0x7ed || landtile == 0x7ee || landtile == 0x7ef ||
           landtile == 0x7f0 || landtile == 0x7f1 || landtile == 0x834 || landtile == 0x835 ||
           landtile == 0x836 || landtile == 0x837 || landtile == 0x838 || landtile == 0x839 ||
           landtile == 0x1d3 || landtile == 0x1d4 || landtile == 0x1d5 || landtile == 0x1d6 ||
           landtile == 0x1d7 || landtile == 0x1d8 || landtile == 0x1d9 || landtile == 0x1da );
}

constexpr bool is_cave_shadow( u16 landtile )
{
  return ( landtile == 0x1db ||  // shadows above caves
           landtile == 0x1ae ||  // more shadows above caves
           landtile == 0x1af || landtile == 0x1b0 || landtile == 0x1b1 || landtile == 0x1b2 ||
           landtile == 0x1b3 || landtile == 0x1b4 || landtile == 0x1b5 );
}

}  // namespace Pol::UoConvert

#endif  // POL_UOCONVERT_TERRAINPLANE_H
