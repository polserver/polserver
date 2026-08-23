/** @file
 *
 * @par History
 */

#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "clib/rawtypes.h"

namespace Pol::Clib
{
class StreamWriter;
}

namespace Pol::Core
{
/// One data file of a save - or a pair of them written together, like pcs.txt and pcequip.txt -
/// expressed as a number of independent pieces plus a way to format any range of them.
struct SavePart
{
  /// What this part is called in the save report.
  std::string name;
  /// How many independent pieces it has. Zero is allowed and writes nothing.
  size_t count = 0;
  /// The files its pieces write to, in the order `format` receives them.
  std::vector<Clib::StreamWriter*> writers;
  /// Format pieces [begin, end) into `out`, which holds one writer per entry of `writers`.
  ///
  /// Called on an arbitrary thread, concurrently with the other ranges of this part and of every
  /// other part, so it must touch nothing that another range touches.
  std::function<void( size_t begin, size_t end, const std::vector<Clib::StreamWriter*>& out )>
      format;
};

/// How much formatting one part turned out to be, summed over the threads that shared it.
using SaveWork = std::vector<std::pair<std::string, s64>>;

struct SaveParallelResult
{
  SaveWork work;
  /// Time the calling thread spent handing formatted text to the files. It is serial, so it
  /// counts against the length of the save directly however many threads are formatting.
  s64 write_ms = 0;
};

/// Write several save files at once, cut into pieces and spread over the task thread pool as a
/// single pool of work.
///
/// A save used to get one thread per file, so the whole thing waited for whichever file was
/// biggest - and on a large shard one file is most of the save. Here the pieces of every file
/// compete for the same threads, so no file is the long pole, and each file still receives its
/// pieces in order: the bytes are exactly what a single thread would have written.
///
/// Runs on the calling thread, which does the writing while the pool formats what comes next.
/// Never call it from a pool thread - it waits on pool work, and a pool waiting on itself can
/// run out of threads to make progress with.
SaveParallelResult write_parallel( const std::vector<SavePart>& parts );
}  // namespace Pol::Core
