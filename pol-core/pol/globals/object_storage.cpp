#include "object_storage.h"

#include "../reftypes.h"
#include "../uobject.h"

namespace Pol
{
namespace Core
{
ObjectStorageManager objStorageManager;

ObjectStorageManager::ObjectStorageManager() :
  incremental_save_count(0),
  current_incremental_save(0),
  incremental_serial_index(),
  deferred_insertions(),
  modified_serials(),
  deleted_serials(),
  clean_objects(0),
  dirty_objects(0),
  incremental_saves_disabled(false),
  objecthash()
{

}

ObjectStorageManager::~ObjectStorageManager()
{
}


void ObjectStorageManager::deinitialize()
{
  objecthash.Clear();
  incremental_serial_index.clear();
  deferred_insertions.clear();
  modified_serials.clear();
  deleted_serials.clear();
}

ObjectStorageManager::MemoryUsage ObjectStorageManager::estimateSize() const
{
  ObjectHash::OH_const_iterator hs_citr = objStorageManager.objecthash.begin(), hs_cend = objStorageManager.objecthash.end();

  MemoryUsage usage;
  memset( &usage, 0, sizeof( usage ) );

  usage.objcount = std::distance( hs_citr, hs_cend );

  for ( ; hs_citr != hs_cend; ++hs_citr )
  {
    size_t size = ( sizeof(void*)* 3 + 1 ) / 2;
    const UObjectRef& ref = ( *hs_citr ).second;
    size += ref->estimatedSize();
    usage.objsize += size;
    if ( ref->isa( UObject::CLASS_ITEM ) )
    {
      usage.obj_item_size += size;
      usage.obj_item_count++;
    }
    else if ( ref->isa( UObject::CLASS_CONTAINER ) )
    {
      usage.obj_cont_size += size;
      usage.obj_cont_count++;
    }
    else if ( ref->isa( UObject::CLASS_CHARACTER ) )
    {
      usage.obj_char_size += size;
      usage.obj_char_count++;
    }
    else if ( ref->isa( UObject::CLASS_NPC ) )
    {
      usage.obj_npc_size += size;
      usage.obj_npc_count++;
    }
    else if ( ref->isa( UObject::CLASS_WEAPON ) )
    {
      usage.obj_weapon_size += size;
      usage.obj_weapon_count++;
    }
    else if ( ref->isa( UObject::CLASS_ARMOR ) )
    {
      usage.obj_armor_size += size;
      usage.obj_armor_count++;
    }
    else if ( ref->isa( UObject::CLASS_MULTI ) )
    {
      usage.obj_multi_size += size;
      usage.obj_multi_count++;
    }
  }

  usage.misc = sizeof(ObjectStorageManager);
  usage.misc += 3 * sizeof(u32*) + modified_serials.capacity() * sizeof( u32 );
  usage.misc += 3 * sizeof(u32*) + deleted_serials.capacity() * sizeof( u32 );

  usage.misc += ( sizeof(pol_serial_t)+sizeof( unsigned ) + ( sizeof(void*) * 3 + 1 ) / 2 ) * incremental_serial_index.size();
  usage.misc += ( sizeof(pol_serial_t)+sizeof( UObject*) + ( sizeof(void*) * 3 + 1 ) / 2 ) * deferred_insertions.size();

  return usage;
}
}
}
