/** @file
 *
 * @par History
 */

#ifndef POL_THREADWATCH_H
#define POL_THREADWATCH_H

#include <cstddef>
#include <string>
#include <vector>

namespace Pol::Core
{
// The threads a running server expects to have. A thread is watched by asking whether it is
// still in clib's ThreadMap, so this detects one that has *died*, not one that is stuck:
// most of them block indefinitely when idle, where no progress and no work look alike.
// Keyed by thread id -- names are not unique, a shard with three aux services has three
// threads called "AuxService".

/// Starts watching the calling thread, under the name it was started with. Call at the top
/// of the thread's own function.
void watch_this_thread();

/// Stops watching the calling thread. Only for one that ends on purpose -- Decay with
/// nothing left to decay, DbgListn with no debug port. Never a destructor: a thread dying of
/// an exception has to stay watched, or that death is the one thing that goes unreported.
void unwatch_this_thread();

struct WatchedThreadStatus
{
  size_t watched = 0;
  std::vector<std::string> missing;
};

/// For the thread status dump.
WatchedThreadStatus watched_thread_status();

/// Stopped threads not yet reported, marking them reported: the log line fires once per
/// death, while the dump above keeps listing them.
std::vector<std::string> take_unreported_stopped_threads();

/// Testing hook: watch a thread id that cannot exist, so the detector can be exercised
/// without stopping a real thread. Reachable through PolCore().internal( 11 ).
void watch_absent_thread_for_testing( std::string name );
}  // namespace Pol::Core

#endif
