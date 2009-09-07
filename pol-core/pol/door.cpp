/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include "clib/endian.h"
#include "clib/passert.h"

#include "network/client.h"
#include "door.h"
#include "itemdesc.h"
#include "objtype.h"
#include "ufunc.h"
#include "uworld.h"

struct Offset {
	signed char xmod;
	signed char ymod;
} offsets[ 4 ] = {
    { +1, -1 },     // EXTOBJ_DOOR1, quadrant 1
    { -1, -1 },     // EXTOBJ_DOOR2, quadrant 2
    { -1, +1 },     // EXTOBJ_DOOR3, quadrant 3
    { +1, +1 }      // EXTOBJ_DOOR4, quadrant 4
};

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
    if (graphic & 0x1)
    {
        ++graphic;
		x += static_cast<s16>(dd.xmod);
		y += static_cast<s16>(dd.ymod);
    }
    else
    {
        --graphic;
		x -= static_cast<s16>(dd.xmod);
		y -= static_cast<s16>(dd.ymod);
    }
    
    MoveItemWorldPosition( oldx, oldy, this, NULL );
	
    graphic_ext = ctBEu16( graphic );
	send_item_to_inrange( this );
}

bool UDoor::is_open() const
{
    if (graphic & 0x01)
        return false;
    else
        return true;
}

void UDoor::open()
{
    if (graphic & 0x01)
        toggle();
}

void UDoor::close()
{
    if (~graphic & 0x01)
        toggle();
}
