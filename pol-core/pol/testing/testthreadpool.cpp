
#include "testenv.h"

#include "clib/threadhelp.h"
#include <thread>

namespace Pol::Testing
{
void dynthreadpool_test()
{
  threadhelp::DynTaskThreadPool pool{ "test" };

  UnitTest( [&]() { return pool.threadpoolsize(); }, 0u, "init" );
  UnitTest( [&]() { return pool.checked_push( [] { return true; } ).get(); }, true, "first work" );
  UnitTest( [&]() { return pool.threadpoolsize(); }, 1u, "pool size after first run" );
  UnitTest(
      [&]()
      {
        try
        {
          pool.checked_push( [] { throw std::exception(); } ).get();
        }
        catch ( std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "exception in checked push" );
  UnitTest(
      [&]()
      {
        pool.push(
            []
            {
              INFO_PRINTLN( "throw" );
              throw std::exception();
            } );
        return true;
      },
      true, "exception in push" );
  UnitTest( [&]() { return pool.checked_push( [] { return true; } ).get(); }, true,
            "would block endless if worker died before" );
}

}  // namespace Pol::Testing
