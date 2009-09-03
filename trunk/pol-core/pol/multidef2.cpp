/*
History
=======
2006/12/05 Shinigami: removed dummy floor creation in ::readobjects and ::readshapes

Notes
=======

*/

#include "clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include <algorithm>
#include <iostream>

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/passert.h"
#include "clib/stlutil.h"
#include "clib/strutil.h"

#include "plib/mapcell.h"
#include "plib/mapshape.h"

#include "tiles.h"
#include "itemdesc.h"
#include "objtype.h"
#include "udatfile.h"
#include "uofilei.h"
#include "uofile.h"
#include "multidef.h"
#include "ustruct.h"
#include "uofilei.h"
//#include "uobject.h"

// 8/9/03 this seems to be used only by uofile03 -Syz
bool MultiDef::readobjects( StaticList& vec, int x, int y, int zbase ) const
{
    bool result = false;
    if (x >= minrx && x <= maxrx && y >= minry && y <= maxry)
    {
        Components::const_iterator itr, end;
        if (findcomponents( itr, end, x, y ))
        {
            for( ; itr != end; ++itr )
            {
                const MULTI_ELEM* elem = (*itr).second;
                unsigned short graphic = getgraphic( elem->objtype );
                if (tile_flags( graphic ) & FLAG::WALKBLOCK)
                {
                    if (elem->is_static)
                    {
                        vec.push_back( StaticRec( graphic, elem->z+zbase ) );
                        result = true;
                    }
                    // Shinigami: removed. doesn't make sense. non-static
					//            items are normal items an can be removed etc.
                   /* else // put a dummy floor tile there
                    {
                        vec.push_back( StaticRec( 0x495, elem->z+zbase ) );
                        result = true;
                    } */
                }
            }
        }
    }
    return result;
}

bool MultiDef::readshapes( MapShapeList& vec, int x, int y, int zbase, unsigned long anyflags ) const
{
    bool result = false;
    if (x >= minrx && x <= maxrx && y >= minry && y <= maxry)
    {
        Components::const_iterator itr, end;
        if (findcomponents( itr, end, x, y ))
        {
            for( ; itr != end; ++itr )
            {
                const MULTI_ELEM* elem = (*itr).second;
                unsigned short graphic = getgraphic( elem->objtype );
                if (tile_flags( graphic ) & anyflags)
                {
                    if (elem->is_static)
                    {
                        MapShape shape;
                        shape.z = elem->z+zbase;
                        shape.height = tileheight( graphic );
                        shape.flags = tile[ graphic ].flags; // pol_flags_by_tile( graphic );
                        if (!shape.height)
                        {
                            ++shape.height;
                            --shape.z;
                        }
                        vec.push_back( shape );
                        result = true;
                    }
                    // Shinigami: removed. doesn't make sense. non-static
					//            items are normal items an can be removed etc.
					// Turley: BOAT added so hold count as boat item (who.multi) (and walkable)
				    else if (type == BOAT) // put a dummy floor there
                    {
                        MapShape shape;
                        shape.z = elem->z+zbase-1;
                        shape.height = 1;
                        shape.flags = FLAG::MOVELAND|
                                      FLAG::ALLOWDROPON|
                                      FLAG::BLOCKSIGHT|
                                      FLAG::OVERFLIGHT;
                        vec.push_back( shape );
                        result = true;
                    } 
                }
            }
        }
    }
    return result;
}