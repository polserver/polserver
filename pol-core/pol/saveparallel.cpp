/** @file
 *
 * @par History
 */

#include "pol/saveparallel.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <exception>
#include <future>
#include <memory>
#include <mutex>
#include <thread>

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
  const size_t width = pool.size();
  size_t total = 0;
  for ( const auto& part : parts )
    total += part.count;
  const auto chunks = cut_into_chunks( parts, std::max<size_t>( 1, width ) );

  // Below a certain amount of work the pool costs more than it saves.
  if ( width < 2 || total < width * CHUNKS_PER_THREAD || chunks.size() < 2 )
  {
    for ( const auto& chunk : chunks )
    {
      Tools::Timer<> timer;
      parts[chunk.part].format( chunk.begin, chunk.end, parts[chunk.part].writers );
      spent[chunk.part] += timer.ellapsed();
    }
    // Nothing was buffered, so the writing happened inside the formatting above.
    return report();
  }

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

  // A file has to receive its chunks in order, but there is no reason the thread that waits for
  // them should also be the one writing them. With both jobs on one thread the two never fully
  // overlap, and on a large shard each of them is about half the save. So: this thread launches
  // chunks, and a writer thread takes each finished one in turn and hands it to its file.
  struct Pipeline
  {
    std::mutex mutex;
    std::condition_variable progress;
    /// Chunks handed to the pool, and chunks written. The writer may not touch a chunk before it
    /// is launched, and a slot may not be relaunched before its last chunk is written.
    size_t launched = 0;
    size_t written = 0;
    bool failed = false;
    std::exception_ptr failure;
  } pipe;

  // Only the writer thread touches write_us, and only before it is joined.
  auto write_finished_chunks = [&]()
  {
    for ( size_t head = 0; head < chunks.size(); ++head )
    {
      try
      {
        {
          // The chunk has to exist before it can be waited on: its future is only put in the slot
          // when it is launched, and the slot still holds the spent future of the chunk before.
          std::unique_lock<std::mutex> lock( pipe.mutex );
          pipe.progress.wait( lock, [&]() { return pipe.launched > head || pipe.failed; } );
          if ( pipe.failed )
            return;
        }
        auto& slot = slots[head % window];
        slot.running.get();  // rethrows whatever a chunk failed with
        const auto& part = parts[chunks[head].part];
        Tools::HighPerfTimer write_timer;
        for ( size_t f = 0; f < part.writers.size(); ++f )
          part.writers[f]->append( slot.buffers[f]->buffer() );
        write_us += write_timer.ellapsed().count();
      }
      catch ( ... )
      {
        std::lock_guard<std::mutex> lock( pipe.mutex );
        pipe.failed = true;
        if ( !pipe.failure )
          pipe.failure = std::current_exception();
        pipe.progress.notify_all();
        return;  // the save has failed, there is nothing to write any more
      }
      {
        std::lock_guard<std::mutex> lock( pipe.mutex );
        ++pipe.written;
      }
      pipe.progress.notify_all();
    }
  };

  size_t next = 0;
  while ( next < window )
  {
    launch( next, next );
    ++next;
  }
  pipe.launched = next;  // nothing else is running yet

  // Joined explicitly on every path below rather than left to the destructor: drain() may only
  // run once the writer has stopped, as both call get() on the same futures, and the destructor
  // would fire after it.
  std::jthread writer;
  try
  {
    writer = std::jthread( write_finished_chunks );
    // Slot `next % window` last held chunk `next - window`, so it is free to reuse as soon as the
    // writer has dealt with that one. Waiting on that is what bounds the memory a save holds:
    // `window` chunks, whatever the size of the world.
    while ( next < chunks.size() )
    {
      {
        std::unique_lock<std::mutex> lock( pipe.mutex );
        pipe.progress.wait( lock, [&]() { return pipe.written + window > next || pipe.failed; } );
        if ( pipe.failed )
          break;
      }
      launch( next % window, next );
      ++next;
      {
        std::lock_guard<std::mutex> lock( pipe.mutex );
        pipe.launched = next;
      }
      pipe.progress.notify_all();
    }
  }
  catch ( ... )
  {
    // The writer may be waiting for a chunk that will now never be launched.
    {
      std::lock_guard<std::mutex> lock( pipe.mutex );
      pipe.failed = true;
    }
    pipe.progress.notify_all();
    if ( writer.joinable() )
      writer.join();
    drain();
    throw;
  }
  writer.join();
  // Chunks the writer never got to, because an earlier one failed.
  drain();
  if ( pipe.failure )
    std::rethrow_exception( pipe.failure );

  return report();
}
}  // namespace Pol::Core
