/** @file
 *
 * @par History
 */

#include "pol/saveparallel.h"

#include <algorithm>
#include <atomic>
#include <future>
#include <memory>
#include <vector>

#include "clib/streamsaver.h"
#include "clib/timer.h"
#include "pol/globals/uvars.h"

namespace Pol::Core
{
namespace
{
/// Pieces per worker thread, per part. Cutting finer than the thread count is what lets an
/// unlucky split even out over the run instead of deciding the total. It also has a floor: a
/// piece wants to be bigger than a write buffer, so that handing it over is a write rather than
/// a copy into somebody else's buffer.
constexpr size_t CHUNKS_PER_THREAD = 16;

/// A range of one part's pieces, the unit a worker actually takes.
struct Chunk
{
  size_t part;
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
    for ( size_t begin = 0; begin < count; begin += per_chunk )
      chunks.push_back( { p, begin, std::min( begin + per_chunk, count ) } );
  }
  return chunks;
}
}  // namespace

SaveParallelResult write_parallel( const std::vector<SavePart>& parts )
{
  std::vector<std::atomic<s64>> spent( parts.size() );
  // One handover is well under a millisecond, so accumulate finer than that.
  std::atomic<s64> write_us{ 0 };
  auto report = [&]()
  {
    SaveParallelResult result;
    result.write_ms = ( write_us.load() + 500 ) / 1000;
    for ( size_t p = 0; p < parts.size(); ++p )
      result.work.push_back( { parts[p].name, spent[p].load() } );
    return result;
  };

  auto& pool = gamestate.task_thread_pool;
  const size_t width = std::max<size_t>( 1, pool.size() );
  const auto chunks = cut_into_chunks( parts, width );
  if ( chunks.empty() )
    return report();

  // A world too small to be worth the pool still goes through it. The alternative was a second
  // path that formatted straight into the files, and on any test shard that is the path the tests
  // took - which left the one that ships covered only by the unit tests that force the sizes.
  // One path, exercised by everything.
  size_t max_files = 1;
  for ( const auto& part : parts )
    max_files = std::max( max_files, part.writers.size() );

  // Where the next chunk comes from. A worker takes one, formats it, hands it to its files and
  // comes back for another, so a chunk that turns out expensive costs its own thread and nobody
  // else - there is no slot to free up and nothing waiting on this chunk in particular.
  std::atomic<size_t> next{ 0 };

  // The pieces of the ordered parts, kept until every one of them exists. Indexed by chunk, so
  // two threads never touch the same entry and the vector never has to grow.
  std::vector<std::vector<std::unique_ptr<Clib::StreamWriter>>> held( chunks.size() );

  auto work = [&]()
  {
    // This thread's own buffers, reused for every unordered chunk it takes. Detached writers:
    // they have no file, so formatting piles up in them until they are handed over.
    std::vector<std::unique_ptr<Clib::StreamWriter>> buffers;
    for ( size_t f = 0; f < max_files; ++f )
      buffers.push_back( std::make_unique<Clib::StreamWriter>() );
    std::vector<Clib::StreamWriter*> out;

    for ( size_t i = next.fetch_add( 1 ); i < chunks.size(); i = next.fetch_add( 1 ) )
    {
      const auto& chunk = chunks[i];
      const auto& part = parts[chunk.part];
      out.clear();
      if ( part.ordered )
      {
        // Its own buffers rather than this thread's, because they outlive the chunk: nothing is
        // written until the whole part is formatted.
        for ( size_t f = 0; f < part.writers.size(); ++f )
        {
          held[i].push_back( std::make_unique<Clib::StreamWriter>() );
          out.push_back( held[i].back().get() );
        }
      }
      else
      {
        for ( size_t f = 0; f < part.writers.size(); ++f )
        {
          buffers[f]->reset_buffer();
          out.push_back( buffers[f].get() );
        }
      }

      Tools::Timer<> timer;
      part.format( chunk.begin, chunk.end, out );
      spent[chunk.part] += timer.ellapsed();

      if ( part.ordered )
        continue;  // written below, once the rest of its file has been formatted

      Tools::HighPerfTimer write_timer;
      for ( size_t f = 0; f < part.writers.size(); ++f )
        part.writers[f]->append_locked( buffers[f]->buffer() );
      write_us += write_timer.ellapsed().count();
    }
  };

  // The calling thread takes chunks too rather than watching the pool do it: there is nothing for
  // it to coordinate, and on a wide pool an idle thread is a whole thread's worth of formatting.
  std::vector<std::future<bool>> workers;
  workers.reserve( width );
  for ( size_t w = 0; w < width; ++w )
    workers.push_back( pool.checked_push( work ) );

  std::exception_ptr failure;
  try
  {
    work();
  }
  catch ( ... )
  {
    failure = std::current_exception();
  }

  // Every worker holds references to the parts, to the chunk list and to whatever those captured,
  // all of which is about to go out of scope, so none of this may be skipped on the way out. A
  // future backed by a promise does not wait in its destructor.
  for ( auto& worker : workers )
  {
    try
    {
      worker.get();
    }
    catch ( ... )
    {
      if ( !failure )
        failure = std::current_exception();
    }
  }
  if ( failure )
    std::rethrow_exception( failure );

  // The ordered parts, now that every piece of them exists. cut_into_chunks emits a part's chunks
  // in piece order, so walking the chunk list gives each file its pieces in the order one thread
  // would have written them. No lock: the workers are done.
  Tools::HighPerfTimer assemble_timer;
  for ( size_t i = 0; i < chunks.size(); ++i )
  {
    const auto& part = parts[chunks[i].part];
    if ( !part.ordered )
      continue;
    for ( size_t f = 0; f < part.writers.size(); ++f )
      part.writers[f]->append( held[i][f]->buffer() );
    held[i].clear();  // a large file is not worth holding a moment longer than it has to be
  }
  write_us += assemble_timer.ellapsed().count();

  return report();
}
}  // namespace Pol::Core
