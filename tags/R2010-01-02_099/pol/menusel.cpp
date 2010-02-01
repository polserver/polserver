/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"
#include <assert.h>

#include "../clib/clib.h"
#include "../clib/endian.h"
#include "../clib/logfile.h"
#include "../clib/passert.h"

#include "accounts/account.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "msghandl.h"
#include "pktin.h"
#include "menu.h"
#include "ufunc.h"

void handle_menu_selection( Client *client, PKTIN_7D *msg )
{
    if (client->chr->menu == NULL)
    {
        Log( "%s/%s tried to use a menu, but none was active.\n",
                client->acct->name(),
                client->chr->name().c_str() );
        return;
    }

	u16 menu_id = cfBEu16( msg->menu_id );
	if (client->chr->menu->menu_id != menu_id)
	{
		printf( "Client tried to use a menu he wasn't entitled to\n" );
		// LOGME illegal menu selection
		client->chr->cancel_menu();
		return;
	}

	Menu* menu = client->chr->menu;
    
	client->chr->menu = NULL;

	if (msg->choice == 0)	// client cancelled menu
	{
        client->chr->cancel_menu();
		return;
	}

	u16 choice = cfBEu16( msg->choice );
	if (choice == 0 || choice > menu->menuitems_.size())
	{
		printf( "Client menu choice out of range\n" );
        client->chr->cancel_menu();
		return;
	}
	// Wow, client chose something valid from the menu, even.
	// Note, the incoming message specified object type, color, and used_item_serial.
	// Since we'd just have to verify these anyway, I'm just going
	// to ignore them, and use the information in the MenuItem.
	// If this turns out not to be workable, we'll have to validate those 
	// input fields, too.

	MenuItem* mi = &menu->menuitems_[ choice - 1 ];
	if (mi->submenu_id)
	{
		client->chr->menu = find_menu( mi->submenu_id );
		send_menu( client,  client->chr->menu);
	}
	else
	{
		
		passert(client->chr->on_menu_selection);
		
		client->chr->on_menu_selection( client, mi, msg );
	}
}

MESSAGE_HANDLER( PKTIN_7D, handle_menu_selection );
