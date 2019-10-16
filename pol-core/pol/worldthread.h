#ifndef __WORLDTHREAD_H
#define __WORLDTHREAD_H

#include "../clib/message_queue.h"
#include "../clib/passert.h"
#include <future>

namespace Pol
{
namespace Core
{
struct WorldThreadMessage
{
  std::function<void()> func;
  std::promise<bool> prom;
};
class WorldThread
{
private:
  WorldThread();
  static std::unique_ptr<WorldThread> worldThread;

public:
  ~WorldThread();
  typedef Clib::message_queue<WorldThreadMessage> msg_queue;

  static void ThreadEntry( void* worldThreadPromise );

  msg_queue _queue;

  bool process_wait();

  template <typename Callback>
  static std::future<bool> request( Callback callback )
  {
    passert_r( worldThread != nullptr, "WorldThread instance not set" );
    auto y = WorldThreadMessage{callback, std::promise<bool>()};
    auto fut = y.prom.get_future();
    // callback(args...);
    worldThread->_queue.push_move( std::move( y ) );
    return fut;
  }

  static void shutdown();
};

}  // namespace Core
}  // namespace Pol
#endif
