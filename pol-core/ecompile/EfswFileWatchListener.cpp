#include "EfswFileWatchListener.h"

#include "clib/logfacility.h"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace Pol::ECompile
{
EfswFileWatchListener::EfswFileWatchListener( efsw::FileWatcher& watcher,
                                              DependencyModifiedCallback callback )
    : watcher( watcher ), callback( callback )
{
}

void EfswFileWatchListener::handleFileAction( efsw::WatchID /*watchid*/, const std::string& dir,
                                              const std::string& filename, efsw::Action action,
                                              std::string oldFilename )
{
  switch ( action )
  {
  case efsw::Actions::Add:
    // std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Added"
    //           << std::endl;
    INFO_PRINTLN( "DIR ({}) FILE ({}) has event Added", dir, filename );
    break;
  case efsw::Actions::Delete:
    // std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Delete"
    //           << std::endl;
    INFO_PRINTLN( "DIR ({}) FILE ({}) has event Delete", dir, filename );
    break;
  case efsw::Actions::Modified:
  {
    INFO_PRINTLN( "DIR ({}) FILE ({}) has event Modified", dir, filename );
    if ( is_watched( dir, filename ) )
    {
      callback( ( fs::path( dir ) / filename ).generic_string() );
    }
    break;
  }
  case efsw::Actions::Moved:
    // std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Moved from ("
    //           << oldFilename << ")" << std::endl;
    INFO_PRINTLN( "DIR ({}) FILE ({}) has event Moved from ({})", dir, filename, oldFilename );
    break;
  default:
    // std::cout << "Should never happen!" << std::endl;
    break;
  }
}

// void EfswFileWatchListener::load_source( const std::string& filename )
// {
//   upsert_source_and_dependents( filename );
// }

void EfswFileWatchListener::add_watch( const std::string& filename )
{
  fs::path filepath( filename );

  auto dir = filepath.parent_path().generic_string();

  if ( dir_to_watchid.find( dir ) == dir_to_watchid.end() )
  {
    auto watchID = watcher.addWatch( dir, this, false );
    INFO_PRINTLN( "- {} [{}] {}", dir, watchID );
    dir_to_watchid.emplace( dir, watchID );
  }
  if ( fs::is_directory( filepath ) )
  {
    watched_dirs.emplace( filename );
  }
  else
  {
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
      // itr->second.emplace( basename );
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

// void EfswFileWatchListener::add_watch_dir( const std::string& dirpath ) {}

bool EfswFileWatchListener::is_watched( const std::string& dir, const std::string& filename )
{
  // if ( watched_dirs.find(dir) !=
  auto dir_itr = dir_to_files.find( fs::canonical( dir ) );
  if ( dir_itr == dir_to_files.end() )
  {
    return false;
  }

  auto file_itr = dir_itr->second.find( filename );

  return file_itr != dir_itr->second.end();
}

// void EfswFileWatchListener::upsert_source_and_dependents( const std::string& filename )
// {
//   callback( filename );
//   fs::path filepath( filename );

//   source_to_deps.find(filename);
//   std::string filename_dep = filepath.replace_extension( ".dep" ).generic_string();

//   std::ifstream ifs( filename_dep.c_str() );

//   if ( !ifs.is_open() )
//   {
//     throw std::runtime_error( "Could not load dependency information." );
//   }

//   std::string depname;
//   while ( getline( ifs, depname ) )
//   {
//     add_watch( depname );
//   }
// }

}  // namespace Pol::ECompile