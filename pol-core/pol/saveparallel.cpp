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
namespace
{
/// The longest run of pieces a worker takes off a part in one claim. A claim is an atomic
/// read-modify-write on a counter every thread hammers, and claiming one piece at a time costs
/// more than formatting one. It decides nothing else: the buffer is handed over between pieces,
/// so how heavy a part's pieces are does not come into it.
constexpr size_t MAX_SAVE_CLAIM = 64;

/// How many pieces of a part a worker takes at a time. A part with few pieces is claimed in
/// shorter runs, or the first worker to reach it walks off with the whole file and the others find
/// nothing left; eight runs per worker is enough for an uneven part to even out between them.
size_t claim_for( size_t count, size_t workers )
{
  return std::clamp( count / ( workers * 8 ), size_t( 1 ), MAX_SAVE_CLAIM );
}

/// Raise `highest` to `value` if it is not already at least that. std::atomic has no max of its
/// own; a failed exchange leaves what it found in `seen`, so the retry compares against whatever
/// the other thread put there.
void raise_to( std::atomic<size_t>& highest, size_t value )
{
  size_t seen = highest.load();
  while ( seen < value )
  {
    if ( highest.compare_exchange_weak( seen, value ) )
      break;
  }
}
}  // namespace

SavePart whole_file_part( std::string name, std::function<void()> write )
{
  return { .name = std::move( name ),
           .count = 1,
           .file = nullptr,
           .format = [write = std::move( write )]( size_t, ChunkOut ) { write(); } };
}

SaveParallelResult write_parallel( const std::vector<SavePart>& parts )
{
  auto& pool = gamestate.task_thread_pool;
  const size_t worker_count = pool.size() + 1;  // the calling thread formats alongside the pool

  std::vector<std::atomic<size_t>> claimed( parts.size() );  // next piece of each part
  std::vector<std::atomic<s64>> format_us( parts.size() );
  std::vector<std::atomic<size_t>> biggest_piece( parts.size() );
  std::vector<std::mutex> locks( parts.size() );  // one per part, guarding its files
  std::atomic<s64> append_us( 0 );
  std::atomic<s64> wait_us( 0 );
  std::atomic<bool> failed( false );
  std::mutex failure_mutex;
  std::exception_ptr failure;

  // Give the file what this worker has formatted for it, and take the buffers back empty.
  // reset_buffer() keeps the capacity, so only the first run of a save allocates.
  auto hand_over = [&append_us, &wait_us]( const SavePart& part, std::mutex& lock,
                                           Clib::StreamWriter& buffer,
                                           Clib::StreamWriter& equip_buffer )
  {
    // A part with no file of its own wrote wherever it wanted to and has nothing to hand over -
    // see whole_file_part.
    if ( part.file != nullptr )
    {
      // Timed either side of the lock, because the two answer different questions: what the
      // writing costs, and what the threads of this part queue behind each other for.
      Tools::HighPerfTimer wait_timer;
      {
        std::lock_guard<std::mutex> guard( lock );
        wait_us += wait_timer.ellapsed().count();
        Tools::HighPerfTimer write_timer;
        part.file->append( buffer.buffer() );
        if ( part.equip != nullptr )
          part.equip->append( equip_buffer.buffer() );
        append_us += write_timer.ellapsed().count();
      }
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
      size_t biggest = 0;  // this worker's heaviest piece of this part
      try
      {
        const size_t claim = claim_for( part.count, worker_count );
        while ( !failed )
        {
          const size_t begin = claimed[p].fetch_add( claim );
          if ( begin >= part.count )
            break;
          took_any = true;
          const size_t end = std::min( begin + claim, part.count );
          // The pieces of this run follow each other in the buffer; the first does not follow the
          // run before it, which some other worker may have claimed and which is not the piece
          // before it anyway.
          bool starts_block = true;
          Tools::HighPerfTimer format_timer;
          for ( size_t piece = begin; piece < end; ++piece )
          {
            const size_t before = buffer.buffer().size() + equip_buffer.buffer().size();
            part.format( piece, ChunkOut{ buffer, equip_buffer, starts_block } );
            starts_block = false;
            const size_t held = buffer.buffer().size();
            const size_t equip_held = equip_buffer.buffer().size();
            // Nothing is handed over inside a piece, so the buffers only grow across one.
            biggest = std::max( biggest, held + equip_held - before );
            // Handed over at exactly the size the file flushes at, so append() writes it
            // straight out instead of copying it first, and between pieces rather than at the end
            // of a run, so heavy pieces cost a worker no more memory than light ones.
            if ( held >= Clib::StreamWriter::FLUSH_THRESHOLD ||
                 equip_held >= Clib::StreamWriter::FLUSH_THRESHOLD )
            {
              format_us[p] += format_timer.ellapsed().count();
              hand_over( part, locks[p], buffer, equip_buffer );
              starts_block = true;  // what follows opens a block of its own
              format_timer = Tools::HighPerfTimer();
            }
          }
          format_us[p] += format_timer.ellapsed().count();
        }
        if ( took_any )  // the tail, and what a part smaller than one run wrote
          hand_over( part, locks[p], buffer, equip_buffer );
        // Once per part per worker rather than per piece, which is what keeps the piece loop off
        // a counter the other threads are on.
        raise_to( biggest_piece[p], biggest );
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

  // The calling thread is a worker too - it has nothing else to do, and it is what makes a save
  // work with an empty pool. Nothing escapes a worker, so joining cannot throw; the first failure
  // is rethrown below.
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
  result.wait_ms = ( wait_us.load() + 500 ) / 1000;
  result.work.reserve( parts.size() );
  for ( size_t p = 0; p < parts.size(); ++p )
    result.work.push_back( { .name = parts[p].name,
                             .elapsed_ms = ( format_us[p].load() + 500 ) / 1000,
                             .pieces = parts[p].count,
                             .biggest_piece = biggest_piece[p].load() } );
  return result;
}
}  // namespace Pol::Core
