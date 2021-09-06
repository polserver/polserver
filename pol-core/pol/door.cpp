/** @file
 *
 * @par History
 */

#include "door.h"

#include <stddef.h>

#include "bscript/executor.h"
#include "clib/rawtypes.h"

#include "base/vector.h"
#include "globals/uvars.h"
#include "item/itemdesc.h"
#include "network/client.h"
#include "syshookscript.h"
#include "ufunc.h"
#include "uworld.h"


namespace Pol
{
namespace Core
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

  unsigned short oldx = x(), oldy = y();

  set_dirty();
  if ( is_open() )
  {
    if ( dd->graphic )
      graphic = dd->graphic;
    else
      graphic = static_cast<u16>( objtype_ );
    setposition( pos() - Vec2d( dd->xmod, dd->ymod ) );
  }
  else
  {
    graphic = dd->open_graphic;
    setposition( pos() + Vec2d( dd->xmod, dd->ymod ) );
  }

  MoveItemWorldPosition( oldx, oldy, this, nullptr );

  send_item_to_inrange( this );
}

bool UDoor::is_open() const
{
  const Items::DoorDesc* dd = static_cast<const Items::DoorDesc*>( &itemdesc() );
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

bool UDoor::get_method_hook( const char* methodname, Bscript::Executor* ex, ExportScript** hook,
                             unsigned int* PC ) const
{
  if ( gamestate.system_hooks.get_method_hook( gamestate.system_hooks.door_method_script.get(),
                                               methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}
}  // namespace Core
}  // namespace Pol
