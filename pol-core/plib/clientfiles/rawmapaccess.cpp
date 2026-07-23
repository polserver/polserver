/** @file
 * UoClientFiles coordinator delegations to the RawMap cache that also assert the
 * map is loaded: rawmap_rawinfo and rawmap_extract_planes.
 */


#include <span>

#include "RawMap.h"
#include "clib/passert.h"
#include "clib/rawtypes.h"
#include "uoclientfiles.h"
#include "ustruct.h"


namespace Pol::Plib
{
signed char UoClientFiles::rawmap_rawinfo( unsigned short x, unsigned short y,
                                           USTRUCT_MAPINFO* gi ) const
{
  passert_always( rawmap_.loaded() );  // caller must rawmapfullread() first
  return rawmap_.rawinfo( x, y, gi );
}

void UoClientFiles::rawmap_extract_planes( std::span<u16> landtile_out, std::span<s8> z_out ) const
{
  passert_always( rawmap_.loaded() );  // caller must rawmapfullread() first
  rawmap_.extract_planes( landtile_out, z_out );
}
}  // namespace Pol::Plib
