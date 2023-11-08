/** @file
 *
 * @par History
 * - 2005/07/05 Shinigami: added Warning if no tile was loaded
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include "tiles.h"

#include <stddef.h>

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/logfacility.h"
#include "clib/passert.h"
#include "clib/strutil.h"

#include "mapfunc.h"
#include "pkg.h"
#include "pol/objtype.h"
#include "systemstate.h"

namespace Pol
{
namespace Plib
{
void load_tile_entry( const Package* /*pkg*/, Clib::ConfigElem& elem )
{
  unsigned short graphic = static_cast<unsigned short>( strtoul( elem.rest(), nullptr, 0 ) );
  passert_always( graphic < ( systemstate.tile.size() ) );
  Tile& entry = systemstate.tile.at(graphic);
  entry.desc = elem.remove_string( "Desc" );
  entry.uoflags = elem.remove_ulong( "UoFlags" );
  entry.layer = static_cast<u8>( elem.remove_ushort( "Layer", 0 ) );
  entry.height = static_cast<u8>( elem.remove_ushort( "Height" ) );
  entry.weight = static_cast<u8>( elem.remove_ushort( "Weight" ) );
  entry.flags = readflags( elem );

  if ( graphic > systemstate.max_graphic )
    systemstate.max_graphic = graphic;
}


u16 get_max_tile()
{
  if ( systemstate.max_graphic > 0x7FFF )
    return 0xFFFF;
  else if ( systemstate.max_graphic > 0x3FFF )
    return 0x7FFF;
  else
    return 0x3FFF;
}

void load_tiles_cfg()
{
  load_all_cfgs( "tiles.cfg", "TILE", load_tile_entry );

  if ( systemstate.max_graphic == 0 )
    ERROR_PRINT << "Warning: No tiles loaded. Please check tiles.cfg\n";
  else
  {
    systemstate.config.max_tile_id = get_max_tile();
    systemstate.tile.resize( systemstate.config.max_tile_id + 1 );
    INFO_PRINT << "Maximum defined graphic: " << Clib::hexint( systemstate.max_graphic ) << ", "
               << "maximum tile id: " << Clib::hexint( systemstate.config.max_tile_id ) << "\n";
  }
}
}  // namespace Plib
}  // namespace Pol
