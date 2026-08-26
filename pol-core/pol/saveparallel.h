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
///
/// Unless `ordered` says otherwise, the pieces are formatted on whatever thread picks them up and
/// appended as soon as they are done, so a file assembled that way holds the same blocks every
/// time but not in the same order, and not in the order one thread would have written them. Three
/// files can be written that way today and each for its own reason:
///
///   - storage.txt: Storage::read files each item under the last StorageArea element it saw, and
///     StorageArea::_items is a map keyed by name, so nothing downstream can tell what order the
///     items arrived in. A piece has to re-open its area - see Storage::print_unit.
///   - pcs.txt/pcequip.txt and npcs.txt/npcequip.txt: written by walking the object hash, whose
///     iteration order is an implementation detail already. Equipment finds its owner by serial,
///     and pcs.txt is read before pcequip.txt, so neither file constrains the other.
///
/// items.txt does NOT qualify: its file order becomes zone-vector order on load, which is the
/// order every WorldIterator hands to scripts. It sets `ordered` instead - see below - so it is
/// still formatted on every thread, just assembled afterwards. multis.txt is small enough that
/// one plain task covers it.
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
  /// This file's blocks have to come out in piece order.
  ///
  /// Its pieces are still formatted on every thread; they are just kept in memory until the last
  /// of them is done and then written in one pass, in order. That costs the whole file in memory
  /// for the length of the save, which is why it is not how every file is written - but for a
  /// file that fits it is far simpler than handing pieces over in order as they finish, and it
  /// leaves no thread waiting for its turn.
  bool ordered = false;
};

/// How much formatting one part turned out to be, summed over the threads that shared it.
using SaveWork = std::vector<std::pair<std::string, s64>>;

struct SaveParallelResult
{
  SaveWork work;
  /// Time spent handing formatted text to the files, summed over the threads that did it. The
  /// files serialise it a file at a time, so a figure approaching the length of the save means
  /// the disk, not the formatting, is what a bigger pool would be waiting for.
  s64 write_ms = 0;
};

/// Write several save files at once: their pieces are cut up and spread over the task thread pool
/// as one pool of work.
///
/// A save used to get one thread per file, so the whole thing waited for whichever file was
/// biggest - and on a large shard one file is most of the save. Here the pieces of every file
/// compete for the same threads, so no file is the long pole and no thread is left idle waiting
/// for a particular piece.
///
/// Runs on the calling thread, which takes pieces alongside the pool. Never call it from a pool
/// thread: it waits on pool work, and a pool waiting on itself can run out of threads to make
/// progress with.
SaveParallelResult write_parallel( const std::vector<SavePart>& parts );
}  // namespace Pol::Core
