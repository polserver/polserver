/*
History
=======
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2009/09/03 MuadDib:   Relocation of multi related cpp/h

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include "../../clib/passert.h"
#include "../../clib/strutil.h"

#include "../../bscript/bstruct.h"
#include "../../bscript/objmembers.h"

#include "../item/itemdesc.h"
#include "../objtype.h"

#include "../udatfile.h"
#include "../uobjcnt.h"
#include "../uobject.h"
#include "../uofilei.h"
#include "../uofile.h"
#include "multi.h"
#include "../ustruct.h"
#include "../polcfg.h"

UMulti::UMulti( const ItemDesc& itemdesc )
    : Item(itemdesc, CLASS_MULTI)
{
	multiid = itemdesc.multiid;

    if (!MultiDefByMultiIDExists( itemdesc.multiid ))
    {
        cerr << "Tried to create a Multi type " << hexint(itemdesc.objtype) << endl;
        throw runtime_error( "Unvalid Multi type" );
    }
    ++umulti_count;
}

UMulti::~UMulti()
{
    --umulti_count;
}

void UMulti::double_click( Client* client )
{
    cerr << "Ack! You can't double-click a multi!" << endl;
    passert(0);
    throw runtime_error( "double_click() on a multi should not be possible." );
}

UBoat* UMulti::as_boat()
{
    return NULL;
}
UHouse* UMulti::as_house()
{
    return NULL;
}

void UMulti::register_object( UObject* obj )
{
}

void UMulti::unregister_object( UObject* obj )
{
}

const char* UMulti::classname() const
{
    return "MULTI";
}

const MultiDef& UMulti::multidef() const
{
    passert( MultiDefByMultiIDExists(multiid));

    return *MultiDefByMultiID(multiid);
}

BStruct* UMulti::footprint() const
{
	const MultiDef& md = multidef();
    std::unique_ptr<BStruct> ret (new BStruct);
	ret->addMember( "xmin", new BLong( x + md.minrx ) );
    ret->addMember( "xmax", new BLong( x + md.maxrx ) );
	ret->addMember( "ymin", new BLong( y + md.minry ) );
	ret->addMember( "ymax", new BLong( y + md.maxry ) );
	return ret.release();
}

BObjectImp* UMulti::get_script_member_id( const int id ) const ///id test
{
    BObjectImp* imp = base::get_script_member_id( id );
    if (imp)
        return imp;

    switch(id)
    {
		case MBR_FOOTPRINT: return footprint(); break;
        default: return NULL;
    }
}

BObjectImp* UMulti::get_script_member( const char *membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
	/*
    BObjectImp* imp = base::get_script_member( membername );
    if (imp)
        return imp;

    if (stricmp( membername, "footprint" ) == 0)
	{
		return footprint();
	}
    return NULL;
	*/
}
