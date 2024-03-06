#ifndef E_COMPILE_EFSWFILEWATCHLISTENER_H
#define E_COMPILE_EFSWFILEWATCHLISTENER_H

#include "clib/wallclock.h"

#include <efsw/efsw.hpp>
#include <filesystem>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <string>

namespace Pol::ECompile
{
static constexpr Clib::wallclock_diff_t DEBOUNCE = 200;

class WatchFileMessage
{
public:
  std::filesystem::path filepath;
  std::filesystem::path old_filepath;
};

class EfswFileWatchListener : public efsw::FileWatchListener
{
public:
  EfswFileWatchListener( efsw::FileWatcher& watcher, const std::set<std::filesystem::path>& extension_filter );
  ~EfswFileWatchListener();
  void handleFileAction( efsw::WatchID watchid, const std::string& dir, const std::string& filename,
                         efsw::Action action, std::string oldFilename ) override;

  bool add_file( const std::filesystem::path& filepath );
  bool add_dir( const std::filesystem::path& dirname );
  bool add_watch_dir( const std::filesystem::path& dirname );
  bool remove_file( const std::filesystem::path& filepath );
  bool is_watched( const std::filesystem::path& filepath );
  void add_message( WatchFileMessage&& message );
  void take_messages( std::list<WatchFileMessage>& messages );

private:
  efsw::FileWatcher& watcher;
  std::set<std::filesystem::path> extension_filter;
  std::set<std::filesystem::path> watched_dirs;
  std::set<std::filesystem::path> watched_files;
  std::map<std::filesystem::path, efsw::WatchID> dir_to_watchid;

  mutable std::mutex mutex;
  // Access should be protected by `mutex`.
  std::list<WatchFileMessage> messages;
  // Access should be protected by `mutex`.
  Clib::wallclock_t handle_messages_by;
};
}  // namespace Pol::ECompile

#endif
