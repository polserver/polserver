/** @file
 *
 * @par History
 */

#pragma once

#include <atomic>
#include <fstream>
#include <future>
#include <optional>
#include <string>

#include "../clib/streamsaver.h"
#include "gameclck.h"
#include "uoexec.h"

namespace Pol::Core
{
class SaveContext
{
  typedef Clib::StreamWriter SaveStrategy;

private:
  std::ofstream _pol;
  std::ofstream _objects;
  std::ofstream _pcs;
  std::ofstream _pcequip;
  std::ofstream _npcs;
  std::ofstream _npcequip;
  std::ofstream _items;
  std::ofstream _multis;
  std::ofstream _storage;
  std::ofstream _resource;
  std::ofstream _guilds;
  std::ofstream _datastore;
  std::ofstream _party;

public:
  SaveContext();
  // allow exception without direct terminate, performs fileoperations which can fail eg diskfull
  ~SaveContext() noexcept( false );
  SaveContext( const SaveContext& ) = delete;
  SaveContext& operator=( const SaveContext& ) = delete;
  SaveStrategy pol;
  SaveStrategy objects;
  SaveStrategy pcs;
  SaveStrategy pcequip;
  SaveStrategy npcs;
  SaveStrategy npcequip;
  SaveStrategy items;
  SaveStrategy multis;
  SaveStrategy storage;
  SaveStrategy resource;
  SaveStrategy guilds;
  SaveStrategy datastore;
  SaveStrategy party;
  static std::shared_future<void> finished;
  static void ready();
  static std::atomic<gameclock_t> last_worldsave_success;
};

void write_system_data( Clib::StreamWriter& sw );
void write_global_properties( Clib::StreamWriter& sw );
void write_shadow_realms( Clib::StreamWriter& sw );

bool commit( const std::string& basename );
bool should_write_data();
std::optional<bool> write_data( std::function<void( bool, u32, u32, s64 )> callback,
                                u32* dirty_writes = nullptr, u32* clean_writes = nullptr,
                                s64* elapsed_ms = nullptr );
}  // namespace Pol::Core
