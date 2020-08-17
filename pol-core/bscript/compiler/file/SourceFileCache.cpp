#include "SourceFileCache.h"

#include "clib/logfacility.h"
#include "clib/timer.h"
#include "compiler/Profile.h"
#include "compiler/file/SourceFile.h"
#include "compiler/file/SourceFileIdentifier.h"

namespace Pol::Bscript::Compiler
{
void SourceFileCache::configure( unsigned how_many )
{
  keep = how_many;
}

std::shared_ptr<SourceFile> SourceFileCache::load( const SourceFileIdentifier& ident,
                                                   Report& report )
{
  const std::string& pathname = ident.pathname;

  if ( keep > 0 )
  {
    const std::lock_guard<std::mutex> guard( mutex );
    ++frequency[pathname];

    auto itr = files.find( pathname );
    if ( itr != files.end() )
    {
      // INFO_PRINT << "Cache hit: " << pathname << "\n";
      profile.cache_hits++;

      // We do not propagate errors here.  This happens when the compilation unit
      // or module is obtained, so that the errors occur at the same "time"
      // whether the file was just loaded, or was cached.
      return ( *itr ).second;
    }
    // INFO_PRINT << "Cache miss: " << pathname << "\n";
    profile.cache_misses++;
  }

  auto sf = SourceFile::load( ident, profile, report );

  if ( keep > 0 && sf )
  {
    const std::lock_guard<std::mutex> guard( mutex );
    auto ins = files.insert( { pathname, sf } );
    if ( !ins.second )
    {
      sf = ( *ins.first ).second;
    }
  }

  return sf;
}

struct SourceFileAndFrequency
{
  std::shared_ptr<SourceFile> source_file;
  unsigned frequency;
};

void SourceFileCache::keep_some()
{
  if ( keep == 0 )
    return;

  std::unique_lock<std::mutex> lock( mutex, std::try_to_lock_t() );
  if ( !lock.owns_lock() )
    return;

  if ( files.size() <= keep )
    return;

  Pol::Tools::HighPerfTimer select_timer;

  std::vector<SourceFileAndFrequency> pathname_frequencies;
  pathname_frequencies.reserve(files.size());
  for ( auto& kv : files )
  {
    pathname_frequencies.push_back( SourceFileAndFrequency{ kv.second, frequency[kv.first] } );
  }
  std::sort( pathname_frequencies.begin(), pathname_frequencies.end(),
             []( const SourceFileAndFrequency& lhs, const SourceFileAndFrequency& rhs ) {
               return lhs.frequency > rhs.frequency;
             } );

  profile.prune_cache_select_micros += select_timer.ellapsed().count();

  Pol::Tools::HighPerfTimer delete_timer;
  for ( auto itr = pathname_frequencies.begin() + keep; itr != pathname_frequencies.end(); ++itr )
  {
    // INFO_PRINT << "Remove from cache: " << ( *itr ).source_file->pathname
    //                                            << "(" << ( *itr ).frequency << ")\n";
    files.erase( ( *itr ).source_file->pathname );
    (*itr).source_file.reset();
  }
  profile.prune_cache_delete_micros += delete_timer.ellapsed().count();

  // INFO_PRINT << "Cache kept:\n";
  // for( auto& kv : files )
  // {
  //   INFO_PRINT << "  - " << kv.first << " (" << frequency[kv.first] << ")\n";
  // }
}

}  // namespace Pol::Bscript::Compiler
