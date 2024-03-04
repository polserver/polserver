#ifndef E_COMPILE_EFSWFILEWATCHLISTENER_H
#define E_COMPILE_EFSWFILEWATCHLISTENER_H

#include <efsw/efsw.hpp>
// #include <efsw/System.hpp>
// #include <efsw/FileSystem.hpp>

#include <map>
#include <set>
#include <string>
#include <functional>

namespace Pol::ECompile
{
using DependencyModifiedCallback = std::function<void(const std::string&)>;

class EfswFileWatchListener : public efsw::FileWatchListener
{
public:
EfswFileWatchListener(efsw::FileWatcher& watcher, DependencyModifiedCallback callback);
  void handleFileAction( efsw::WatchID watchid, const std::string& dir, const std::string& filename,
                         efsw::Action action, std::string oldFilename ) override;

  // void load_source( const std::string& filename );
  void add_watch(const std::string& filename);
  void remove_watch(const std::string& filename);
  // void add_watch_dir(const std::string& dirpath);
  bool is_watched( const std::string& dir, const std::string& filename );
private:
  efsw::FileWatcher& watcher;
  DependencyModifiedCallback callback;
  std::map<std::string, std::set<std::string>> dir_to_files;
  std::set<std::string> watched_dirs;
  std::map<std::string, efsw::WatchID> dir_to_watchid;
  // std::map<std::string, std::set<std::string>> source_to_deps;
  // std::map<std::string, std::string> dep_to_source;

  // void upsert_source_and_dependents( const std::string& filename );
};
}  // namespace Pol::Bscript::Compiler
#endif
