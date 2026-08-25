/** @file
 *
 * @par History
 */

#include "pol/saveparallel.h"

#include <algorithm>
#include <atomic>
#include <future>
#include <memory>

#include "clib/streamsaver.h"
#include "clib/timer.h"
#include "pol/globals/uvars.h"

namespace Pol::Core
{
namespace
{
/// Pieces per worker thread, per part. Cutting finer than the thread count is what lets an
/// unlucky split even out over the run instead of deciding the total.
constexpr size_t CHUNKS_PER_THREAD = 16;
/// Pieces formatted but not yet written, at most. This bounds the memory a save holds on top of
/// the world: the buffers of the chunks in flight, and nothing that grows with the file.
constexpr size_t WINDOW_PER_THREAD = 2;

/// A range of one part's pieces, the unit the pool actually schedules.
struct Chunk
{
  size_t part;
  size_t begin;
  size_t end;
};

/// A slot in the sliding window: buffers for one chunk in flight.
struct Slot
{
  std::vector<std::unique_ptr<Clib::StreamWriter>> buffers;
  /// The subset of `buffers` this chunk's part writes to, which is what format() is handed.
  std::vector<Clib::StreamWriter*> out;
  std::future<bool> running;
};

std::vector<Chunk> cut_into_chunks( const std::vector<SavePart>& parts, size_t width )
{
  std::vector<Chunk> chunks;
  for ( size_t p = 0; p < parts.size(); ++p )
  {
    const size_t count = parts[p].count;
    if ( !count )
      continue;
    const size_t wanted = std::min( count, width * CHUNKS_PER_THREAD );
    const size_t per_chunk = ( count + wanted - 1 ) / wanted;
    for ( size_t begin = 0; begin < count; begin += per_chunk )
      chunks.push_back( { p, begin, std::min( begin + per_chunk, count ) } );
  }
  return chunks;
}
}  // namespace

SaveParallelResult write_parallel( const std::vector<SavePart>& parts )
{
  std::vector<std::atomic<s64>> spent( parts.size() );
  s64 write_us = 0;  // one append is well under a millisecond, so accumulate finer than that
  auto report = [&]()
  {
    SaveParallelResult result;
    result.write_ms = ( write_us + 500 ) / 1000;
    for ( size_t p = 0; p < parts.size(); ++p )
      result.work.push_back( { parts[p].name, spent[p].load() } );
    return result;
  };

  auto& pool = gamestate.task_thread_pool;
  // Never zero: window is a modulus below, and a save with nothing to write still has to reach
  // the end of this function rather than divide by it.
  const size_t width = std::max<size_t>( 1, pool.size() );
  const auto chunks = cut_into_chunks( parts, width );
  if ( chunks.empty() )
    return report();

  // A world too small to be worth the pool still goes through it. The alternative was a second
  // path that formatted straight into the files, and on any test shard that is the path the
  // tests took - which left the one that ships covered only by the unit tests that force the
  // sizes. One path, exercised by everything.
  const size_t window = std::min( chunks.size(), width * WINDOW_PER_THREAD );
  size_t max_files = 1;
  for ( const auto& part : parts )
    max_files = std::max( max_files, part.writers.size() );

  std::vector<Slot> slots( window );
  for ( auto& slot : slots )
    for ( size_t f = 0; f < max_files; ++f )
      slot.buffers.push_back( std::make_unique<Clib::StreamWriter>() );

  auto launch = [&]( size_t slot_index, size_t chunk_index )
  {
    auto& slot = slots[slot_index];
    const auto& chunk = chunks[chunk_index];
    const auto& part = parts[chunk.part];
    slot.out.clear();
    for ( size_t f = 0; f < part.writers.size(); ++f )
    {
      slot.buffers[f]->reset_buffer();
      slot.out.push_back( slot.buffers[f].get() );
    }
    slot.running = pool.checked_push(
        [&part, &slot, &spent, chunk]()
        {
          Tools::Timer<> timer;
          part.format( chunk.begin, chunk.end, slot.out );
          spent[chunk.part] += timer.ellapsed();
        } );
  };

  // Leaving early is not an option while chunks are still in flight: they hold references to the
  // slots, to the parts and to whatever those captured, all of which is about to go out of scope.
  // A future backed by a promise does not wait in its destructor, so wait here.
  auto drain = [&slots]()
  {
    for ( auto& slot : slots )
    {
      if ( !slot.running.valid() )
        continue;
      try
      {
        slot.running.get();
      }
      catch ( ... )
      {
      }
    }
  };

  // A file has to receive its chunks in order, so this thread takes them in order: it waits for
  // the chunk at the head of the window, hands its buffers to the files, and puts the next chunk
  // into the slot that just came free. Waiting on the head is also what bounds the memory a save
  // holds: `window` chunks, whatever the size of the world.
  size_t next = 0;
  while ( next < window )
  {
    launch( next, next );
    ++next;
  }

  try
  {
    for ( size_t head = 0; head < chunks.size(); ++head )
    {
      auto& slot = slots[head % window];
      slot.running.get();  // rethrows whatever the chunk failed with
      const auto& part = parts[chunks[head].part];
      Tools::HighPerfTimer write_timer;
      for ( size_t f = 0; f < part.writers.size(); ++f )
        part.writers[f]->append( slot.buffers[f]->buffer() );
      write_us += write_timer.ellapsed().count();
      if ( next < chunks.size() )
      {
        launch( head % window, next );
        ++next;
      }
    }
  }
  catch ( ... )
  {
    drain();
    throw;
  }

  return report();
}
}  // namespace Pol::Core
