#include "EfswFileWatchListener.h"

#include "clib/logfacility.h"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;
template <>
struct fmt::formatter<fs::path> : fmt::formatter<std::string>
{
  fmt::format_context::iterator format( const fs::path& l, fmt::format_context& ctx ) const
  {
    {
      return fmt::formatter<std::string>::format( l.generic_string(), ctx );
    }
  }
};

namespace Pol::ECompile
{
EfswFileWatchListener::EfswFileWatchListener( efsw::FileWatcher& watcher )
    : watcher( watcher ), mutex(), messages(), handle_messages_by( 0 )
{
}

EfswFileWatchListener::~EfswFileWatchListener()
{
  for ( const auto& watchID : dir_to_watchid )
  {
    watcher.removeWatch( watchID.second );
  }
}

void EfswFileWatchListener::handleFileAction( efsw::WatchID /*watchid*/, const std::string& dir,
                                              const std::string& filename, efsw::Action action,
                                              std::string oldFilename )
{
  auto filepath = fs::path( dir ) / fs::path( filename );
  auto filepathstr = filepath.generic_string();
  if ( !is_watched( filepath ) )
  {
    return;
  }

  switch ( action )
  {
  case efsw::Actions::Delete:
    INFO_PRINTLN( "DIR ({}) FILE ({}) has event Delete", dir, filename );
    add_message( WatchFileMessage{ "", filepathstr } );
    break;

  case efsw::Actions::Modified:
  case efsw::Actions::Add:
  case efsw::Actions::Moved:
    INFO_PRINTLN( "DIR ({}) FILE ({}) has event Added/Modified/Moved", dir, filename );
    add_message( WatchFileMessage{ filepathstr, oldFilename } );
    break;

  default:
    break;
  }
}

void EfswFileWatchListener::add_watch_file( const fs::path& filepath )
{
  auto dir = filepath.parent_path();
  efsw::WatchID watchID;

  if ( auto itr = dir_to_watchid.find( dir ); itr != dir_to_watchid.end() )
  {
    watchID = itr->second;
  }
  else
  {
    watchID = watcher.addWatch( dir.generic_string(), this, false );
    ERROR_PRINTLN( "Add watch: {} [{}]", dir, watchID );
    dir_to_watchid.emplace( dir, watchID );
  }

  auto filename = filepath.filename();
  auto itr = dir_to_files.find( dir );
  if ( itr == dir_to_files.end() )
  {
    dir_to_files.emplace( dir, std::set<fs::path>{ filename } );
    ERROR_PRINTLN( "  Files: [{}]", filename );
  }
  else
  {
    itr->second.emplace( filename );
    ERROR_PRINTLN(
        "Update watch: {} [{}]\n"
        "  Files: [{}]",
        dir, watchID, itr->second );
  }
}

void EfswFileWatchListener::add_watch_dir( const std::filesystem::path& dir )
{
  efsw::WatchID watchID;
  if ( auto itr = dir_to_watchid.find( dir ); itr != dir_to_watchid.end() )
  {
    watchID = itr->second;
    ERROR_PRINTLN( "Set as watch dir: {} [{}]", dir, watchID );
  }
  else
  {
    watchID = watcher.addWatch( dir, this, true );
    dir_to_watchid.emplace( dir, watchID );
    ERROR_PRINTLN( "Add watch dir: {} [{}]", dir, watchID );
  }
  watched_dirs.emplace( dir );
}

void EfswFileWatchListener::remove_watch_file( const std::filesystem::path& filepath )
{
  bool remove_watch = false;

  auto dir = filepath.parent_path();
  auto filename = filepath.filename();
  auto dir_to_files_itr = dir_to_files.find( dir );
  if ( dir_to_files_itr != dir_to_files.end() )
  {
    auto& files = dir_to_files_itr->second;
    files.erase( filename );
    if ( files.size() == 0 )
    {
      dir_to_files.erase( dir_to_files_itr );
      remove_watch = true;
    }
    else
    {
      remove_watch = false;
    }
  }

  if ( remove_watch )
  {
    if ( auto dir_to_watchid_itr = dir_to_watchid.find( dir );
         dir_to_watchid_itr != dir_to_watchid.end() )
    {
      ERROR_PRINTLN( "Remove watch {} [{}]", dir_to_watchid_itr->first,
                     dir_to_watchid_itr->second );
      watcher.removeWatch( dir_to_watchid_itr->second );
      dir_to_watchid.erase( dir_to_watchid_itr );
    }
  }
}

bool EfswFileWatchListener::is_watched( const std::filesystem::path& filepath )
{
  auto dir = filepath.parent_path();
  auto filename = filepath.filename();
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

void EfswFileWatchListener::add_message( WatchFileMessage message )
{
  bool add = true;
  {
    std::lock_guard<std::mutex> lock( mutex );
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

void EfswFileWatchListener::take_messages( std::list<WatchFileMessage>& to_messages )
{
  std::lock_guard<std::mutex> lock( mutex );
  if ( Clib::wallclock() > handle_messages_by )
  {
    to_messages.splice( to_messages.end(), messages );
  }
}

}  // namespace Pol::ECompile