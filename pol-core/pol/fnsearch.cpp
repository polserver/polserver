/*
History
=======
2009/09/03 MuadDib:   Relocation of multi related cpp/h


Notes
=======

*/

#include "clib/stl_inc.h"

#include "fnsearch.h"

#include "clib/stlutil.h"

#include "charactr.h"
#include "item.h"
#include "storage.h"
#include "multi/multi.h"
#include "uworld.h"

#include "objecthash.h"


UObject* system_find_object( u32 serial )
{
	UObject* obj = objecthash.Find(serial);
	if(obj != NULL && !obj->orphan())
		return obj;
	else
		return NULL;
}

Character* system_find_mobile( u32 serial /*, int sysfind_flags*/ )
{
	UObject* obj = objecthash.Find(serial);
	if(obj != NULL && obj->ismobile() && !obj->orphan())
    	return static_cast<Character*>(obj);
	else
		return NULL;
}

Item *system_find_item( u32 serial/*, int sysfind_flags */)
{
	UObject* obj = objecthash.Find(serial);
	if(obj != NULL && obj->isitem() && !obj->orphan())
		return static_cast<Item*>(obj);
	else
		return NULL;
}

UMulti* system_find_multi( u32 serial )
{
	UObject* obj = objecthash.Find(serial);
	if(obj != NULL && obj->ismulti() && !obj->orphan())
		return static_cast<UMulti*>(obj);
	else
		return NULL;
}

// find_character: find a logged-in character given a serial number.
Character *find_character( u32 serial )
{
    Character* chr = system_find_mobile( serial );
    if (chr != NULL && chr->logged_in)
        return chr;
    else
        return NULL;
}

Item* find_toplevel_item( u32 serial )
{
    Item* item = system_find_item( serial );
    if (item && item->container)
        return NULL;
    //{
    //    const UObject* owner = item->toplevel_owner();
    //    if (owner->ismobile())
    //    {
    //        Character* chr = static_cast<Character*>(owner);
    //        if (!chr->logged_in)
    //            return NULL;
    //    }
    //}
    return item;
}

UObject *find_toplevel_object( u32 serial )
{
    if (IsItem( serial ))
    {
        return find_toplevel_item( serial );
    }
    else
    {
        return find_character( serial );
    }
}
