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
/// Where one piece of a part formats to: a buffer per file the part writes.
struct ChunkOut
{
  Clib::StreamWriter& file;
  /// The second file of a part that writes a pair - pcs.txt with pcequip.txt, npcs.txt with
  /// npcequip.txt. The parts that write a single file leave it alone.
  Clib::StreamWriter& equip;
  /// Nothing this piece can lean on precedes it in the buffer: it is the first piece of a claimed
  /// run, or the first since the buffer was handed to the file. A worker's buffer holds the runs
  /// it happened to claim, which are not adjacent pieces, so this is the only thing a piece may
  /// read anything into. A piece that has to tell the loader something - which storage area it
  /// belongs to, say - says it whenever this is set.
  bool starts_block;
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
  /// The file its pieces are appended to. Null for a part that writes its own file directly and
  /// hands nothing back - see whole_file_part.
  Clib::StreamWriter* file = nullptr;
  /// The second file, for the parts that write a pair. Null for the rest.
  Clib::StreamWriter* equip = nullptr;
  /// Format one piece into `out`.
  ///
  /// Called on an arbitrary thread, concurrently with the other pieces of this part and of every
  /// other part, so it must touch nothing another piece touches. A piece is where the scheduler
  /// is free to hand the buffer to the file, so what a piece may assume about the text in front
  /// of it is exactly `out.starts_block`.
  std::function<void( size_t piece, ChunkOut out )> format;
};

/// A part made of one indivisible piece per element of `objects`, which is the shape of every big
/// file: `write_one` gets one element and the buffers to write it into. The part owns the vector,
/// so nothing the caller holds has to outlive the save.
template <typename T, typename WriteOne>
SavePart object_part( std::string name, std::vector<T> objects, Clib::StreamWriter* file,
                      Clib::StreamWriter* equip, WriteOne write_one )
{
  const size_t count = objects.size();
  return { .name = std::move( name ),
           .count = count,
           .file = file,
           .equip = equip,
           .format = [objects = std::move( objects ), write_one]( size_t piece, ChunkOut out )
           { write_one( objects[piece], out ); } };
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
  s64 elapsed_ms = 0;
  /// How many pieces it was cut into. A save cannot finish before its longest single piece does,
  /// so a part with few pieces against the width of the pool is where a slow save's long pole
  /// hides; with the bytes its files received, this also says how heavy a piece of it is.
  size_t pieces = 0;
  /// The most text any one piece of it produced. A part of many small pieces and one enormous one
  /// finishes when that one does, which is what the piece count and an average over it cannot
  /// say. Zero for a part that writes its own file, since nothing it produces passes through a
  /// buffer to be measured.
  size_t biggest_piece = 0;
};

struct SaveParallelResult
{
  std::vector<SaveTaskStat> work;
  /// Time spent writing formatted text into the files, summed over the threads that did it and
  /// measured with the file's lock already held. Different files do not wait for each other, so
  /// this is an upper bound on what the writing added to the save rather than a share of it.
  s64 write_ms = 0;
  /// Time spent waiting for a file, summed the same way: one file takes one block at a time, so
  /// this is what the threads of a part queued behind each other for. Next to write_ms it says
  /// whether a file's single lock is costing the save anything.
  s64 wait_ms = 0;
};

/// Write every file of a save at once, cut into pieces and spread over the task thread pool as a
/// single pool of work.
///
/// Every part offers its pieces to the same threads, so no one file is the long pole. A worker
/// claims a run of pieces off the part's counter, formats them one at a time into a buffer of its
/// own, and hands that buffer to the file under the part's lock once it has grown past a
/// megabyte. The check falls between pieces, so a worker holds one piece more than the threshold
/// however uneven the pieces of a part are, whatever the size of the world - and formatting after
/// the first megabyte allocates nothing.
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
