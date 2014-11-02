/*
History
=======
2005/07/05 Shinigami: added Warning if no landtile was loaded

Notes
=======

*/

#include "../plib/mapfunc.h"
#include "../plib/pkg.h"

#include "../clib/cfgelem.h"
#include "../clib/passert.h"
#include "../clib/logfacility.h"

namespace Pol {
  namespace Core {

	const unsigned LANDTILE_COUNT = 0x4000;
	struct LandTile
	{
	  unsigned int uoflags;
	  unsigned int flags;
	};
	LandTile landtiles[LANDTILE_COUNT];
	bool landtiles_loaded = false;

	void load_landtile_entry( const Plib::Package* /*pkg*/, Clib::ConfigElem& elem )
	{
	  unsigned short graphic = static_cast<unsigned short>( strtoul( elem.rest(), NULL, 0 ) );
	  passert_always( graphic < LANDTILE_COUNT );

	  landtiles[graphic].uoflags = elem.remove_ulong( "UoFlags" );
	  landtiles[graphic].flags = Plib::readflags( elem );

	  landtiles_loaded = true;
	}

	void load_landtile_cfg()
	{
	  load_all_cfgs( "landtiles.cfg", "landtile", load_landtile_entry );

      if ( !landtiles_loaded )
        ERROR_PRINT << "Warning: No landtiles loaded. Please check landtiles.cfg\n";
	}

	unsigned int landtile_uoflags( unsigned short landtile )
	{
	  passert_always( landtile < LANDTILE_COUNT );
	  return landtiles[landtile].uoflags;
	}

	unsigned int landtile_flags( unsigned short landtile )
	{
	  passert_always( landtile < LANDTILE_COUNT );
	  return landtiles[landtile].flags;
	}

  }
}