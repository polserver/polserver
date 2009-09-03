/*
History
=======
2009/08/06 MuadDib:   Added gotten_by code for items.
2009/08/07 MuadDib:   Altered Layer code in equip_item() to use tile_layer prop instead. Cleaner, less vars, less execution.

Notes
=======

*/

#include "clib/stl_inc.h"
#include <assert.h>
#include <stdio.h>

#include "bscript/bobject.h"

#include "clib/endian.h"
#include "clib/logfile.h"

#include "charactr.h"
#include "client.h"
#include "msghandl.h"
#include "pktin.h"
#include "ufunc.h"
#include "uofile.h"
#include "uvars.h"

void equip_item( Client *client, PKTIN_13 *msg )
{
	u32 serial = cfBEu32( msg->serial );
	u8 layer = msg->layer;
	u32 equip_on_serial = cfBEu32( msg->equipped_on );

	if ((layer > HIGHEST_LAYER) || (layer == 0) || client->chr->dead())
	{
		send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP );
		return;
	}
	
	Item *item = client->chr->gotten_item;

	if (item == NULL)
	{
		Log( "Character %08lX tried to equip item %08lx, which did not exist in gotten_items.\n",
			 client->chr->serial, serial );
		send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP ); // 5
		return;
	}

    if (item->serial != serial)
	{
		Log( "Character %08lX tried to equip item %08lx, but had gotten item %08lX\n",
			 client->chr->serial, 
			 serial,
			 item->serial);
		send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP ); // 5
		item->gotten_by = NULL;
		return;
	}

	ItemRef itemref(item);

	item->layer = item->tile_layer;
	client->chr->gotten_item->inuse(false);
	item->is_gotten(false);
	item->gotten_by = NULL;
	client->chr->gotten_item = NULL;

    Character* equip_on = NULL;
    if (equip_on_serial == client->chr->serial)
    {
        equip_on = client->chr;
    }
    else
    {
        equip_on = find_character( equip_on_serial );
        if (equip_on == NULL ||
            !client->chr->can_clothe( equip_on ))
        {
		    send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP );

		    undo_get_item( client->chr, item );
            return;		
        }
    }

	if (equip_on->layer_is_equipped( item->tile_layer ))
	{
		// it appears the client already checks for this, so this code hasn't been exercised.
		// we'll assume client mouse holds on to object
        // 3D Client doesn't check for this!
		send_item_move_failure( client, MOVE_ITEM_FAILURE_ALREADY_WORN );

        undo_get_item( client->chr, item ); //added 11/01/03 for 3d client
		return;
	}

    if (!equip_on->strong_enough_to_equip( item ))
    {
		send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP );
		// the client now puts the item back where it was before.

		// return the item to wherever it was. (?)
		undo_get_item( client->chr, item );
        if (client->chr == equip_on)
        {
            send_sysmessage( client, "You are not strong enough to use that." );
        }
        else
        {
            send_sysmessage( client, "Insufficient strength to equip that." );
        }
        return;		
    }

    if (!equip_on->equippable( item ) ||
        !item->check_equiptest_scripts( equip_on ) ||
        !item->check_equip_script( equip_on, false ))
    {
		send_item_move_failure( client, MOVE_ITEM_FAILURE_ILLEGAL_EQUIP );
		if(item->orphan())
		{
			return;
		}
		undo_get_item( client->chr, item );
		return;		
    }

	if(item->orphan())
	{
		return;
	}
    
	equip_on->equip( item );
	send_wornitem_to_inrange( equip_on, item );
}

MESSAGE_HANDLER( PKTIN_13, equip_item );