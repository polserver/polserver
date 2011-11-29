/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include "../clib/endian.h"
#include "../clib/passert.h"

#include "network/client.h"
#include "door.h"
#include "item/itemdesc.h"
#include "objtype.h"
#include "ufunc.h"
#include "uworld.h"

UDoor::UDoor( const DoorDesc& descriptor ) :
	ULockable( descriptor, CLASS_ITEM )
{
}

void UDoor::builtin_on_use( Client *client )
{
    if (locked_)
    {
        private_say_above( client->chr, this, "That is locked." );
    }
    else
    {
        toggle();
    }
}

void UDoor::toggle()
{
    const DoorDesc& dd = fast_find_doordesc( objtype_ );

    unsigned short oldx = x, oldy = y;

    set_dirty();
    if (is_open())
    {
        if (dd.graphic)
			graphic = dd.graphic;
		else
			graphic = objtype_;
		x -= dd.xmod;
		y -= dd.ymod;
    }
    else
    {
        graphic = dd.open_graphic;
		x += dd.xmod;
		y += dd.ymod;
    }
    
    MoveItemWorldPosition( oldx, oldy, this, NULL );
	
	send_item_to_inrange( this );
}

bool UDoor::is_open() const
{
	const DoorDesc& dd = fast_find_doordesc( objtype_ );

    if (graphic == dd.open_graphic)
        return true;
    else
        return false;
}

void UDoor::open()
{
    if (is_open())
        toggle();
}

void UDoor::close()
{
    if (!is_open())
	    toggle();
}
