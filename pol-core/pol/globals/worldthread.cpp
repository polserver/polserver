#include "worldthread.h"
#include "../clib/threadhelp.h"
#include "polsem.h"

namespace Pol
{
namespace Core
{
WorldThread worldThread;


WorldThread::WorldThread() : _thread( new threadhelp::TaskThreadPool ) {}

WorldThread::~WorldThread() {}

void WorldThread::init()
{
  _thread->init_pool( 1, "WorldThread" );
}
void WorldThread::deinitialize()
{
  _thread.reset();
}
std::future<bool> WorldThread::request( std::function<void()> callback )
{
  return _thread->checked_push( [ callback = move(callback) ] {
    PolLock lck;
    callback();
  } );
}

}  // namespace Core
}  // namespace Pol
