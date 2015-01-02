#include "object_storage.h"


namespace Pol {
namespace Core {
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
}
}
