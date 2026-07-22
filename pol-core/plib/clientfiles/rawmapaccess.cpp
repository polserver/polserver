/** @file
 * Raw map accessors over the RawMap full-read cache: getmapinfo (with
 * neighbour z-averaging), safe_getmapinfo, rawmap_extract_planes.
 *
 * @par History
 * - 2005/01/13 Shinigami: safe_getmapinfo - add missing checks for x and y
 * - 2005/01/23 Shinigami: rawmapinfo - used constant WORLD_X/Y... bad idea
 */


#include <cstdio>
#include <map>
#include <stdexcept>
#include <stdlib.h>

#include "clib/passert.h"
#include "clib/rawtypes.h"
#include "clidata.h"
#include "poltype.h"
#include "uoclientfiles.h"
#include "ustruct.h"

#include "RawMap.h"


namespace Pol::Plib
{
void UoClientFiles::read_map_difs()
{
  num_map_patches = rawmap.load_map_difflist( mapdifl_file );
}

bool UoClientFiles::rawmap_loaded() const
{
  return rawmap_ready;
}

void UoClientFiles::rawmapfullread()
{
  rawmap.set_bounds( uo_map_width, uo_map_height );

  unsigned int blocks = 0;
  if ( mapfile == nullptr )
    blocks = rawmap.load_full_map( uo_mapid, uopmapfile );
  else
    blocks = rawmap.load_full_map( mapfile, mapdif_file );

  if ( blocks )
    rawmap_ready = true;
}

void UoClientFiles::rawmap_extract_planes( std::span<u16> landtile_out,
                                           std::span<s8> z_out ) const
{
  passert_always( rawmap_ready );  // caller must rawmapfullread() first
  rawmap.extract_planes( landtile_out, z_out );
}
}  // namespace Pol::Plib
