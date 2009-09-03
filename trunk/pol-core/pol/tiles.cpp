/*
History
=======
2005/07/05 Shinigami: added Warning if no tile was loaded

Notes
=======

*/

#include "clib/stl_inc.h"

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/passert.h"

#include "plib/mapcell.h"
#include "plib/mapfunc.h"
#include "plib/pkg.h"

#include "tiles.h"

Tile tile[ TILE_COUNT ];
bool tiles_loaded = false;

void load_tile_entry( const Package* pkg, ConfigElem& elem )
{
    unsigned short graphic = static_cast<unsigned short>(strtoul( elem.rest(), NULL, 0 ));
    passert_always( graphic < TILE_COUNT );
    Tile& entry = tile[ graphic ];
    entry.desc = elem.remove_string( "Desc" );
    entry.uoflags = elem.remove_ulong( "UoFlags" );
    entry.layer = static_cast<u8>(elem.remove_ushort( "Layer", 0 ));
    entry.height = static_cast<u8>(elem.remove_ushort( "Height" ));
    entry.weight = static_cast<u8>(elem.remove_ushort( "Weight" ));
    entry.flags = readflags( elem );

    tiles_loaded = true;
}

void load_tiles_cfg()
{
    load_all_cfgs( "tiles.cfg", "TILE", load_tile_entry );

    if (!tiles_loaded)
        cerr << "Warning: No tiles loaded. Please check tiles.cfg" << endl;
}
