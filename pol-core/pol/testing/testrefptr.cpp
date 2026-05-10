/** @file
 *
 * @par History
 */

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "../../clib/logfacility.h"
#include "../../clib/refptr.h"
#include "testenv.h"

namespace Pol::Testing
{
namespace
{
class RefCountedFoo final : public ref_counted
{
public:
  RefCountedFoo() { ++live_count; }
  ~RefCountedFoo() override { --live_count; }

  static std::atomic<int> live_count;
};

std::atomic<int> RefCountedFoo::live_count{ 0 };

constexpr int kPoolSize = 16;
constexpr int kThreads = 8;
constexpr int kIterations = 50000;

// Each thread owns its own destination ref_ptr<>s and pulls source pointers
// out of a shared pool of pre-existing ref_ptr<>s. This exercises the
// destination-side race that the M1 fix targets without entering the
// (documented as unsupported) territory of mutating the same ref_ptr instance
// from multiple threads.
void hammer( const std::vector<ref_ptr<RefCountedFoo>>& pool, int seed )
{
  ref_ptr<RefCountedFoo> a;
  ref_ptr<RefCountedFoo> b;
  unsigned x = static_cast<unsigned>( seed ) * 2654435761u;
  for ( int i = 0; i < kIterations; ++i )
  {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    const auto& src = pool[x % pool.size()];
    switch ( ( x >> 8 ) & 3u )
    {
    case 0:
      a = src;  // copy-assign
      break;
    case 1:
      b.set( src.get() );  // raw set
      break;
    case 2:
    {
      ref_ptr<RefCountedFoo> tmp( src );
      a = std::move( tmp );  // move-assign
      break;
    }
    default:
      a.clear();
      break;
    }
  }
}
}  // namespace

void refptr_test()
{
  const int baseline = RefCountedFoo::live_count.load();

  {
    std::vector<ref_ptr<RefCountedFoo>> pool;
    pool.reserve( kPoolSize );
    for ( int i = 0; i < kPoolSize; ++i )
      pool.emplace_back( new RefCountedFoo );

    std::vector<std::thread> workers;
    workers.reserve( kThreads );
    for ( int t = 0; t < kThreads; ++t )
      workers.emplace_back( [&pool, t]() { hammer( pool, t + 1 ); } );

    for ( auto& w : workers )
      w.join();

    // Every Foo must still be alive: only the pool holds canonical refs.
    if ( RefCountedFoo::live_count.load() == baseline + kPoolSize )
      UnitTest::inc_successes();
    else
    {
      INFO_PRINTLN( "refptr_test: live={} expected={} after hammer",
                    RefCountedFoo::live_count.load(), baseline + kPoolSize );
      UnitTest::inc_failures();
    }
  }

  // Pool is destroyed; all Foos must be released.
  if ( RefCountedFoo::live_count.load() == baseline )
    UnitTest::inc_successes();
  else
  {
    INFO_PRINTLN( "refptr_test: live={} expected={} after pool destruction",
                  RefCountedFoo::live_count.load(), baseline );
    UnitTest::inc_failures();
  }
}
}  // namespace Pol::Testing
