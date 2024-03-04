#include "EfswFileWatchListener.h"

#include "clib/logfacility.h"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace Pol::ECompile
{
EfswFileWatchListener::EfswFileWatchListener( efsw::FileWatcher& watcher )
    : watcher( watcher ), messages(), _mutex(), handle_messages_by(0)
{
}

void EfswFileWatchListener::handleFileAction( efsw::WatchID /*watchid*/, const std::string& dir,
                                              const std::string& filename, efsw::Action action,
                                              std::string oldFilename )
{
  auto filepath = fs::path(dir) / fs::path(filename);
  auto filepathstr = filepath.generic_string();
  if (!is_watched(dir, filename)) {
    return;
  }
  switch ( action )
  {
  case efsw::Actions::Delete:
    INFO_PRINTLN( "DIR ({}) FILE ({}) has event Delete", dir, filename );
    add_message(WatchFileMessage{"", filepathstr});
    break;
  case efsw::Actions::Modified:
  case efsw::Actions::Add:
  case efsw::Actions::Moved:
    INFO_PRINTLN( "DIR ({}) FILE ({}) has event Added/Modified/Moved", dir, filename );
    add_message(WatchFileMessage{filepathstr, oldFilename});
    break;
  default:
    break;
  }
}

void EfswFileWatchListener::add_watch_file( const std::string& filename )
{
  fs::path filepath( filename );

  auto dir = filepath.parent_path().generic_string();

  if ( dir_to_watchid.find( dir ) == dir_to_watchid.end() )
  {
    auto watchID = watcher.addWatch( dir, this, false );
    dir_to_watchid.emplace( dir, watchID );
  }

  auto basename = filepath.filename().generic_string();
  auto itr = dir_to_files.find( dir );
  if ( itr == dir_to_files.end() )
  {
    dir_to_files.emplace( dir, std::set<std::string>{ basename } );
  }
  else
  {
    itr->second.emplace( basename );
  }
}

void EfswFileWatchListener::add_watch_dir( const std::string& dirname )
{
  auto dir = fs::canonical(fs::path(dirname)).generic_string();
  if ( dir_to_watchid.find( dir ) == dir_to_watchid.end() )
  {
    auto watchID = watcher.addWatch( dir, this, true );
    dir_to_watchid.emplace( dir, watchID );
  }
  watched_dirs.emplace( dir );
}

void EfswFileWatchListener::remove_watch( const std::string& filename ) {
fs::path filepath( filename );
  bool remove_watch = false;

  auto dir_to_watchid_itr = dir_to_watchid.end();

  if ( auto watch_dir_itr = watched_dirs.find(filename); watch_dir_itr != watched_dirs.end())
  {
    watched_dirs.erase(watch_dir_itr);
    dir_to_watchid_itr = dir_to_watchid.find(filename);
    remove_watch = true;
  }
  else if (!fs::is_directory(filepath))
  {
    auto dir = filepath.parent_path().generic_string();
    dir_to_watchid_itr = dir_to_watchid.find(dir);
    auto basename = filepath.filename().generic_string();
    auto dir_to_files_itr = dir_to_files.find( dir );
    if ( dir_to_files_itr != dir_to_files.end() )
    {
      auto &files = dir_to_files_itr->second;
      files.erase(basename);
      if (files.size() == 0)
      {
        dir_to_files.erase(dir_to_files_itr);
      } else {
        remove_watch = false;
      }
    }
  }
  if ( remove_watch && dir_to_watchid_itr != dir_to_watchid.end() )
  {
    ERROR_PRINTLN( "Remove watch {} [{}]", dir_to_watchid_itr->first, dir_to_watchid_itr->second );
    watcher.removeWatch( dir_to_watchid_itr->second );
    dir_to_watchid.erase( dir_to_watchid_itr );
  }
}

bool EfswFileWatchListener::is_watched( const std::string& dirname, const std::string& filename )
{
  auto dir = fs::canonical( fs::path( dirname ) ).generic_string();
  auto dir_itr = dir_to_files.find( fs::canonical( dir ) );
  if ( dir_itr != dir_to_files.end() )
  {
    auto file_itr = dir_itr->second.find( filename );

    if ( file_itr != dir_itr->second.end() )
      return true;
  }

  auto ext = fs::path( filename ).extension().generic_string();
  if ( ext.compare( ".src" ) == 0 || ext.compare( ".hsr" ) == 0 || ext.compare( ".asp" ) == 0 )
  {
    for ( const auto& dir : watched_dirs )
    {
      if ( dirname.rfind( dir, 0 ) == 0 )
      {
        return true;
      }
    }
  }
  return false;
}

void EfswFileWatchListener::add_message( WatchFileMessage message )
{
  bool add = true;
  {
    std::lock_guard<std::mutex> lock( _mutex );
    for ( auto itr = messages.begin(); itr != messages.end(); )
    {
      if ( itr->filename == message.filename && itr->old_filename == message.old_filename )
      {
        add = false;
        break;
      }
    }
    if ( add )
    {
      messages.push_back( message );
      handle_messages_by = Clib::wallclock() + DEBOUNCE;
    }
  }
}

void EfswFileWatchListener::take_messages(std::list<WatchFileMessage>& to_messages)
{
  std::lock_guard<std::mutex> lock( _mutex );
  if (Clib::wallclock() > handle_messages_by)
  {
        to_messages.splice(to_messages.end(), messages);

  }
}

}  // namespace Pol::ECompile