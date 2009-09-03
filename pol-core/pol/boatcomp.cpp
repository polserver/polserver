/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"
#include "boatcomp.h"

UPlank::UPlank( const ItemDesc& descriptor ) : ULockable( descriptor, CLASS_ITEM )
{
}

void UPlank::setboat( UBoat* boat )
{
    boat_.set(boat);
}

void UPlank::destroy()
{
    boat_.clear();
	base::destroy();
}
