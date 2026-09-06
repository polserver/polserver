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
  /// The second file of a part that writes a pair - pcs.txt with pcequip.txt. Unused by the rest.
  Clib::StreamWriter& equip;
  /// This piece opens a block: nothing it can lean on precedes it in the buffer. A worker's runs
  /// are not adjacent pieces, so whatever the loader needs - the StorageArea header, say - has to
  /// be repeated whenever this is set.
  bool starts_block;
};

/// One data file of a save - or a pair written together - as a number of independent pieces plus
/// a way to format any range of them. Pieces reach the file in whatever order the threads finish
/// them, so each has to carry everything the loader needs to place it.
struct SavePart
{
  /// What this part is called in the save report.
  std::string name;
  /// How many independent pieces it has. Zero writes nothing.
  size_t count = 0;
  /// The file its pieces are appended to. Null for a part that writes its own - see
  /// whole_file_part.
  Clib::StreamWriter* file = nullptr;
  /// The second file, for the parts that write a pair. Null for the rest.
  Clib::StreamWriter* equip = nullptr;
  /// Format one piece into `out`. Called on an arbitrary thread, concurrently with every other
  /// piece, so it must touch nothing another piece touches.
  std::function<void( size_t piece, ChunkOut out )> format;
};

/// A part of one piece per element of `objects`, which is the shape of every big file. The part
/// owns the vector, so nothing the caller holds has to outlive the save.
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

/// A file written whole by one thread: accounts.txt, which has no StreamWriter to hand pieces
/// through, and the ones too small to cut up. Nothing else writes to it, so it needs no buffer
/// and streams to disk at its own flush threshold.
SavePart whole_file_part( std::string name, std::function<void()> write );

/// One part of a world save: how much formatting it turned out to be, summed over the threads
/// that shared it.
struct SaveTaskStat
{
  std::string name;
  s64 elapsed_ms = 0;
  /// How many pieces it was cut into. A save cannot finish before its longest piece does, so few
  /// pieces against the width of the pool is where a slow save's long pole hides.
  size_t pieces = 0;
  /// The most text any one piece produced - what a piece count and an average cannot say. Zero
  /// for a part that writes its own file.
  size_t biggest_piece = 0;
};

struct SaveParallelResult
{
  std::vector<SaveTaskStat> work;
  /// Writing text into the files, summed over the threads and measured with the file's lock
  /// held. Files do not wait for each other, so this is an upper bound rather than a share.
  s64 write_ms = 0;
  /// Waiting for a file, summed the same way: what the threads of a part queued behind each other
  /// for. Next to write_ms it says whether a file's single lock costs anything.
  s64 wait_ms = 0;
};

/// Write every file of a save at once, cut into pieces and spread over the task pool as one pool
/// of work, so no single file is the long pole. A worker claims a run of pieces, formats them
/// into a buffer of its own, and hands that buffer to the file under the part's lock once it
/// passes a megabyte - the check falls between pieces, so a worker holds one piece more than the
/// threshold however uneven they are.
///
/// Nothing waits on anything: a worker owes no other worker a turn, so nothing can be stranded.
/// The first piece to throw brings the save down, the others stop at their next claim.
///
/// The calling thread formats alongside the pool and joins it at the end, so a save finishes even
/// on an empty pool.
SaveParallelResult write_parallel( const std::vector<SavePart>& parts );
}  // namespace Pol::Core
