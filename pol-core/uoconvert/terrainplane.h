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
  void build( int w, int h, bool need_low_z = true );

  std::size_t index( int x, int y ) const
  {
    return static_cast<std::size_t>( y ) * width + x;
  }
};

// Landtile classifiers used by the plane build and by ProcessSolidBlock. They inspect
// only the landtile id (the pre-refactor versions took a whole USTRUCT_MAPINFO but read
// nothing but .landtile).
bool is_no_draw( u16 landtile );
bool is_cave_exit( u16 landtile );
bool is_cave_shadow( u16 landtile );

}  // namespace Pol::UoConvert

#endif  // POL_UOCONVERT_TERRAINPLANE_H
