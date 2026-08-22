/** @file
 *
 * @par History
 */

#include "pol/threadwatch.h"

#include <atomic>
#include <map>
#include <memory>
#include <mutex>

#include "clib/strutil.h"
#include "clib/threadhelp.h"

namespace Pol::Core
{
namespace
{
struct WatchedThread
{
  std::string name;
  bool reported = false;

  // Written by the watched thread itself without the mutex, since it cannot take a lock from
  // the top of its own loop, and read here under it. Held by pointer so the node keeps its
  // address, and erased only by its own thread, so it cannot be freed while its owner writes.
  // Zero means the thread never checks in and is watched for presence alone.
  std::atomic<polclock_t> deadline{ 0 };

  // The deadline a stall was last announced for, ours alone. A check-in publishes a new one,
  // so this re-arms without the watched thread having to clear anything.
  polclock_t announced_deadline = 0;

  // Testing hook only: judge it late even though no such thread exists, which would otherwise
  // be reported as a death and never as a stall.
  bool assume_present = false;
};

std::mutex watched_mutex;
std::map<size_t, std::unique_ptr<WatchedThread>> watched_threads;
thread_local WatchedThread* my_slot = nullptr;

// How long past its own promise a thread runs before it is called stalled. The window the
// shared clock check used, which in twenty years never fired against a healthy shard -- but
// that demanded all four threads go silent together, so it was a bound on a whole-server
// stall. Per thread it is only ever a bound on a hang: a thread that has *died* is caught by
// the ThreadMap comparison below within the second, with no window at all.
constexpr polclock_t checkin_grace = 30 * POLCLOCKS_PER_SEC;

// No real thread id is 0 or 1, so they can stand for watched threads that are never found.
constexpr size_t absent_thread_id = 0;
constexpr size_t stalled_thread_id = 1;

struct Snapshot
{
  WatchedThreadStatus status;
  ThreadWatchReport report;
};

/// Reads the registry against the ThreadMap: which watched threads are gone, which are past
/// their deadline, and how many are watched. All under one acquisition, because the status
/// dump prints the count beside the thread listing and comparing the two by hand is what
/// catches a thread filed under another thread's id -- which nothing here can see by itself,
/// a wrong but live id looking exactly like health.
///
/// @param announce mark what is returned as reported, so it is named once rather than every
///                 second. The status dump passes false and keeps listing it.
Snapshot take_snapshot( bool announce )
{
  // Copied before taking our own lock, so the two are never held at once.
  threadhelp::ThreadMap::Contents live;
  threadhelp::threadmap_instance().CopyContents( live );

  // A paused clock is not a stalled server. polclock() keeps running through a pause and is
  // corrected on release, so deadlines set before one end up further out afterwards rather
  // than nearer: erring towards saying nothing, which is the right direction for a report
  // that is only useful while it is believed.
  const bool judge_deadlines = is_polclock_paused_at_zero();
  const polclock_t now = polclock();

  Snapshot snap;
  size_t checking_in = 0;
  size_t late = 0;

  std::lock_guard<std::mutex> guard( watched_mutex );
  snap.status.watched = watched_threads.size();
  for ( auto& [tid, thread] : watched_threads )
  {
    if ( live.count( tid ) == 0 && !thread->assume_present )
    {
      snap.status.missing.push_back( thread->name );
      if ( !thread->reported )
      {
        if ( announce )
          thread->reported = true;
        snap.report.stopped.push_back( thread->name );
      }
      // Death is the stronger answer and it has just been given. A thread that has stopped
      // has also stopped checking in, and saying both about the same thread reads as two
      // faults.
      continue;
    }

    const polclock_t deadline = thread->deadline.load( std::memory_order_relaxed );
    if ( !deadline || !judge_deadlines )
      continue;
    ++checking_in;
    if ( !timer_expired( deadline, now ) )
      continue;

    ++late;
    snap.status.overdue.push_back(
        fmt::format( "{} ({}s late)", thread->name, ( now - deadline ) / POLCLOCKS_PER_SEC ) );
    if ( thread->announced_deadline != deadline )
    {
      if ( announce )
        thread->announced_deadline = deadline;
      snap.report.stalled.push_back( thread->name );
    }
  }

  // One thread late among threads that are keeping up is that thread's problem. All of them
  // late together is the world lock's, and saying so once beats naming four innocents.
  snap.report.everything_stalled = late > 1 && late == checking_in;
  return snap;
}
}  // namespace

void watch_this_thread()
{
  // clib publishes the name before handing control to the thread's function. It is capped at
  // 31 characters there, while the thread listing keeps the full string.
  auto slot = std::make_unique<WatchedThread>();
  slot->name = threadhelp::current_thread_name();
  my_slot = slot.get();

  std::lock_guard<std::mutex> guard( watched_mutex );
  watched_threads[threadhelp::thread_pid()] = std::move( slot );
}

void unwatch_this_thread()
{
  my_slot = nullptr;

  std::lock_guard<std::mutex> guard( watched_mutex );
  watched_threads.erase( threadhelp::thread_pid() );
}

void thread_checkin( unsigned sleep_ms )
{
  // Nothing to promise if this thread is not watched -- and nothing is lost, since the slot
  // is claimed at the top of the same function that calls this.
  if ( my_slot == nullptr )
    return;
  my_slot->deadline.store( polclock() + sleep_ms / 10 + checkin_grace, std::memory_order_relaxed );
}

WatchedThreadStatus watched_thread_status()
{
  return take_snapshot( false ).status;
}

ThreadWatchReport take_thread_watch_report()
{
  return take_snapshot( true ).report;
}

void watch_absent_thread_for_testing( std::string name )
{
  auto slot = std::make_unique<WatchedThread>();
  slot->name = std::move( name );

  std::lock_guard<std::mutex> guard( watched_mutex );
  watched_threads[absent_thread_id] = std::move( slot );
}

void watch_stalled_thread_for_testing( std::string name )
{
  auto slot = std::make_unique<WatchedThread>();
  slot->name = std::move( name );
  slot->assume_present = true;
  // A deadline a whole grace period in the past, so the next sweep judges it late.
  slot->deadline.store( polclock() - checkin_grace, std::memory_order_relaxed );

  std::lock_guard<std::mutex> guard( watched_mutex );
  watched_threads[stalled_thread_id] = std::move( slot );
}
}  // namespace Pol::Core
