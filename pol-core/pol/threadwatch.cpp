/** @file
 *
 * @par History
 */

#include "pol/threadwatch.h"

#include <map>
#include <mutex>

#include "clib/threadhelp.h"

namespace Pol::Core
{
namespace
{
struct WatchedThread
{
  std::string name;
  bool reported = false;
};

std::mutex watched_mutex;
std::map<size_t, WatchedThread> watched_threads;

// No real thread id is 0, so it can stand for a watched thread that can never be found.
constexpr size_t absent_thread_id = 0;

/// Collects the watched threads gone from the ThreadMap, and how many are watched. Both under
/// one lock: the report prints the count beside the thread listing, and comparing the two is
/// what catches a thread registered under another thread's id -- which nothing here can see,
/// since a wrong but live id looks exactly like a healthy one.
///
/// @param only_unreported skip the ones already reported, and mark those returned.
WatchedThreadStatus collect_stopped( bool only_unreported )
{
  // Copied before taking our own lock, so the two are never held at once.
  threadhelp::ThreadMap::Contents live;
  threadhelp::threadmap_instance().CopyContents( live );

  WatchedThreadStatus status;
  std::lock_guard<std::mutex> guard( watched_mutex );
  status.watched = watched_threads.size();
  for ( auto& entry : watched_threads )
  {
    if ( live.count( entry.first ) != 0 )
      continue;
    if ( only_unreported )
    {
      if ( entry.second.reported )
        continue;
      entry.second.reported = true;
    }
    status.missing.push_back( entry.second.name );
  }
  return status;
}
}  // namespace

void watch_this_thread()
{
  // clib publishes the name before handing control to the thread's function. It is capped at
  // 31 characters there, while the thread listing keeps the full string.
  std::lock_guard<std::mutex> guard( watched_mutex );
  watched_threads[threadhelp::thread_pid()] =
      WatchedThread{ threadhelp::current_thread_name(), false };
}

void unwatch_this_thread()
{
  std::lock_guard<std::mutex> guard( watched_mutex );
  watched_threads.erase( threadhelp::thread_pid() );
}

WatchedThreadStatus watched_thread_status()
{
  return collect_stopped( false );
}

std::vector<std::string> take_unreported_stopped_threads()
{
  return collect_stopped( true ).missing;
}

void watch_absent_thread_for_testing( std::string name )
{
  std::lock_guard<std::mutex> guard( watched_mutex );
  watched_threads[absent_thread_id] = WatchedThread{ std::move( name ), false };
}
}  // namespace Pol::Core
