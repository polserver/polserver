/** @file
 *
 * @par History
 */

#ifndef POL_THREADWATCH_H
#define POL_THREADWATCH_H

#include <cstddef>
#include <string>
#include <vector>

#include "pol/polclock.h"

namespace Pol::Core
{
// The threads a running server expects to have. Two questions are asked of each. Is it still
// in clib's ThreadMap, which catches one that has *died*; and, for the few whose loops run
// unconditionally, is it back by the time it promised, which catches one that is alive but no
// longer getting anywhere. Keyed by thread id -- names are not unique, a shard with three aux
// services has three threads called "AuxService".

/// Starts watching the calling thread, under the name it was started with. Call at the top
/// of the thread's own function.
void watch_this_thread();

/// Stops watching the calling thread. Only for one that ends on purpose -- Decay with
/// nothing left to decay, DbgListn with no debug port. Never a destructor: a thread dying of
/// an exception has to stay watched, or that death is the one thing that goes unreported.
void unwatch_this_thread();

/// Promises that the calling thread will be back within the sleep it is about to take, plus
/// a grace period. Call immediately before sleeping, with the same duration: no one window
/// suits both a 30ms decay pass and a 60-second idle scripts thread, so each thread
/// publishes its own. One that never calls this is watched for presence alone.
void thread_checkin( unsigned sleep_ms );

struct WatchedThreadStatus
{
  size_t watched = 0;
  std::vector<std::string> missing;
  std::vector<std::string> overdue;  // "Name (Ns late)"
};

/// For the thread status dump. Reports without consuming: a thread stays listed for as long
/// as it is missing or late.
WatchedThreadStatus watched_thread_status();

struct ThreadWatchReport
{
  std::vector<std::string> stopped;  // gone from the ThreadMap, named once
  std::vector<std::string> stalled;  // past its deadline, named once per stall
  /// Every thread that checks in is late at the same moment. That has one cause rather than
  /// several: they do their work under the world lock, so whoever holds it stops all of them
  /// together.
  bool everything_stalled = false;
};

/// What has gone wrong since the last call, marking it reported. One call per sweep, so that
/// both answers come from one instant and each is announced once.
ThreadWatchReport take_thread_watch_report();

/// Testing hooks: watch thread ids that cannot exist, one merely absent and one absent and
/// already late, so both detectors can be exercised without stopping a real thread.
/// Reachable through PolCore().internal( 11 ) and ( 12 ).
void watch_absent_thread_for_testing( std::string name );
void watch_stalled_thread_for_testing( std::string name );
}  // namespace Pol::Core

#endif
