/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/cfgelem.h"

#include "lockable.h"

ULockable::ULockable( const ItemDesc& itemdesc, UOBJ_CLASS uobj_class ) :
    Item(itemdesc, uobj_class),
    locked_(false)
{
}

void ULockable::readProperties( ConfigElem& elem )
{
    base::readProperties( elem );
    locked_ = elem.remove_bool( "Locked", false );
}

void ULockable::printProperties( StreamWriter& sw ) const
{
    base::printProperties(sw);
    
    if (locked_)
        sw() << "\tLocked\t" << locked_ << pf_endl;
}

//dave 12-20
Item* ULockable::clone() const
{
    ULockable* item = static_cast<ULockable*>(base::clone());

	item->locked_ = locked_;
    return item;
}
