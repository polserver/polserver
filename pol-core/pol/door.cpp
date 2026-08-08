/** @file
 *
 * @par History
 */

#include "pol/door.h"

#include <stddef.h>

#include "bscript/executor.h"
#include "clib/rawtypes.h"

#include "pol/base/vector.h"
#include "pol/globals/uvars.h"
#include "pol/item/itemdesc.h"
#include "pol/item/location.h"
#include "pol/network/client.h"
#include "pol/syshookscript.h"
#include "pol/ufunc.h"


namespace Pol::Core
{
UDoor::UDoor( const Items::DoorDesc& descriptor ) : ULockable( descriptor, UOBJ_CLASS::CLASS_ITEM )
{
}

void UDoor::builtin_on_use( Network::Client* client )
{
  if ( locked() )
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
  const Items::DoorDesc* dd = static_cast<const Items::DoorDesc*>( &itemdesc() );

  Pos4d newpos = pos();

  set_dirty();
  if ( is_open() )
  {
    if ( dd->graphic )
      graphic = dd->graphic;
    else
      graphic = static_cast<u16>( objtype_ );
    newpos = newpos - dd->mod;
  }
  else
  {
    graphic = dd->open_graphic;
    newpos = newpos + dd->mod;
  }

  // A door on a boat is one of its components, and a boat refuses to take a component aboard as a
  // traveller, so the multi lookups place_at() does here always come to nothing. It goes through
  // place_at anyway so that the rule has no exceptions to remember: a boat moves its own cargo
  // directly, because the multi cannot have changed, and everything else says where it is going.
  //
  // Nothing to check: a door that is being toggled is in the world, and the only thing place_at
  // refuses is a destination without a realm, which this one takes from the door itself.
  (void)Items::place_at( *this, newpos );

  send_item_to_inrange( this );
}

bool UDoor::is_open() const
{
  const Items::DoorDesc* dd = static_cast<const Items::DoorDesc*>( &itemdesc() );
  if ( graphic == dd->open_graphic )
    return true;
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

bool UDoor::get_method_hook( const char* methodname, Bscript::Executor* ex, ExportScript** hook,
                             unsigned int* PC ) const
{
  if ( gamestate.system_hooks.get_method_hook( gamestate.system_hooks.door_method_script.get(),
                                               methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}
}  // namespace Pol::Core
