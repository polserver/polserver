#ifndef GLOBALS_OBJ_STORAGE_H
#define GLOBALS_OBJ_STORAGE_H

#include <cstddef>
#include <map>
#include <unordered_map>
#include <vector>

#include "../../clib/rawtypes.h"
#include "../../plib/poltype.h"
#include "../objecthash.h"

namespace Pol
{
namespace Core
{
// if index is UINT_MAX, has been deleted
using DeferList = std::multimap<pol_serial_t, UObject*>;

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
}  // namespace Core
}  // namespace Pol
#endif
