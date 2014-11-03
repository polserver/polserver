/*
History
=======
2009/12/02 Turley:    added config.max_tile_id - Tomi


Notes
=======

*/

// TODO: consider joining this file with landtile.cpp, tiles.cpp and clidata.h and renaming it all to something consistent.
// TODO: encapsulate the tile[] vector here
#include "clidata.h"

#include "tiles.h"
#include "polcfg.h"

#include "../clib/passert.h"
#include "../plib/mapcell.h"

#include <string>

namespace Pol {
  namespace Core {
	unsigned char tilelayer( unsigned short tilenum )
	{
	  if ( tilenum <= config.max_tile_id )
	  {
		return tile[tilenum].layer;
	  }
	  else
	  {
		return 0;
	  }
	}

	char tileheight( unsigned short tilenum )
	{
	  if ( tilenum <= config.max_tile_id )
	  {
		char height = tile[tilenum].height;
		if ( tile[tilenum].flags & Plib::FLAG::GRADUAL )
		  height /= 2;
		return height;
	  }
	  else
	  {
		return 0;
	  }
	}

	u32 tile_flags( unsigned short tilenum )
	{
	  if ( tilenum <= config.max_tile_id )
	  {
		return tile[tilenum].flags;
	  }
	  else
	  {
		return 0;
	  }
	}

	u32 tile_uoflags( unsigned short tilenum )
	{
	  if ( tilenum <= config.max_tile_id )
	  {
		return tile[tilenum].uoflags;
	  }
	  else
	  {
		return 0;
	  }
	}

	std::string tile_desc( unsigned short tilenum )
	{
	  if ( tilenum <= config.max_tile_id )
	  {
		return tile[tilenum].desc;
	  }
	  else // a multi, probably
	  {
		return "multi";
	  }
	}

	unsigned short tileweight( unsigned short tilenum )
	{
	  if ( tilenum <= config.max_tile_id )
	  {
		return tile[tilenum].weight;
	  }
	  else
	  {
		return 0;
	  }
	}
  }
}