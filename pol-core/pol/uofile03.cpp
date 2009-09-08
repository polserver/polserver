/*
History
=======
2009/09/03 MuadDib:   Relocation of multi related cpp/h


Notes
=======

*/

#include "clib/stl_inc.h"

#include <stdio.h>
#include <string.h>

#include "clib/strutil.h"

#include "charactr.h"
#include "core.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "polcfg.h"
#include "udatfile.h"
#include "ustruct.h"
#include "uvars.h"
#include "multi/multi.h"
#include "uofile.h"
#include "uofilei.h"
#include "uworld.h"
#include "zone.h"

//#define DEBUG_BOATS


void readdynamics( StaticList& vec, unsigned short x, unsigned short y, Realm* realm, Items& ivec, bool doors_block )
{
    unsigned short wx, wy;
    w_convert2( x, y, wx, wy );
    ZoneItems& witems = realm->zone[wx][wy].items;
    for (ZoneItems::const_iterator itr = witems.begin(); itr != witems.end(); ++itr )
	{
		Item *item = *itr;
		if ((item->x == x) && (item->y == y))
        {
            if (tile_flags(item->graphic) & USTRUCT_TILE::FLAG_WALKBLOCK)
		    {
                if (doors_block || (~tile_flags( item->graphic ) & USTRUCT_TILE::FLAG_DOOR))
                    vec.push_back( StaticRec( item->graphic, item->z ) );
            }
 
                // tile_flags(item->graphic) & USTRUCT_TILE::FLAG__UNKNOWN_01000000)
            if (has_walkon_script( item->objtype_ ))
            {
                ivec.push_back( item );
            }
		}
	}
}

void readdynamics( const Character* chr, 
                   StaticList& vec, 
                   unsigned short x, unsigned short y, 
                   Items& ivec, bool doors_block )
{
    unsigned short wx, wy;
    w_convert2( x, y, wx, wy );
    ZoneItems& witems = chr->realm->zone[wx][wy].items;
    for (ZoneItems::const_iterator itr = witems.begin(); itr != witems.end(); ++itr )
	{
		Item *item = *itr;
		if ((item->x == x) && (item->y == y))
        {
            // cout << "dynamic found: " << hexint( item->objtype_ ) << " " << item->x << "," << item->y << endl;

            if (tile_flags(item->graphic) & USTRUCT_TILE::FLAG_WALKBLOCK)
		    {
                if (!chr->can_move( item ) &&
                    (doors_block || (~tile_flags( item->graphic ) & USTRUCT_TILE::FLAG_DOOR)))
                {
                    vec.push_back( StaticRec( item->graphic, item->z ) );
                }
            }
 
                // tile_flags(item->graphic) & USTRUCT_TILE::FLAG__UNKNOWN_01000000)
            if (has_walkon_script( item->objtype_ ))
            {
                // cout << "dynamic has walkon script" << endl;
                ivec.push_back( item );
            }
		}
	}
}

typedef std::vector<UMulti*> MultiList;
void readmultis( StaticList& vec, unsigned short x, unsigned short y, Realm* realm, MultiList& mvec )
{
    unsigned short wxL, wyL, wxH, wyH;
    w_convert2( x-64, y-64, wxL, wyL );
    w_convert2( x+64, y+64, wxH, wyH );

    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneMultis& wmultis = realm->zone[wx][wy].multis;

            for( ZoneMultis::iterator itr = wmultis.begin(), end = wmultis.end(); itr != end; ++itr )
            {
                UMulti* multi = *itr;
                const MultiDef& def = multi->multidef();
                if (def.readobjects( vec, int(x) - multi->x, int(y) - multi->y, multi->z ))
                {
                    mvec.push_back( multi );
                }
            }
        }
    }


/*
    for( Multis::const_iterator itr = multis.begin(); itr != multis.end(); ++itr )
    {
        UMulti* multi = *itr;
        const MultiDef& def = multi->multidef();
        if (def.readobjects( vec, int(x) - multi->x, int(y) - multi->y, multi->z ))
        {
            mvec.push_back( multi );
        }
    }
*/
}
void readmultis( StaticList& vec, unsigned short x, unsigned short y, Realm* realm )
{
    unsigned short wxL, wyL, wxH, wyH;
    w_convert2( x-64, y-64, wxL, wyL );
    w_convert2( x+64, y+64, wxH, wyH );

    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneMultis& wmultis = realm->zone[wx][wy].multis;

            for( ZoneMultis::iterator itr = wmultis.begin(), end = wmultis.end(); itr != end; ++itr )
            {
                UMulti* multi = *itr;
                const MultiDef& def = multi->multidef();
                def.readobjects( vec, int(x) - multi->x, int(y) - multi->y, multi->z );
            }
        }
    }
}

/* The supporting multi is the highest multi that is below or equal
 * to the Z-coord of the supported object.
 */
UMulti* uo_find_supporting_multi( MultiList& mvec, int z )
{
    UMulti* found = NULL;
    for( MultiList::const_iterator itr = mvec.begin(), end = mvec.end(); itr != end; ++itr )
    {
        UMulti* multi = (*itr);
        if (multi->z <= z)
        {
            if ((found == NULL) ||
                (multi->z > found->z))
            {
                found = multi;
            }
        }
    }
    return found;
}


