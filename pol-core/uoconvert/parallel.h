#ifndef POL_UOCONVERT_PARALLEL_H
#define POL_UOCONVERT_PARALLEL_H

#include <algorithm>
#include <cstddef>
#include <exception>
#include <mutex>
#include <thread>
#include <vector>

namespace Pol::UoConvert
{
// Fork-join parallel for-each over [0, n). Splits the range into contiguous
// chunks, one per worker thread, and calls body(i) for every i in the chunk.
// Joins before returning, so the call itself acts as a barrier.
//
// - max_threads == 0 selects std::thread::hardware_concurrency() (floored to 1).
// - Runs inline on the calling thread (no threads spawned) when the resolved
//   worker count is 1 or n <= 1 -- this is the serial/verification path.
// - Chunks are contiguous, so each worker streams a contiguous slice of any
//   row-major array indexed by i (cache-friendly for the terrain-plane passes
//   and the block rows).
// - If body throws, the first exception is captured and rethrown on the calling
//   thread after all workers join (remaining indices in other chunks may still
//   run to completion first).
//
// body(i) is invoked concurrently for distinct i, so it must not mutate shared
// state without synchronization.
template <typename Body>
void parallel_for( std::size_t n, Body&& body, unsigned max_threads = 0 )
{
  if ( n == 0 )
    return;

  unsigned hw = max_threads != 0 ? max_threads : std::thread::hardware_concurrency();
  if ( hw < 1 )
    hw = 1;
  const std::size_t workers = std::min<std::size_t>( hw, n );

  if ( workers <= 1 )
  {
    for ( std::size_t i = 0; i < n; ++i )
      body( i );
    return;
  }

  std::exception_ptr eptr;
  std::mutex eptr_mtx;
  auto run_range = [&]( std::size_t begin, std::size_t end )
  {
    try
    {
      for ( std::size_t i = begin; i < end; ++i )
        body( i );
    }
    catch ( ... )
    {
      std::lock_guard<std::mutex> lk( eptr_mtx );
      if ( !eptr )
        eptr = std::current_exception();
    }
  };

  const std::size_t chunk = ( n + workers - 1 ) / workers;  // ceil division
  std::vector<std::thread> threads;
  threads.reserve( workers - 1 );
  for ( std::size_t w = 1; w < workers; ++w )
  {
    const std::size_t begin = w * chunk;
    if ( begin >= n )
      break;
    const std::size_t end = std::min( begin + chunk, n );
    threads.emplace_back( [&run_range, begin, end]() { run_range( begin, end ); } );
  }
  // The calling thread takes the first chunk while the workers run.
  run_range( 0, std::min( chunk, n ) );

  for ( auto& t : threads )
    t.join();

  if ( eptr )
    std::rethrow_exception( eptr );
}
}  // namespace Pol::UoConvert

#endif  // POL_UOCONVERT_PARALLEL_H
