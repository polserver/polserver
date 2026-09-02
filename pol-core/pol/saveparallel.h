/** @file
 *
 * @par History
 */

#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "clib/rawtypes.h"

namespace Pol::Clib
{
class StreamWriter;
}

namespace Pol::Core
{
/// Pieces a worker takes off a part in one claim, when the part does not say otherwise.
///
/// The claim is an atomic read-modify-write on a counter every thread is hammering, so the
/// cacheline it lives on has to travel between cores once per claim and those trips cannot
/// overlap. Claiming one piece at a time would spend more on that than on the formatting of a
/// piece; a run of this length pushes it out of the picture.
constexpr size_t DEFAULT_SAVE_CLAIM = 64;

/// Where one piece of a part formats to: a buffer per file the part writes.
struct ChunkOut
{
  Clib::StreamWriter& file;
  /// The second file of a part that writes a pair - pcs.txt with pcequip.txt, npcs.txt with
  /// npcequip.txt. The parts that write a single file leave it alone.
  Clib::StreamWriter& equip;
};

/// One data file of a save - or a pair of them written together, like pcs.txt and pcequip.txt -
/// expressed as a number of independent pieces plus a way to format any range of them.
///
/// A part's pieces reach its file in whatever order the threads happen to finish them, so each
/// piece has to carry everything the loader needs to place it. Where that is not free the piece
/// says so itself: a container writes its whole subtree, and a storage piece repeats the
/// StorageArea header that tells the loader which area it belongs to.
struct SavePart
{
  /// What this part is called in the save report.
  std::string name;
  /// How many independent pieces it has. Zero is allowed and writes nothing.
  size_t count = 0;
  /// How many pieces a worker takes at a time, which a part sizes to how heavy its pieces are.
  ///
  /// A run is formatted whole before any of it is handed over - format() is opaque to the
  /// scheduler, so it cannot be interrupted partway - which makes this what bounds a worker's
  /// buffer. A part whose piece is a bank box wants far fewer of them at a time than one whose
  /// piece is a single item.
  size_t claim = DEFAULT_SAVE_CLAIM;
  /// The file its pieces are appended to. Null for a part that writes its own file directly and
  /// hands nothing back - see whole_file_part.
  Clib::StreamWriter* file = nullptr;
  /// The second file, for the parts that write a pair. Null for the rest.
  Clib::StreamWriter* equip = nullptr;
  /// Format pieces [begin, end) into `out`.
  ///
  /// Called on an arbitrary thread, concurrently with the other ranges of this part and of every
  /// other part, so it must touch nothing that another range touches.
  std::function<void( size_t begin, size_t end, ChunkOut out )> format;
};

/// A part made of one indivisible piece per element of `objects`, which is the shape of every big
/// file: `write_one` gets one element and the buffers to write it into. The part owns the vector,
/// so nothing the caller holds has to outlive the save.
template <typename T, typename WriteOne>
SavePart object_part( std::string name, std::vector<T> objects, Clib::StreamWriter* file,
                      Clib::StreamWriter* equip, WriteOne write_one,
                      size_t claim = DEFAULT_SAVE_CLAIM )
{
  const size_t count = objects.size();
  return { .name = std::move( name ),
           .count = count,
           .claim = claim,
           .file = file,
           .equip = equip,
           .format =
               [objects = std::move( objects ), write_one]( size_t begin, size_t end, ChunkOut out )
           {
             for ( size_t i = begin; i < end; ++i )
               write_one( objects[i], out );
           } };
}

/// A file that is written whole by one thread: either because it has no StreamWriter to hand
/// pieces through, which is accounts.txt and its own writing path, or because it is too small to
/// be worth cutting up. One piece, claimed by whichever thread reaches it first, writing straight
/// into the file while the other threads carry on with the rest of the save.
///
/// Nothing else writes to that file, so there is no order to keep between threads and no reason
/// to stage the text in a buffer first - which also leaves the file streaming to disk at its own
/// flush threshold instead of piling up in memory while the world is stopped.
SavePart whole_file_part( std::string name, std::function<void()> write );

/// One part of a world save: how much formatting it turned out to be, summed over the threads
/// that shared it.
struct SaveTaskStat
{
  std::string name;
  s64 elapsed_ms;
};

struct SaveParallelResult
{
  std::vector<SaveTaskStat> work;
  /// Time spent handing formatted text to the files, summed over the threads that did it. A file
  /// takes one block at a time, but different files do not wait for each other, so this is an
  /// upper bound on what the writing added to the save rather than a share of it.
  s64 write_ms = 0;
};

/// Write every file of a save at once, cut into pieces and spread over the task thread pool as a
/// single pool of work.
///
/// Every part offers its pieces to the same threads, so no one file is the long pole. A worker
/// claims a run of pieces off the part's counter, formats them into a buffer of its own, and
/// hands that buffer to the file under the part's lock once it has grown past a megabyte. It
/// holds one buffer pair whatever the size of the world, and formatting after the first run
/// allocates nothing.
///
/// Nothing waits on anything here: a worker that has claimed a run owes no other worker a turn,
/// which is what keeps a failure from stranding the rest. The first piece to throw brings the
/// save down - the others stop at their next claim and its exception reaches the caller.
///
/// The calling thread formats alongside the pool and joins it at the end, so a save finishes even
/// if the pool is busy with something else - or empty, on that thread alone. Calling this from a
/// pool thread works for the same reason, but ties one up for the length of the save.
SaveParallelResult write_parallel( const std::vector<SavePart>& parts );
}  // namespace Pol::Core
