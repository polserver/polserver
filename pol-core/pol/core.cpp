/*
History
=======
2009/09/03 MuadDib:   Relocation of multi related cpp/h

Notes
=======

*/

#include "clib/stl_inc.h"

#include "plib/realm.h"

#include "mobile/charactr.h"
#include "network/cgdata.h"
#include "network/client.h"
#include "core.h"
#include "item/itemdesc.h"
#include "polsem.h"
#include "module/polsystemmod.h"
#include "multi/multi.h"
#include "realms.h"
#include "ufunc.h"
#include "uworld.h"

void SetSysTrayPopupText( const char* text );
#ifdef _WIN32
static Priority tipPriority = ToolTipPriorityNone;
#endif
void CoreSetSysTrayToolTip( const string& text, Priority priority )
{
#ifdef _WIN32
    if (priority >= tipPriority)
    {
        tipPriority = priority;
        SetSysTrayPopupText( text.c_str() );
    }
#endif
}

bool move_character_to( Character* chr, 
                        unsigned short x,
                        unsigned short y,
                        int z,
                        long flags,
						Realm* oldrealm )
{
       // FIXME consider consolidating with similar code in CHARACTER.CPP
    int newz;
    UMulti* supporting_multi = NULL;
    Item* walkon_item = NULL;
	int new_boost = 0;

    // cout << "mct z: " << z << endl;
    if (flags & MOVEITEM_FORCELOCATION)
    {
        if (x >= chr->realm->width() || y >= chr->realm->height())
        {
            // cout << "mct: 1" << endl;
            return false;
        }

        chr->realm->walkheight( x, y, z, &newz, &supporting_multi, &walkon_item, true, MOVEMODE_LAND, &new_boost );
        newz = z;
    }
    else
    {
        if (!chr->realm->walkheight( chr, x, y, z, &newz, &supporting_multi, &walkon_item, &new_boost ))
        {
            // cout << "mct: 2" << endl;
            return false;
        }
    }
    chr->set_dirty();

	if(( oldrealm != NULL) && (oldrealm != chr->realm) )
	{
		chr->lastx = 0;
		chr->lasty = 0;
		chr->lastz = 0;		
	}
	else
	{
		chr->lastx = chr->x;
		chr->lasty = chr->y;
		chr->lastz = chr->z;
	}
    // probably easier and safer to just check if (chr->logged_in)
	if(chr->isa(UObject::CLASS_NPC) || chr->client) //dave 3/26/3 otherwise exception trying to move offline mobile not found in characters list
	{
		MoveCharacterWorldPosition( chr->x, chr->y, x, y, chr, oldrealm );
	}
    chr->x = x;
    chr->y = y;
    chr->z = static_cast<s8>(newz);

	chr->gradual_boost = new_boost;
    chr->position_changed();
    if (supporting_multi != NULL)
    {
        supporting_multi->register_object( chr );
    }

    // teleport( chr );
    if (chr->has_active_client())    
    {
		if(oldrealm != chr->realm)
		{
			send_new_subserver( chr->client );
			send_owncreate( chr->client, chr );
		}
        send_goxyz( chr->client, chr );
        
        // send_goxyz seems to stop the weather.  This will force a refresh, if the client cooperates.
        chr->client->gd->weather_region = NULL;
    }
	if(chr->isa(UObject::CLASS_NPC) || chr->client) //dave 3/26/3 dont' tell moves of offline PCs
	{
		chr->tellmove();
	}
    if (chr->has_active_client())
	{
        send_objects_newly_inrange( chr->client );
		chr->check_light_region_change();
	}
    if (walkon_item != NULL)
    {
        walkon_item->walk_on( chr );
    }

    chr->lastx = chr->x;
    chr->lasty = chr->y;
    chr->lastz = chr->z;

    return true;
}

/* For us to care, the item must:
   1) be directly under the current position
   2) have a "walk on" script
*/

Item* find_walkon_item( Items& ivec, int z )
{
    //cout << "find walkon item, z=" << z << endl;
    for( Items::const_iterator itr = ivec.begin(), end = ivec.end(); itr != end; ++itr )
    {
        Item* item = (*itr);
        //cout << item->description << " at z= " << int(item->z) << endl;
        if (z == item->z || z == item->z+1) 
        {
            if (has_walkon_script( item->objtype_ ))
            {
                return item;
            }
        }
    }
    //cout << "um, no" << endl;
    return NULL;
}

void atomic_cout( const string& msg )
{
    PolLock lck;
    cout << msg << endl;
}
