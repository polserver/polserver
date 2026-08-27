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
};

/// One data file of a save - or a pair of them written together, like pcs.txt and pcequip.txt -
/// expressed as a number of independent pieces plus a way to format any range of them.
struct SavePart
{
  /// What this part is called in the save report.
  std::string name;
  /// How many independent pieces it has. Zero is allowed and writes nothing.
  size_t count = 0;
  /// The file its pieces are appended to, in piece order. Null for a part that writes its own
  /// file directly and has nothing to hand back - see whole_file_part.
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
                      Clib::StreamWriter* equip, WriteOne write_one )
{
  const size_t count = objects.size();
  return { .name = std::move( name ),
           .count = count,
           .file = file,
           .equip = equip,
           .format =
               [objects = std::move( objects ), write_one]( size_t begin, size_t end, ChunkOut out )
           {
             for ( size_t i = begin; i < end; ++i )
               write_one( objects[i], out );
           } };
}

/// A file small enough not to be worth cutting up: one piece, formatted on whichever thread claims
/// it, straight into the file. Nothing else writes to that file, so there is no order to keep and
/// no reason to stage the text in a buffer first - which also leaves the file streaming to disk at
/// its own flush threshold instead of piling up in memory while the world is stopped.
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
  /// takes its pieces one at a time, but different files do not wait for each other, so this is
  /// an upper bound on what the writing added to the save rather than a share of it.
  s64 write_ms = 0;
};

/// Write every file of a save at once, cut into pieces and spread over the task thread pool as a
/// single pool of work.
///
/// A save used to get one thread per file, so the whole thing waited for whichever file was
/// biggest - and on a large shard one file is most of the save. Here the pieces of every file
/// compete for the same threads, so no file is the long pole, and each file still receives its
/// pieces in order: the bytes are exactly what a single thread would have written.
///
/// The calling thread formats alongside the pool and joins it at the end, so a save finishes even
/// if the pool is busy with something else - or empty, on that thread alone. Calling this from a
/// pool thread works for the same reason, but ties one up for the length of the save.
///
/// The first piece to throw brings the save down: the rest stop where they are and its exception
/// reaches the caller.
SaveParallelResult write_parallel( const std::vector<SavePart>& parts );
}  // namespace Pol::Core
