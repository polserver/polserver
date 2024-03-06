#include "EfswFileWatchListener.h"

#include "clib/logfacility.h"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace Pol::ECompile
{
EfswFileWatchListener::EfswFileWatchListener(
    efsw::FileWatcher& watcher, const std::set<std::filesystem::path>& extension_filter )
    : watcher( watcher ),
      extension_filter( extension_filter ),
      watched_dirs(),
      watched_files(),
      mutex(),
      messages(),
      handle_messages_by( 0 )
{
}

EfswFileWatchListener::~EfswFileWatchListener()
{
  for ( const auto& v : dir_to_watchid )
  {
    watcher.removeWatch( v.second );
  }
}

// Implements the libefsw virtual function for handling filesystem events.
void EfswFileWatchListener::handleFileAction( efsw::WatchID /*watchid*/, const std::string& dir,
                                              const std::string& filename, efsw::Action action,
                                              std::string oldFilename )
{
  fs::path dirPath( dir );
  auto filepath = dirPath / fs::path( filename );

  // Only send events for watched files
  if ( !is_watched( filepath ) )
  {
    return;
  }

  switch ( action )
  {
  case efsw::Actions::Delete:
    // Delete message: Empty filepath, Non-empty old_filepath
    add_message( WatchFileMessage{ fs::path( "" ), filepath } );
    break;

  case efsw::Actions::Moved:
  {
    // Moved message: Non-empty filepath, Non-empty old_filepath
    auto oldFilepath = dirPath / fs::path( oldFilename );
    add_message( WatchFileMessage{ filepath, oldFilepath } );
    break;
  }
  case efsw::Actions::Modified:
  case efsw::Actions::Add:
    // Changed message: Non-empty filepath, empty old_filepath
    add_message( WatchFileMessage{ filepath, fs::path( "" ) } );
    break;

  default:
    break;
  }
}

// Adds a file to watched files. Returns `true` if the file was newly added to
// `watched_files`.
bool EfswFileWatchListener::add_file( const fs::path& filepath )
{
  return watched_files.emplace( filepath ).second;
}


// Adds a directory to watched directories. Returns `true` if the directory was newly added to
// `watched_dirs`.
bool EfswFileWatchListener::add_dir( const std::filesystem::path& dir )
{
  return watched_dirs.emplace( dir ).second;
}


// Adds a directory to the filesystem watcher. Returns `true` if the directory
// has a newly added watchID added to `dir_to_watchid`.
//
// Throws if the filesystem watcher fails to add the watch.
bool EfswFileWatchListener::add_watch_dir( const std::filesystem::path& dir )
{
  if ( auto itr = dir_to_watchid.find( dir ); itr == dir_to_watchid.end() )
  {
    auto watchID = watcher.addWatch( dir.generic_string(), this, true );
    if ( watchID < 0 )
    {
      throw std::runtime_error( "Failed to watch " + dir.generic_string() );
    }
    dir_to_watchid.emplace( dir, watchID );
    return true;
  }

  return false;
}

// Removes a file from watched files. Returns `true` if the file was removed
// from `watched_files`.
bool EfswFileWatchListener::remove_file( const std::filesystem::path& filepath )
{
  return watched_files.erase( filepath ) == 1;
}

// Returns `true` if a file is watched. A file is watched if it is explicitly inside
// `watched_files`, or it is in a subdirectory of any directory inside
// `watched_dirs`.
bool EfswFileWatchListener::is_watched( const std::filesystem::path& filepath )
{
  if ( watched_files.find( filepath ) != watched_files.end() )
    return true;

  auto dir = filepath.parent_path();
  auto ext = fs::path( filepath ).extension();

  if ( extension_filter.find( ext ) != extension_filter.end() )
  {
    auto dirname = dir.generic_string();
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

// Adds a `message` to the message queue. The messages is only added if there is
// not one of the same `filepath` and `old_filepath`.
void EfswFileWatchListener::add_message( WatchFileMessage&& message )
{
  bool add = true;
  {
    std::lock_guard<std::mutex> lock( mutex );
    for ( auto itr = messages.begin(); itr != messages.end(); )
    {
      if ( itr->filepath == message.filepath && itr->old_filepath == message.old_filepath )
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

// Removes the messages and inserts them into the provided `to_message` list.
// The messages are only removed if the `handle_messages_by` time clock has
// passed.
void EfswFileWatchListener::take_messages( std::list<WatchFileMessage>& to_messages )
{
  std::lock_guard<std::mutex> lock( mutex );
  if ( Clib::wallclock() > handle_messages_by )
  {
    to_messages.splice( to_messages.end(), messages );
  }
}

}  // namespace Pol::ECompile