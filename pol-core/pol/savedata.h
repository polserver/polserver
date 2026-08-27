/** @file
 *
 * @par History
 */

#pragma once

#include <array>
#include <atomic>
#include <fstream>
#include <future>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "clib/rawtypes.h"
#include "clib/streamsaver.h"
#include "pol/gameclck.h"
#include "pol/saveparallel.h"
#include "pol/uoexec.h"

namespace Pol::Core
{
/// One output file of a world save: how much it received.
struct SaveFileStat
{
  std::string_view name;
  size_t bytes;
};

/// What a finished world save reports back.
struct SaveResult
{
  bool success{ false };
  u32 clean_writes{ 0 };
  u32 dirty_writes{ 0 };
  /// The part the world is stopped for: every object serialized and (today) written.
  s64 critical_ms{ 0 };
  /// Draining the write buffers and closing the files, after the world is running again.
  s64 flush_ms{ 0 };
  /// Handing formatted text to the files, summed over the threads that did it. A file takes its
  /// pieces one at a time, but different files do not wait for each other, so this is an upper
  /// bound on what the writing added to the save rather than a share of it.
  s64 write_ms{ 0 };
  /// The .ndt -> .txt renames.
  s64 commit_ms{ 0 };
  std::vector<SaveFileStat> files;
  std::vector<SaveTaskStat> tasks;
};

class SaveContext
{
  using SaveStrategy = Clib::StreamWriter;

public:
  SaveContext();
  // allow exception without direct terminate, performs fileoperations which can fail eg diskfull
  ~SaveContext() noexcept( false );
  SaveContext( const SaveContext& ) = delete;
  SaveContext& operator=( const SaveContext& ) = delete;
  SaveStrategy pol;
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

  /// Every output file paired with the basename it commits under, so that the writer list, the
  /// commit list and the reported statistics cannot drift apart.
  std::array<std::pair<std::string_view, Clib::StreamWriter*>, 12> files();

  static std::shared_future<void> finished;
  static void ready();
  static std::atomic<gameclock_t> last_worldsave_success;
};

void write_system_data( Clib::StreamWriter& sw );
void write_global_properties( Clib::StreamWriter& sw );
void write_shadow_realms( Clib::StreamWriter& sw );

bool commit( const std::string& basename );
bool should_write_data();
std::optional<bool> write_data( std::function<void( const SaveResult& )> callback,
                                SaveResult* critical_result = nullptr );
}  // namespace Pol::Core
