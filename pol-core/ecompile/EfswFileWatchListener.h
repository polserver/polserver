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
  std::string filename;
  std::string old_filename;
};

class EfswFileWatchListener : public efsw::FileWatchListener
{
public:
  EfswFileWatchListener( efsw::FileWatcher& watcher );
  ~EfswFileWatchListener();
  void handleFileAction( efsw::WatchID watchid, const std::string& dir, const std::string& filename,
                         efsw::Action action, std::string oldFilename ) override;

  void add_watch_file( const std::filesystem::path& filepath );
  void add_watch_dir( const std::filesystem::path& dirname );
  void remove_watch_file( const std::filesystem::path& filepath );
  bool is_watched( const std::filesystem::path& filepath );
  void add_message( WatchFileMessage message );
  void take_messages( std::list<WatchFileMessage>& messages );

private:
  efsw::FileWatcher& watcher;
  std::map<std::filesystem::path, std::set<std::filesystem::path>> dir_to_files;
  std::set<std::filesystem::path> watched_dirs;
  std::map<std::filesystem::path, efsw::WatchID> dir_to_watchid;

  mutable std::mutex mutex;
  // Access should be protected by `mutex`.
  std::list<WatchFileMessage> messages;
  // Access should be protected by `mutex`.
  Clib::wallclock_t handle_messages_by;
};
}  // namespace Pol::ECompile
#endif
