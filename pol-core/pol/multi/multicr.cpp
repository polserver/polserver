/*
History
=======
2009/09/03 MuadDib:	  Changes for account related source file relocation
                      Changes for multi related source file relocation

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include "../../bscript/berror.h"

#include "../../clib/endian.h"
#include "../../clib/strutil.h"

#include "boat.h"
#include "house.h"
#include "../item/itemdesc.h"
#include "../objtype.h"
#include "../ufunc.h"
#include "../uworld.h"

#include "../objecthash.h"

bool isboat( u16 objtype )
{
    return find_itemdesc( objtype ).type == ItemDesc::BOATDESC;
}

bool ishouse( u16 objtype )
{
    return find_itemdesc( objtype ).type == ItemDesc::HOUSEDESC;
}

// scripted_create duplicates some of this work
//Dave changed 3/8/3 to use objecthash
UMulti* UMulti::create( const ItemDesc& descriptor, u32 serial )
{
    UMulti* multi = NULL;

    if ( descriptor.type == ItemDesc::BOATDESC )
    {
        multi = new UBoat( descriptor );
    }
    else if ( descriptor.type == ItemDesc::HOUSEDESC )
    {
        multi = new UHouse( descriptor );
    }
    else
    {
        cerr << "Tried to create multi " << hexint(descriptor.objtype) << " but no definition exists in itemdesc.cfg" << endl;
        return NULL;
    }

    if (serial)
        multi->serial = UseItemSerialNumber( serial );
    else
        multi->serial = GetNewItemSerialNumber();

    multi->serial_ext = ctBEu32( multi->serial );
   
	////HASH
	objecthash.Insert(multi);
	////

    return multi;
}

#include "../uoscrobj.h"
BObjectImp* UMulti::scripted_create( const ItemDesc& descriptor, u16 x, u16 y, s8 z, Realm* realm, int flags )
{
    if (descriptor.type == ItemDesc::BOATDESC)
        return UBoat::scripted_create( descriptor, x, y, z, realm, flags );
    else if (descriptor.type == ItemDesc::HOUSEDESC)
        return UHouse::scripted_create( descriptor, x,y, z, realm, flags );
    else
        return new BError( "Don't know what kind of multi to make" );
}
