/** @file
 *
 * @par History
 */


#include "lockable.h"

#include "../clib/cfgelem.h"
#include "../clib/streamsaver.h"

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
    sw() << "\tLocked\t" << locked() << pf_endl;
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
}
}
