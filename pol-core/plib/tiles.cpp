/** @file
 *
 * @par History
 * - 2005/07/05 Shinigami: added Warning if no tile was loaded
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include "tiles.h"

#include <algorithm>

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

Tile read_tile( Clib::ConfigElem& elem )
{
  Tile entry;
  entry.desc = elem.remove_string( "Desc" );
  entry.uoflags = elem.remove_ulong( "UoFlags" );
  entry.layer = static_cast<u8>( elem.remove_ushort( "Layer", 0 ) );
  entry.height = static_cast<u8>( elem.remove_ushort( "Height" ) );
  entry.weight = static_cast<u8>( elem.remove_ushort( "Weight" ) );
  entry.flags = readflags( elem );

  return entry;
}

void load_tile_entry( const Package* /*pkg*/, Clib::ConfigElem& elem )
{
  unsigned short graphic = static_cast<unsigned short>( strtoul( elem.rest(), nullptr, 0 ) );
  systemstate.tile.at( graphic ) = read_tile( elem );

  if ( graphic > systemstate.max_graphic )
    systemstate.max_graphic = graphic;
};


constexpr u16 expected_max_tile_id( u16 max_graphic )
{
  if ( max_graphic > UOBJ_SA_MAX )
    return UOBJ_HSA_MAX;
  else if ( max_graphic > UOBJ_DEFAULT_MAX )
    return UOBJ_SA_MAX;
  else
    return UOBJ_DEFAULT_MAX;
}

u16 choose_max_tile_id( u16 current, u16 expected )
{
  if ( current == 0 )
    return expected;

  return std::min( current, expected );
}

void load_tiles_cfg()
{
  systemstate.max_graphic = 0;
  systemstate.tile.assign( UOBJ_HSA_MAX + 1, Tile() );

  load_all_cfgs( "tiles.cfg", "TILE", load_tile_entry );

  if ( systemstate.max_graphic == 0 )
  {
    ERROR_PRINT << "Error: No tiles loaded. Please check tiles.cfg\n";
    return;
  }

  auto expected_max = expected_max_tile_id( systemstate.max_graphic );
  auto new_max = choose_max_tile_id( systemstate.config.max_tile_id, expected_max );

  if ( new_max != expected_max )
  {
    INFO_PRINT2(
        "Warning: MaxTileID in pol.cfg does not match graphics in tiles.cfg"
        "\tMax graphic in tiles.cfg: {:#x}\n"
        "\tCurrent MaxTileID: {:#x}\n"
        "\tExpected MaxTileID: {:#x}",
        systemstate.max_graphic, systemstate.config.max_tile_id, expected_max );
  }

  systemstate.config.max_tile_id = new_max;
  systemstate.tile.resize( systemstate.config.max_tile_id + 1 );
}
}  // namespace Plib
}  // namespace Pol
