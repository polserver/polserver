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
#include <mutex>
#include <utility>

#include "clib/logfacility.h"
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

/// A range of one part's pieces: what a thread claims, formats and appends in one go.
struct Chunk
{
  size_t part;
  /// Position among its own part's chunks, which is the order its file has to receive them in.
  size_t seq;
  size_t begin;
  size_t end;
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
    size_t seq = 0;
    for ( size_t begin = 0; begin < count; begin += per_chunk )
      chunks.push_back( { p, seq++, begin, std::min( begin + per_chunk, count ) } );
  }
  return chunks;
}

/// What keeps one part's files reading exactly as a single thread would have written them: which
/// of its chunks it is the turn of, and the threads holding the ones after it.
struct Turnstile
{
  std::mutex mutex;
  std::condition_variable changed;
  size_t turn = 0;
};
}  // namespace

SavePart whole_file_part( std::string name, std::function<void()> write )
{
  return { .name = std::move( name ),
           .count = 1,
           .file = nullptr,
           .format = [write = std::move( write )]( size_t, size_t, ChunkOut ) { write(); } };
}

SaveParallelResult write_parallel( const std::vector<SavePart>& parts )
{
  auto& pool = gamestate.task_thread_pool;
  const size_t width = std::max<size_t>( 1, pool.size() );
  const auto chunks = cut_into_chunks( parts, width );

  std::vector<std::atomic<s64>> format_us( parts.size() );
  std::vector<Turnstile> turnstiles( parts.size() );
  std::atomic<s64> append_us( 0 );  // one append is well under a millisecond
  std::atomic<size_t> claimed( 0 );
  std::atomic<bool> failed( false );
  std::mutex failure_mutex;
  std::exception_ptr failure;

  // A chunk that throws leaves its part's turn where it is, so everyone waiting on that part -
  // and on any other part, since the save is over either way - has to be let go. The first
  // failure is the one the save reports; the rest stop where they are.
  auto abort_the_save = [&]( std::exception_ptr reason )
  {
    {
      std::lock_guard<std::mutex> lock( failure_mutex );
      if ( !failure )
        failure = std::move( reason );
    }
    failed = true;
    for ( auto& turnstile : turnstiles )
    {
      // The usual condition_variable rule: a predicate is changed under the lock the threads
      // waiting on it hold, or a thread that read it just before waiting never sees the notify.
      std::lock_guard<std::mutex> lock( turnstile.mutex );
      turnstile.changed.notify_all();
    }
  };

  // Take the next chunk, format it into buffers of one's own, and append it to its files once the
  // chunks before it have been appended. One pair of buffers per thread rather than per chunk, so
  // the memory a save holds on top of the world is a chunk per thread whatever the size of the
  // world, and formatting after the first chunk allocates nothing.
  auto worker = [&]()
  {
    Clib::StreamWriter buffer;
    Clib::StreamWriter equip_buffer;
    while ( !failed )
    {
      const size_t index = claimed++;
      if ( index >= chunks.size() )
        break;
      const auto& chunk = chunks[index];
      const auto& part = parts[chunk.part];
      auto& turnstile = turnstiles[chunk.part];
      try
      {
        buffer.reset_buffer();
        equip_buffer.reset_buffer();
        Tools::HighPerfTimer format_timer;
        part.format( chunk.begin, chunk.end, ChunkOut{ buffer, equip_buffer } );
        format_us[chunk.part] += format_timer.ellapsed().count();

        std::unique_lock<std::mutex> lock( turnstile.mutex );
        turnstile.changed.wait( lock, [&]() { return turnstile.turn == chunk.seq || failed; } );
        if ( failed )
          break;
        Tools::HighPerfTimer append_timer;
        // A part with no file of its own wrote wherever it wanted to and has nothing to hand
        // over - see whole_file_part. It still takes its turn, so the code below reads the same
        // for every part.
        if ( part.file != nullptr )
        {
          part.file->append( buffer.buffer() );
          if ( part.equip != nullptr )
            part.equip->append( equip_buffer.buffer() );
        }
        append_us += append_timer.ellapsed().count();
        ++turnstile.turn;
        lock.unlock();
        turnstile.changed.notify_all();
      }
      catch ( ... )
      {
        // Which file it was, here where that is known. What went wrong is reported by whoever
        // catches the rethrow below, once, rather than by every thread that gives up.
        POLLOG_ERRORLN( "failed to store the {} datafile", part.name );
        abort_the_save( std::current_exception() );
        break;
      }
    }
  };

  // The calling thread is a worker too: it has nothing else to do, and on a wide pool that is a
  // whole thread's worth of formatting. It is also what makes a save possible at all with an
  // empty pool. Nothing escapes a worker, so joining them cannot throw - the first failure is
  // rethrown below instead.
  std::vector<std::future<bool>> workers;
  workers.reserve( pool.size() );
  for ( size_t t = 0; t < pool.size(); ++t )
    workers.push_back( pool.checked_push( worker ) );
  worker();
  for ( auto& running : workers )
    running.wait();
  if ( failure )
    std::rethrow_exception( failure );

  SaveParallelResult result;
  result.write_ms = ( append_us.load() + 500 ) / 1000;
  result.work.reserve( parts.size() );
  for ( size_t p = 0; p < parts.size(); ++p )
    result.work.push_back( { parts[p].name, ( format_us[p].load() + 500 ) / 1000 } );
  return result;
}
}  // namespace Pol::Core
