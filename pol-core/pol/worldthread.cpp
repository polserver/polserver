#include "worldthread.h"
#include "../clib/esignal.h"
#include "../clib/make_unique.hpp"
#include "polsem.h"

namespace Pol
{
namespace Core
{
std::unique_ptr<WorldThread> WorldThread::worldThread;

void WorldThread::ThreadEntry(void* worldThreadPromise)
{
  auto p = static_cast<std::promise<void>*>( worldThreadPromise );
  passert_r( worldThread == nullptr, "WorldThread already started" );
  worldThread = std::unique_ptr<WorldThread>( new WorldThread() );
  p->set_value();

  POLLOG_INFO << "WorldThread started...\n";
  while ( !Clib::exit_signalled )
  {
    // ignore remaining tasks
    if ( !worldThread->process_wait() )
      break;
  }
  POLLOG_INFO << "WorldThread ended.\n";
}

WorldThread::WorldThread() : _queue() {}

WorldThread::~WorldThread()
{
  _queue.cancel();
}

void WorldThread::shutdown()
{
  passert_r( worldThread != nullptr, "WorldThread does not exist" );
  worldThread.reset();
}

bool WorldThread::process_wait()
{
  try
  {
    WorldThreadMessage task;
    _queue.pop_wait( &task );
    {
      PolLock lck;
      try
      {
        task.func();
        task.prom.set_value( true );
      }
      catch ( std::exception& ex )
      {
        task.prom.set_exception( std::make_exception_ptr(ex) );
      }
    }
    return true;
  }
  catch ( msg_queue::Canceled& )
  {
    return false;
  }
}

}  // namespace Core
}  // namespace Pol
