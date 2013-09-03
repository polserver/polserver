/*
History
=======
2005/07/05 Shinigami: added Warning if no landtile was loaded

Notes
=======

*/


#include "../clib/stl_inc.h"

#include "../clib/cfgelem.h"
#include "../clib/passert.h"

#include "../plib/mapfunc.h"
#include "../plib/pkg.h"

const unsigned LANDTILE_COUNT = 0x4000;
struct LandTile
{
    unsigned int uoflags;
    unsigned int flags;
};
LandTile landtiles[ LANDTILE_COUNT ];
bool landtiles_loaded = false;

void load_landtile_entry( const Package* pkg, ConfigElem& elem )
{
    unsigned short graphic = static_cast<unsigned short>(strtoul( elem.rest(), NULL, 0 ));
    passert_always( graphic < LANDTILE_COUNT );

    landtiles[ graphic ].uoflags = elem.remove_ulong( "UoFlags" );
    landtiles[ graphic ].flags = readflags( elem );
    
    landtiles_loaded = true;
}

void load_landtile_cfg()
{
    load_all_cfgs( "landtiles.cfg", "landtile", load_landtile_entry );

    if (!landtiles_loaded)
        cerr << "Warning: No landtiles loaded. Please check landtiles.cfg" << endl;
}

unsigned int landtile_uoflags( unsigned short landtile )
{
    passert_always( landtile < LANDTILE_COUNT );
    return landtiles[ landtile ].uoflags;
}

unsigned int landtile_flags( unsigned short landtile )
{
    passert_always( landtile < LANDTILE_COUNT );
    return landtiles[ landtile ].flags;
}

