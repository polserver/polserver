#include "object_storage.h"

#include <string.h>

#include "../../clib/stlutil.h"
#include "../objecthash.h"
#include "../reftypes.h"
#include "../uobject.h"

namespace Pol
{
namespace Core
{
ObjectStorageManager objStorageManager;

ObjectStorageManager::ObjectStorageManager()
    : deferred_insertions(),
      modified_serials(),
      deleted_serials(),
      clean_objects( 0 ),
      dirty_objects( 0 ),
      objecthash()
{
}

ObjectStorageManager::~ObjectStorageManager() {}


void ObjectStorageManager::deinitialize()
{
  objecthash.Clear();
  deferred_insertions.clear();
  modified_serials.clear();
  deleted_serials.clear();
}

ObjectStorageManager::MemoryUsage ObjectStorageManager::estimateSize() const
{
  ObjectHash::OH_const_iterator hs_citr = objStorageManager.objecthash.begin(),
                                hs_cend = objStorageManager.objecthash.end();

  MemoryUsage usage;
  memset( &usage, 0, sizeof( usage ) );

  usage.objcount = std::distance( hs_citr, hs_cend );
  for ( ; hs_citr != hs_cend; ++hs_citr )
  {
    const UObjectRef& ref = ( *hs_citr ).second;
    auto size = ref->estimatedSize();
    usage.objsize += size;
    if ( ref->isa( UOBJ_CLASS::CLASS_ITEM ) )
    {
      usage.obj_item_size += size;
      usage.obj_item_count++;
    }
    else if ( ref->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
    {
      usage.obj_cont_size += size;
      usage.obj_cont_count++;
    }
    else if ( ref->isa( UOBJ_CLASS::CLASS_CHARACTER ) )
    {
      usage.obj_char_size += size;
      usage.obj_char_count++;
    }
    else if ( ref->isa( UOBJ_CLASS::CLASS_NPC ) )
    {
      usage.obj_npc_size += size;
      usage.obj_npc_count++;
    }
    else if ( ref->isa( UOBJ_CLASS::CLASS_WEAPON ) )
    {
      usage.obj_weapon_size += size;
      usage.obj_weapon_count++;
    }
    else if ( ref->isa( UOBJ_CLASS::CLASS_ARMOR ) )
    {
      usage.obj_armor_size += size;
      usage.obj_armor_count++;
    }
    else if ( ref->isa( UOBJ_CLASS::CLASS_MULTI ) )
    {
      usage.obj_multi_size += size;
      usage.obj_multi_count++;
    }
  }

  usage.misc = objecthash.estimateSize() + Clib::memsize( modified_serials ) +
               Clib::memsize( deleted_serials ) + Clib::memsize( deferred_insertions );

  return usage;
}
}  // namespace Core
}  // namespace Pol
