/** @file
 *
 * @par History
 */

#include "pol/saveparallel.h"

#include <algorithm>
#include <atomic>
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

  std::vector<std::atomic<size_t>> claimed( parts.size() );  // next piece of each part
  std::vector<std::atomic<s64>> format_us( parts.size() );
  std::vector<std::mutex> locks( parts.size() );  // one per part, guarding its files
  std::atomic<s64> append_us( 0 );
  std::atomic<bool> failed( false );
  std::mutex failure_mutex;
  std::exception_ptr failure;

  // Give the file what this worker has formatted for it, and take the buffers back empty.
  // reset_buffer() keeps the capacity, so only the first run of a save allocates.
  auto hand_over = [&append_us]( const SavePart& part, std::mutex& lock, Clib::StreamWriter& buffer,
                                 Clib::StreamWriter& equip_buffer )
  {
    // A part with no file of its own wrote wherever it wanted to and has nothing to hand over -
    // see whole_file_part.
    if ( part.file != nullptr )
    {
      Tools::HighPerfTimer append_timer;
      {
        std::lock_guard<std::mutex> guard( lock );
        part.file->append( buffer.buffer() );
        if ( part.equip != nullptr )
          part.equip->append( equip_buffer.buffer() );
      }
      append_us += append_timer.ellapsed().count();
    }
    buffer.reset_buffer();
    equip_buffer.reset_buffer();
  };

  // Walk the parts in order, taking runs off whichever one still has pieces left. Every worker
  // walks the same order, so they stay on the same part as each other and one buffer pair covers
  // a worker for the whole save rather than one per file it might touch.
  auto worker = [&]()
  {
    Clib::StreamWriter buffer;
    Clib::StreamWriter equip_buffer;
    for ( size_t p = 0; p < parts.size() && !failed; ++p )
    {
      const auto& part = parts[p];
      if ( !part.count )
        continue;
      bool took_any = false;
      try
      {
        while ( !failed )
        {
          const size_t begin = claimed[p].fetch_add( part.claim );
          if ( begin >= part.count )
            break;
          took_any = true;
          Tools::HighPerfTimer format_timer;
          part.format( begin, std::min( begin + part.claim, part.count ),
                       ChunkOut{ buffer, equip_buffer } );
          format_us[p] += format_timer.ellapsed().count();
          // Handed over at exactly the size the file flushes at, so append() writes the block
          // straight out instead of copying it into a buffer that is about to be written anyway.
          if ( buffer.buffer().size() >= Clib::StreamWriter::FLUSH_THRESHOLD ||
               equip_buffer.buffer().size() >= Clib::StreamWriter::FLUSH_THRESHOLD )
            hand_over( part, locks[p], buffer, equip_buffer );
        }
        if ( took_any )  // the tail, and what a part smaller than one run wrote
          hand_over( part, locks[p], buffer, equip_buffer );
      }
      catch ( ... )
      {
        // Which file it was, here where that is known. What went wrong is reported by whoever
        // catches the rethrow below, once, rather than by every thread that gives up.
        POLLOG_ERRORLN( "failed to store the {} datafile", part.name );
        {
          std::lock_guard<std::mutex> guard( failure_mutex );
          if ( !failure )
            failure = std::current_exception();
        }
        failed = true;
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
