/** @file
 *
 * @par History
 */


#include "lockable.h"

#include "../clib/cfgelem.h"
#include "../clib/streamsaver.h"
#include "globals/uvars.h"
#include "uobject.h"

namespace Pol
{
namespace Core
{
ULockable::ULockable( const Items::ItemDesc& itemdesc, UOBJ_CLASS uobj_class )
    : Item( itemdesc, uobj_class )
{
}

void ULockable::readProperties( Clib::ConfigElem& elem )
{
  base::readProperties( elem );
  locked( elem.remove_bool( "Locked", false ) );
}

void ULockable::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );

  if ( locked() )
    sw.add( "Locked", locked() );
}

// dave 12-20
Items::Item* ULockable::clone() const
{
  ULockable* item = static_cast<ULockable*>( base::clone() );

  item->locked( locked() );
  return item;
}

size_t ULockable::estimatedSize() const
{
  return base::estimatedSize();
}

bool ULockable::get_method_hook( const char* methodname, Bscript::Executor* ex,
                                 Core::ExportScript** hook, unsigned int* PC ) const
{
  if ( Core::gamestate.system_hooks.get_method_hook(
           Core::gamestate.system_hooks.lockable_method_script.get(), methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}
}
}
