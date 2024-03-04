#ifndef E_COMPILE_EFSWFILEWATCHLISTENER_H
#define E_COMPILE_EFSWFILEWATCHLISTENER_H

#include "clib/wallclock.h"

#include <efsw/efsw.hpp>
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
EfswFileWatchListener(efsw::FileWatcher& watcher);
  void handleFileAction( efsw::WatchID watchid, const std::string& dir, const std::string& filename,
                         efsw::Action action, std::string oldFilename ) override;

  void add_watch_file(const std::string& filename);
  void add_watch_dir(const std::string& dirname);
  void remove_watch(const std::string& filename);
  bool is_watched( const std::string& dir, const std::string& filename );
  void add_message(WatchFileMessage message);
  void take_messages(std::list<WatchFileMessage>& messages);

private:
  efsw::FileWatcher& watcher;
  std::map<std::string, std::set<std::string>> dir_to_files;
  std::set<std::string> watched_dirs;
  std::map<std::string, efsw::WatchID> dir_to_watchid;

  std::list<WatchFileMessage> messages;
  mutable std::mutex _mutex;
  Clib::wallclock_t handle_messages_by;
};
}  // namespace Pol::Bscript::Compiler
#endif
