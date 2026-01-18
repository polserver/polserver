/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#include "fnsearch.h"

#include <stddef.h>

#include "globals/object_storage.h"
#include "item/item.h"
#include "mobile/charactr.h"
#include "multi/multi.h"
#include "objecthash.h"
#include "uobject.h"


namespace Pol::Core
{
UObject* system_find_object( u32 serial )
{
  UObject* obj = objStorageManager.objecthash.Find( serial );
  if ( obj != nullptr && !obj->orphan() )
    return obj;
  return nullptr;
}

Mobile::Character* system_find_mobile( u32 serial /*, int sysfind_flags*/ )
{
  UObject* obj = objStorageManager.objecthash.Find( serial );
  if ( obj != nullptr && obj->ismobile() && !obj->orphan() )
    return static_cast<Mobile::Character*>( obj );
  return nullptr;
}

Items::Item* system_find_item( u32 serial /*, int sysfind_flags */ )
{
  UObject* obj = objStorageManager.objecthash.Find( serial );
  if ( obj != nullptr && obj->isitem() && !obj->orphan() )
    return static_cast<Items::Item*>( obj );
  return nullptr;
}

Multi::UMulti* system_find_multi( u32 serial )
{
  UObject* obj = objStorageManager.objecthash.Find( serial );
  if ( obj != nullptr && obj->ismulti() && !obj->orphan() )
    return static_cast<Multi::UMulti*>( obj );
  return nullptr;
}

// find_character: find a logged-in character given a serial number.
Mobile::Character* find_character( u32 serial )
{
  Mobile::Character* chr = system_find_mobile( serial );
  if ( chr != nullptr && chr->logged_in() )
    return chr;
  return nullptr;
}

Items::Item* find_toplevel_item( u32 serial )
{
  Items::Item* item = system_find_item( serial );
  if ( item != nullptr && item->container != nullptr )
    return nullptr;
  //{
  //    const UObject* owner = item->toplevel_owner();
  //    if (owner->ismobile())
  //    {
  //        Character* chr = static_cast<Character*>(owner);
  //        if (!chr->logged_in)
  //            return nullptr;
  //    }
  //}
  return item;
}

UObject* find_toplevel_object( u32 serial )
{
  if ( IsItem( serial ) )
  {
    return find_toplevel_item( serial );
  }

  return find_character( serial );
}
}  // namespace Pol::Core
