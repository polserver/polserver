/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#include "fnsearch.h"

#include "../clib/stlutil.h"

#include "globals/object_storage.h"
#include "mobile/charactr.h"
#include "item/item.h"
#include "storage.h"
#include "multi/multi.h"
#include "uworld.h"

namespace Pol {
  namespace Core {
	UObject* system_find_object( u32 serial )
	{
	  UObject* obj = objStorageManager.objecthash.Find( serial );
	  if ( obj != NULL && !obj->orphan() )
		return obj;
	  else
		return NULL;
	}

	Mobile::Character* system_find_mobile( u32 serial /*, int sysfind_flags*/ )
	{
	  UObject* obj = objStorageManager.objecthash.Find( serial );
	  if ( obj != NULL && obj->ismobile() && !obj->orphan() )
        return static_cast<Mobile::Character*>( obj );
	  else
		return NULL;
	}

	Items::Item *system_find_item( u32 serial/*, int sysfind_flags */ )
	{
	  UObject* obj = objStorageManager.objecthash.Find( serial );
	  if ( obj != NULL && obj->isitem() && !obj->orphan() )
        return static_cast<Items::Item*>( obj );
	  else
		return NULL;
	}

	Multi::UMulti* system_find_multi( u32 serial )
	{
	  UObject* obj = objStorageManager.objecthash.Find( serial );
	  if ( obj != NULL && obj->ismulti() && !obj->orphan() )
		return static_cast<Multi::UMulti*>( obj );
	  else
		return NULL;
	}

	// find_character: find a logged-in character given a serial number.
    Mobile::Character *find_character( u32 serial )
	{
      Mobile::Character* chr = system_find_mobile( serial );
	  if ( chr != NULL && chr->logged_in )
		return chr;
	  else
		return NULL;
	}

    Items::Item* find_toplevel_item( u32 serial )
	{
      Items::Item* item = system_find_item( serial );
	  if ( item != NULL && item->container != NULL )
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
	  if ( IsItem( serial ) )
	  {
		return find_toplevel_item( serial );
	  }
	  else
	  {
		return find_character( serial );
	  }
	}
  }
}