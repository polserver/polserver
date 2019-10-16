#ifndef __WORLDTHREAD_H
#define __WORLDTHREAD_H

#include "../../clib/threadhelp.h"
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
public:
  WorldThread();
  ~WorldThread();
  void init();
  void deinitialize();
  std::future<bool> request( std::function<void()> callback );

private:
  std::unique_ptr<threadhelp::TaskThreadPool> _thread;
};

extern WorldThread worldThread;

}  // namespace Core
}  // namespace Pol
#endif
