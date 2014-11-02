/*
History
=======


Notes
=======

*/
#include "door.h"

#include "network/client.h"
#include "item/itemdesc.h"
#include "objtype.h"
#include "ufunc.h"
#include "uworld.h"

#include "../clib/passert.h"


namespace Pol {
  namespace Core {
	UDoor::UDoor( const Items::DoorDesc& descriptor ) :
	  ULockable( descriptor, CLASS_ITEM )
	{}

	void UDoor::builtin_on_use( Network::Client *client )
	{
	  if ( locked_ )
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
      const Items::DoorDesc* dd = static_cast<const Items::DoorDesc*>( &itemdesc( ) );

	  unsigned short oldx = x, oldy = y;

	  set_dirty();
	  if ( is_open() )
	  {
		if ( dd->graphic )
		  graphic = dd->graphic;
		else
		  graphic = static_cast<u16>( objtype_ );
		x -= dd->xmod;
		y -= dd->ymod;
	  }
	  else
	  {
		graphic = dd->open_graphic;
		x += dd->xmod;
		y += dd->ymod;
	  }

	  MoveItemWorldPosition( oldx, oldy, this, NULL );

	  send_item_to_inrange( this );
	}

	bool UDoor::is_open() const
	{
      const Items::DoorDesc* dd = static_cast<const Items::DoorDesc*>( &itemdesc( ) );
	  if ( graphic == dd->open_graphic )
		return true;
	  else
		return false;
	}

	void UDoor::open()
	{
	  if ( !is_open() )
		toggle();
	}

	void UDoor::close()
	{
	  if ( is_open() )
		toggle();
	}

    size_t UDoor::estimatedSize() const
    {
      return base::estimatedSize();
    }
  }
}