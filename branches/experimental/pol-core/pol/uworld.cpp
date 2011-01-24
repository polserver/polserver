/*
History
=======
2005/01/23 Shinigami: check_item_integrity & check_character_integrity - fix for multi realm support (had used WGRID_X & WGRID_Y)
                      ClrCharacterWorldPosition - Tokuno MapDimension doesn't fit blocks of 64x64 (WGRID_SIZE)
2009/09/03 MuadDib:   Relocation of multi related cpp/h

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/logfile.h"
#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"

#include "../plib/realm.h"
#include "mobile/charactr.h"
#include "item/item.h"
#include "multi/multi.h"
#include "realms.h"
#include "uvars.h"
#include "uworld.h"

void add_item_to_world( Item* item )
{
	Zone& zone = getzone( item->x, item->y, item->realm );

    passert( find_in( zone.items, item) == zone.items.end() );

    ++item->realm->toplevel_item_count;
    zone.items.push_back( item );
}

void remove_item_from_world( Item* item )
{
    Zone& zone = getzone( item->x, item->y, item->realm );

    ZoneItems::iterator itr = find_in( zone.items, item );
    if (itr == zone.items.end())
    {
        Log2( "remove_item_from_world: item 0x%lx at %d,%d does not exist in world zone\n",
                    item->serial, item->x, item->y );

        passert( itr != zone.items.end() );
    }

    --item->realm->toplevel_item_count;
    zone.items.erase( itr );
}

void add_multi_to_world( UMulti* multi )
{
    Zone& zone = getzone( multi->x, multi->y, multi->realm );
    zone.multis.push_back( multi );
}

void remove_multi_from_world( UMulti* multi )
{
    Zone& zone = getzone( multi->x, multi->y, multi->realm );
    ZoneMultis::iterator itr = find_in( zone.multis, multi );
    
    passert(itr != zone.multis.end());

    zone.multis.erase( itr );
}

void move_multi_in_world( unsigned short oldx, unsigned short oldy,
                          unsigned short newx, unsigned short newy,
                          UMulti* multi, Realm* oldrealm )
{
    Zone& oldzone = getzone( oldx, oldy, oldrealm );
    Zone& newzone = getzone( newx, newy, multi->realm );

    if (&oldzone != &newzone)
    {
        ZoneMultis::iterator itr = find_in( oldzone.multis, multi );
        passert( itr != oldzone.multis.end() );
        oldzone.multis.erase( itr );

        newzone.multis.push_back( multi );
    }
}

int get_toplevel_item_count()
{
	std::vector<Realm*>::const_iterator itr;

	int count = 0;
	for(itr = Realms->begin(); itr != Realms->end(); ++itr)
		count += (*itr)->toplevel_item_count;
	return count;
}

int get_mobile_count()
{
	std::vector<Realm*>::const_iterator itr;

	int count = 0;
	for(itr = Realms->begin(); itr != Realms->end(); ++itr)
		count += (*itr)->mobile_count;
	return count;
}


World::World() :
    toplevel_item_count(0),
    mobile_count(0)
{
}

//4-17-04 Rac destroyed the world! in favor of splitting its duties amongst the realms
//World world;

void SetCharacterWorldPosition( Character* chr )
{
    Zone& zone = getzone( chr->x, chr->y, chr->realm );

    passert( !zone.characters.count( chr ) );

    ++chr->realm->mobile_count;
    zone.characters.insert( chr );
}

void ClrCharacterWorldPosition( Character* chr, const char* reason )
{
    Zone& zone = getzone( chr->x, chr->y, chr->realm );
    unsigned wgridx = chr->realm->width() / WGRID_SIZE;
    unsigned wgridy = chr->realm->height() / WGRID_SIZE;
    
    // Tokuno-Fix
    if (wgridx * WGRID_SIZE < chr->realm->width())
      wgridx++;
    if (wgridy * WGRID_SIZE < chr->realm->height())
      wgridy++;
    
    if (!zone.characters.count( chr ) )
    {
        Log( "ClrCharacterWorldPosition(%s): mob (0x%lx,0x%lx) supposedly at (%d,%d) isn't in correct zone\n",
              reason, chr->serial, chr->serial_ext, chr->x, chr->y );
        for( unsigned zonex = 0; zonex < wgridx; ++zonex )
        {
            for( unsigned zoney = 0; zoney < wgridy; ++zoney )
            {
                if (zone.characters.count( chr ))
                    Log( "ClrCharacterWorldPosition: Found mob in zone (%d,%d)\n",
                                zonex, zoney );
            }
        }
        passert( zone.characters.count( chr ) );
    }

    --chr->realm->mobile_count;
    zone.characters.erase( chr );
}

void MoveCharacterWorldPosition( unsigned short oldx, unsigned short oldy,
                                 unsigned short newx, unsigned short newy,
                                 Character* chr, Realm* oldrealm )
{
	if(oldrealm == NULL)
		oldrealm = chr->realm;
    Zone& oldzone = getzone( oldx, oldy, oldrealm );
    Zone& newzone = getzone( newx, newy, chr->realm);

    if (&oldzone != &newzone)
    {
        passert( oldzone.characters.count( chr ) );
        passert( !newzone.characters.count( chr ) );

        oldzone.characters.erase( chr );
		
        newzone.characters.insert( chr );
		if(chr->realm != oldrealm)
		{
			--oldrealm->mobile_count;
			++chr->realm->mobile_count;
		}
    }
}

void MoveItemWorldPosition( unsigned short oldx, unsigned short oldy,
                            Item* item, Realm* oldrealm )
{
	if(oldrealm == NULL)
		oldrealm = item->realm;
    Zone& oldzone = getzone( oldx, oldy, oldrealm );
    Zone& newzone = getzone( item->x, item->y, item->realm );

    if (&oldzone != &newzone)
    {
        ZoneItems::iterator itr = find_in( oldzone.items, item );

        if (itr == oldzone.items.end())
        {
            Log2( "MoveItemWorldPosition: item 0x%lx at old-x/y(%d,%d - %s) new-x/y(%d,%d - %s) does not exist in world zone. \n",
                        item->serial, oldx, oldy, oldrealm->name().c_str(), item->x, item->y, item->realm->name().c_str() );

            passert( itr != oldzone.items.end() );
        }

        oldzone.items.erase( itr );
        
        passert( find_in( newzone.items, item ) == newzone.items.end() );
        newzone.items.push_back( item );
    }
}

// Dave added this for debugging a single zone

bool check_single_zone_item_integrity(int x, int y, Realm* realm)
{
    try
    {
        ZoneItems& witem = realm->zone[x][y].items;

        for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
        {
            Item* item = *itr;
            unsigned short wx, wy;
            zone_convert( item->x, item->y, wx, wy, realm );
            if (wx != x || wy != y)
            {
                Log( "Item 0x%lx in zone (%d,%d) but location is (%d,%d) (zone %d,%d)\n",
                         item->serial,
                         x, y,
                         item->x, item->y,
                         wx, wy );
                return false;
            }
        }
    }
    catch( ... )
    {
        Log( "item integ problem at zone (%d,%d)\n", x, y );
        return false;
    }
	return true;
}


bool check_item_integrity()
{
    bool ok = true;
	std::vector<Realm*>::iterator itr;
	for(itr = Realms->begin(); itr != Realms->end(); ++itr)
	{
		Realm* realm = *itr;
    
    unsigned int gridwidth = realm->width() / WGRID_SIZE;
    unsigned int gridheight = realm->height() / WGRID_SIZE;
    
    // Tokuno-Fix
    if (gridwidth * WGRID_SIZE < realm->width())
      gridwidth++;
    if (gridheight * WGRID_SIZE < realm->height())
      gridheight++;

	    for( unsigned x = 0; x < gridwidth; ++x )
		{
			for( unsigned y = 0; y < gridheight; ++y )
			{	
				if (!check_single_zone_item_integrity(x,y,realm))
				    ok = false;
			}	
		}
	}
	return ok;
}

void check_character_integrity()
{
    // TODO: iterate through the object hash?
    //for( unsigned i = 0; i < characters.size(); ++i )
    //{
    //    Character* chr = characters[i];
    //    unsigned short wx, wy;
    //    w_convert( chr->x, chr->y, wx, wy );
    //    if (!world.zone[wx][wy].characters.count(chr))
    //    {
    //        cout << "Character " << chr->serial << " at " << chr->x << "," << chr->y << " is not in its zone." << endl;
    //    }
    //}
	std::vector<Realm*>::iterator itr;
	for(itr = Realms->begin(); itr != Realms->end(); ++itr)
	{
		Realm* realm = *itr;

    unsigned int gridwidth = realm->width() / WGRID_SIZE;
    unsigned int gridheight = realm->height() / WGRID_SIZE;
    
    // Tokuno-Fix
    if (gridwidth * WGRID_SIZE < realm->width())
      gridwidth++;
    if (gridheight * WGRID_SIZE < realm->height())
      gridheight++;
    
    for( unsigned x = 0; x < gridwidth; ++x )
		{
			for( unsigned y = 0; y < gridheight; ++y )
			{
				ZoneCharacters& wchr = realm->zone[x][y].characters;

				for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
				{
					Character* chr = *itr;
					unsigned short wx, wy;
					zone_convert( chr->x, chr->y, wx, wy, chr->realm );
					if (wx != x || wy != y)
						cout << "Character " << chr->serial << " in a zone, but elsewhere" << endl;
				}
			}
		}
	}
}
