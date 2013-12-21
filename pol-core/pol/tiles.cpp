/*
History
=======
2005/07/05 Shinigami: added Warning if no tile was loaded
2009/12/02 Turley:    added config.max_tile_id - Tomi

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/passert.h"

#include "../plib/mapcell.h"
#include "../plib/mapfunc.h"
#include "../plib/pkg.h"

#include "tiles.h"
#include "polcfg.h"
namespace Pol {
  namespace Core {
	bool tiles_loaded = false;
	Tile *tile;

	void load_tile_entry( const Plib::Package* pkg, Clib::ConfigElem& elem )
	{
	  unsigned short graphic = static_cast<unsigned short>( strtoul( elem.rest(), NULL, 0 ) );
	  passert_always( graphic < ( config.max_tile_id + 1 ) );
	  Tile& entry = tile[graphic];
	  entry.desc = elem.remove_string( "Desc" );
	  entry.uoflags = elem.remove_ulong( "UoFlags" );
	  entry.layer = static_cast<u8>( elem.remove_ushort( "Layer", 0 ) );
	  entry.height = static_cast<u8>( elem.remove_ushort( "Height" ) );
	  entry.weight = static_cast<u8>( elem.remove_ushort( "Weight" ) );
	  entry.flags = Plib::readflags( elem );

	  tiles_loaded = true;
	}

	void load_tiles_cfg()
	{
	  tile = new Tile[static_cast<size_t>( config.max_tile_id + 1 )];

	  load_all_cfgs( "tiles.cfg", "TILE", load_tile_entry );

	  if ( !tiles_loaded )
		cerr << "Warning: No tiles loaded. Please check tiles.cfg" << endl;
	}

	void unload_tiles()
	{
	  delete[] tile;
	}
  }
}