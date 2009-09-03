/*
History
=======
2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule

Notes
=======

*/

#include "clib/stl_inc.h"

#include "bscript/bobject.h"
#include "bscript/berror.h"
#include "bscript/impstr.h"

#include "realms.h"
#include "storage.h"
#include "storagemod.h"
#include "uoexhelp.h"
#include "uoscrobj.h"

template<>
TmplExecutorModule<StorageExecutorModule>::FunctionDef
	TmplExecutorModule<StorageExecutorModule>::function_table[] =
{
	{ "StorageAreas",					&StorageExecutorModule::mf_StorageAreas },
	{ "FindStorageArea",				&StorageExecutorModule::mf_FindStorageArea },
	{ "CreateStorageArea",				&StorageExecutorModule::mf_CreateStorageArea },
	{ "FindRootItemInStorageArea",		&StorageExecutorModule::mf_FindRootItemInStorageArea },
	{ "CreateRootItemInStorageArea",	&StorageExecutorModule::mf_CreateRootItemInStorageArea },
	{ "DestroyRootItemInStorageArea",	&StorageExecutorModule::mf_DestroyRootItemInStorageArea }	
};
template<>
int TmplExecutorModule<StorageExecutorModule>::function_table_size =
	arsize(function_table);

BObjectImp* CreateStorageAreasImp();
BObjectImp* StorageExecutorModule::mf_StorageAreas()
{
    return CreateStorageAreasImp();
}


BObjectImp* StorageExecutorModule::mf_FindStorageArea()
{
	const String* str = getStringParam(0);
	if ( str != NULL )
	{
		StorageArea *area = storage.find_area(str->value());

		if ( area )
			return new BApplicPtr(&storage_area_type, area);
	}
	return new BLong(0); // non-string passed, or not found.
}

BObjectImp* StorageExecutorModule::mf_CreateStorageArea()
{
	String* name = EXPLICIT_CAST(String*, BObjectImp*)(getParamImp(0, BObjectImp::OTString));
	if ( name )
	{
		StorageArea *area = storage.create_area(name->value());
		if ( area )
			return new BApplicPtr(&storage_area_type, area);
	}
	return new BLong( 0 ); // non-string passed, or not found.
}

BObjectImp* StorageExecutorModule::mf_FindRootItemInStorageArea()
{
	StorageArea* area = EXPLICIT_CAST(StorageArea*, void*)(exec.getApplicPtrParam(0, &storage_area_type));
	const String *name = getStringParam(1);

	if ( !area || !name )
		return new BError("Invalid parameter type");

	Item *item = area->find_root_item(name->value());
	
	if ( item != NULL )
		return new EItemRefObjImp(item);
	else
		return new BError("Root item not found.");
}

BObjectImp* StorageExecutorModule::mf_DestroyRootItemInStorageArea()
{
	StorageArea* area = EXPLICIT_CAST(StorageArea*, void*)(getApplicPtrParam(0, &storage_area_type));
	const String *name = getStringParam(1);

	if ( !area || !name )
		return new BError("Invalid parameter type");

	bool result = area->delete_root_item(name->value());
	return new BLong( result ? 1 : 0 );
}

BObjectImp* StorageExecutorModule::mf_CreateRootItemInStorageArea()
{ 
	StorageArea* area = EXPLICIT_CAST(StorageArea*, void*)(getApplicPtrParam(0, &storage_area_type));
	const String* name;
	const ItemDesc* descriptor;

	if ( area == NULL || !getStringParam(1, name) || !getObjtypeParam(exec, 2, descriptor) )
		return new BError("Invalid parameter type");

	Item *item = Item::create(*descriptor);
	if ( item == NULL )
		return new BError("Unable to create item");

	item->setname(name->value());

	if(item->realm == NULL)
		item->realm = find_realm(string("britannia"));

	area->insert_root_item(item);

	return new EItemRefObjImp(item);
}


