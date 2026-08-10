#ifndef GLOBALS_OBJ_STORAGE_H
#define GLOBALS_OBJ_STORAGE_H

#include <cstddef>
#include <map>
#include <unordered_map>
#include <vector>

#include "clib/rawtypes.h"
#include "plib/poltype.h"
#include "pol/objecthash.h"


namespace Pol::Core
{
/// An object whose container had not been read yet, keyed on that container's serial.
///
/// The layer and slot are the ones the save recorded. They are carried rather than left on the
/// object because they say where it goes rather than what it is: the layer is needed to rebuild a
/// corpse's list, which is not saved, and the slot is a preference the container may not be able
/// to honour.
struct DeferredInsertion
{
  UObject* obj;
  u8 saved_layer;
  u8 saved_slot;
};

// if index is UINT_MAX, has been deleted
using DeferList = std::multimap<pol_serial_t, DeferredInsertion>;

class ObjectStorageManager
{
public:
  ObjectStorageManager();
  ~ObjectStorageManager();
  ObjectStorageManager( const ObjectStorageManager& ) = delete;
  ObjectStorageManager& operator=( const ObjectStorageManager& ) = delete;

  void deinitialize();
  struct MemoryUsage;

  MemoryUsage estimateSize() const;

  DeferList deferred_insertions;
  std::vector<u32> modified_serials;
  std::vector<u32> deleted_serials;
  unsigned int clean_objects;
  unsigned int dirty_objects;

  ObjectHash objecthash;

  struct MemoryUsage
  {
    size_t misc;

    size_t objsize;
    size_t objcount;

    size_t obj_item_size;
    size_t obj_cont_size;
    size_t obj_char_size;
    size_t obj_npc_size;
    size_t obj_weapon_size;
    size_t obj_armor_size;
    size_t obj_multi_size;
    size_t obj_item_count;
    size_t obj_cont_count;
    size_t obj_char_count;
    size_t obj_npc_count;
    size_t obj_weapon_count;
    size_t obj_armor_count;
    size_t obj_multi_count;
  };
};

extern ObjectStorageManager objStorageManager;
}  // namespace Pol::Core

#endif
