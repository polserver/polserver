/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:    Changes for account related source file relocation
 *                          Changes for multi related source file relocation
 */


#include "boatcomp.h"

#include "../baseobject.h"
#include "../lockable.h"


namespace Pol::Multi
{
UPlank::UPlank( const Items::ItemDesc& descriptor )
    : Core::ULockable( descriptor, Core::UOBJ_CLASS::CLASS_ITEM )
{
}

void UPlank::setboat( UBoat* boat )
{
  boat_.set( boat );
}

void UPlank::destroy()
{
  boat_.clear();
  base::destroy();
}

size_t UPlank::estimatedSize() const
{
  return base::estimatedSize() + sizeof( ref_ptr<UBoat> );
}
}  // namespace Pol::Multi
