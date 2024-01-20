/** @file
 *
 * @par History
 * - 2005/07/05 Shinigami: added Warning if no landtile was loaded
 */


#include "landtile.h"

#include <stddef.h>

#include "../clib/cfgelem.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../plib/clidata.h"
#include "../plib/mapfunc.h"
#include "../plib/pkg.h"
#include "globals/uvars.h"

namespace Pol
{
namespace Core
{
void load_landtile_entry( const Plib::Package* /*pkg*/, Clib::ConfigElem& elem )
{
  unsigned short graphic = static_cast<unsigned short>( strtoul( elem.rest(), nullptr, 0 ) );
  passert_always( graphic < Plib::LANDTILE_COUNT );

  gamestate.landtiles[graphic].uoflags = elem.remove_ulong( "UoFlags" );
  gamestate.landtiles[graphic].flags = Plib::readflags( elem );

  gamestate.landtiles_loaded = true;
}

void load_landtile_cfg()
{
  load_all_cfgs( "landtiles.cfg", "landtile", load_landtile_entry );

  if ( !gamestate.landtiles_loaded )
    ERROR_PRINTLN( "Warning: No landtiles loaded. Please check landtiles.cfg" );
}

unsigned int landtile_uoflags( unsigned short landtile )
{
  passert_always( landtile < Plib::LANDTILE_COUNT );
  return gamestate.landtiles[landtile].uoflags;
}

unsigned int landtile_flags( unsigned short landtile )
{
  passert_always( landtile < Plib::LANDTILE_COUNT );
  return gamestate.landtiles[landtile].flags;
}
}  // namespace Core
}  // namespace Pol
