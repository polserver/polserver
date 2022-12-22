/** @file
 *
 * @par History
 * - 2005/07/05 Shinigami: added Warning if no tile was loaded
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include "tiles.h"

#include <stddef.h>

#include "../clib/cfgelem.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "mapfunc.h"
#include "pkg.h"
#include "systemstate.h"

namespace Pol
{
namespace Plib
{
void load_tile_entry( const Package* /*pkg*/, Clib::ConfigElem& elem )
{
  unsigned short graphic = static_cast<unsigned short>( strtoul( elem.rest(), nullptr, 0 ) );
  passert_always( graphic < ( systemstate.config.max_tile_id + 1 ) );
  Tile& entry = systemstate.tile[graphic];
  entry.desc = elem.remove_string( "Desc" );
  entry.uoflags = elem.remove_ulong( "UoFlags" );
  entry.layer = static_cast<u8>( elem.remove_ushort( "Layer", 0 ) );
  entry.height = static_cast<u8>( elem.remove_ushort( "Height" ) );
  entry.weight = static_cast<u8>( elem.remove_ushort( "Weight" ) );
  entry.flags = readflags( elem );

  systemstate.tiles_loaded = true;
}

void load_tiles_cfg()
{
  u32 tile_count = systemstate.config.max_tile_id + 1;
  systemstate.tile = new Tile[static_cast<size_t>( tile_count )];
  memset( systemstate.tile, 0, sizeof( Tile ) * tile_count );

  load_all_cfgs( "tiles.cfg", "TILE", load_tile_entry );

  if ( !systemstate.tiles_loaded )
    ERROR_PRINT << "Warning: No tiles loaded. Please check tiles.cfg\n";
}
}  // namespace Plib
}  // namespace Pol