// new Z given new X, Y, and old Z.
bool uo_walkheight(int x, int y, int oldz, 
                int* newz, 
                UMulti** pmulti, Item** pwalkon,
                bool doors_block,
                MOVEMODE movemode) 
{
    static StaticList vec;
    static MultiList mvec;
    static Items ivec;
    
    if (!VALID_WORLD_LOC(x,y, find_realm(string("britannia")))
    {
        return false;
    }

    vec.clear();
    mvec.clear();
    ivec.clear();
    
    readdynamics( vec, x, y, ivec, true );
    readmultis( vec, x, y, mvec );
    readstatics( vec, x, y );

    bool result;
    standheight(movemode, vec, x, y, oldz, 
                &result, newz);
    if (result)
    {
        if (!mvec.empty())
        {
            *pmulti = uo_find_supporting_multi( mvec, *newz );
        }
        else
        {
            *pmulti = NULL;
        }
        if (!ivec.empty())
        {
            *pwalkon = find_walkon_item( ivec, *newz );
        }
        else
        {
            *pwalkon = NULL;
        }
        
    }
    return result;
}

// new Z given new X, Y, and old Z.
bool uo_walkheight(const Character* chr,
                int x, int y, int oldz, 
                int* newz, 
                UMulti** pmulti, Item** pwalkon) 
{
    static StaticList vec;
    static MultiList mvec;
    static Items ivec;
    
    if (!VALID_WORLD_LOC(x,y, chr->realm))
    {
        return false;
    }


    vec.clear();
    mvec.clear();
    ivec.clear();
    
    readdynamics( chr, vec, x, y, ivec, chr->doors_block() );
    readmultis( vec, x, y, mvec );
    readstatics( vec, x, y );

    bool result;
    standheight(chr->movemode, vec, x, y, oldz, &result, newz);
    if (result)
    {
        if (!mvec.empty())
        {
            *pmulti = uo_find_supporting_multi( mvec, *newz );
        }
        else
        {
            *pmulti = NULL;
        }
        if (!ivec.empty())
        {
            //cout << "walkon vec isn't empty" << endl;
            *pwalkon = find_walkon_item( ivec, *newz );
            //if (*pwalkon)
            //    cout << "found a walkon" << endl;
        }
        else
        {
            *pwalkon = NULL;
        }
        
    }
    //cout << "walkheight result: " << result << endl;
    return result;
}

// new Z given new X, Y, and old Z.
bool uo_dropheight(int x, int y, int oldz, 
                int* newz, 
                UMulti** pmulti) 
{
    static StaticList vec;
    static MultiList mvec;
    static Items ivec;
    
    if (!VALID_WORLD_LOC(x,y, find_realm(string("britannia")))
    {
        return false;
    }

    vec.clear();
    mvec.clear();
    ivec.clear();
    
    readdynamics( vec, x, y, ivec, true );
    readmultis( vec, x, y, mvec );
    readstatics( vec, x, y );

    bool result = groundheight(x,y, newz);

    statics_dropheight(vec, x, y, oldz, &result, newz);
    if (result)
    {
        if (!mvec.empty())
        {
            *pmulti = uo_find_supporting_multi( mvec, *newz );
        }
        else
        {
            *pmulti = NULL;
        }
        
    }
    return result;
}


bool uo_lowest_standheight( int x, int y, int* z )
{
    static StaticList vec;
    vec.clear();
    readstatics( vec, x, y );
    USTRUCT_MAPINFO mi;
    getmapinfo( x, y, z, &mi );
    bool res = true;
    standheight( MOVEMODE_LAND, vec, x, y, *z, &res, z );
    return res;
}

UMulti* uo_find_supporting_multi( unsigned short x, unsigned short y, int z )
{
    if (!VALID_WORLD_LOC( x,y, find_realm(string("britannia") ))
    {
        return NULL;
    }

    static StaticList vec;
    static MultiList mvec;
    vec.clear();
    mvec.clear();
    readmultis( vec, x, y, mvec );

    if (!mvec.empty())
    {
        return uo_find_supporting_multi( mvec, z );
    }
    else
    {
        return NULL;
    }
}

bool uo_passable( unsigned short ax, unsigned short ay )
{
    /*
        * Assume if the map says water's here, there won't be statics on top.
        *   Note, it _may_ be possible, if the map says there's water at the center,
        *   to assume there's water for the entire hull.
        *   Also note this early continue precludes dynamics blocking...
        */
        USTRUCT_MAPINFO mi;
        rawmapinfo( ax, ay, &mi );
        if (mi.landtile >= USTRUCT_MAPINFO::WATER__LOWEST && mi.landtile <= USTRUCT_MAPINFO::WATER__HIGHEST)
            return true;
        // possible early 'return false' if level

    
    /*
        * Last chance: does statics0.mul have a water tile here, and no other blocking statics?
        */
        static StaticList vec;
        vec.clear();
        readstatics( vec, ax, ay ); // FIXME is WALKBLOCK right here?
        // readnonboatdynamics
        bool onwater = false;
        for( StaticList::const_iterator itr = vec.begin(), end=vec.end(); itr != end; ++itr )
        {
            const StaticRec& srec = *itr;
                    // if (tile_flags( srec.graphic ) & USTRUCT_TILE::FLAG_LIQUID)

            if (srec.graphic >= 0x1796 && srec.graphic <= 0x17B2) // FIXME hardcoded
            //  if (tile_flags( srec.graphic ) & USTRUCT_TILE::FLAG_PASSABLE_WATER == 
            //           USTRUCT_TILE::FLAG_PASSABLE_WATER)
            {
                onwater = true;
            }
            else
            {
#ifdef DEBUG_BOATS
                    cout << "Location at " << ax << "," << ay << " has a non-water static: " << hexint( srec.graphic ) << endl;
#endif
                return false;
            }
        }
        if (!onwater)
        {
#ifdef DEBUG_BOATS
                cout << "Location at " << ax << "," << ay << " has no water static" << endl;
#endif
            return false;
        }

    /*
     * None of the above fired, so it's passable
     */
    return true;
}

