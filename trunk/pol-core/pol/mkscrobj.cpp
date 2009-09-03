/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"
#include "mkscrobj.h"
#include "uoscrobj.h"

BObjectImp* make_mobileref( Character* chr )
{
    return new ECharacterRefObjImp( chr );
}

BObjectImp* make_itemref( Item* item )
{
    return new EItemRefObjImp( item );
}

BObjectImp* make_boatref( UBoat* boat )
{
    return new EUBoatRefObjImp( boat );
}
