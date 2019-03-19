
#include "nodethread.h"
#include "../clib/esignal.h"
#include "../clib/threadhelp.h"
#include "../polclock.h"
#include "napi-wrap.h"
#include "node.h"
#include "nodecall.h"

using namespace Napi;

namespace Pol
{
namespace Node
{
ThreadSafeFunction tsfn;
Napi::ObjectReference requireRef;
std::promise<bool> ready;
std::atomic<bool> running = false;

void node_thread()
{
  POLLOG_INFO << "Starting node thread\n";
  char *argv[] = {"node", "./main.js"}, argc = 2;
  RegisterBuiltinModules();
  int ret;
  try
  {
    running = true;
    ret = node::Start( argc, argv );
    POLLOG_INFO << "Node thread finished with return value " << ret << "\n";
  }
  catch ( std::exception& ex )
  {
    POLLOG_INFO << "Node thread errored with message " << ex.what() << "\n";
  }
  running = false;
}

void node_shutdown_thread()
{
  int i = 0;
  while ( !Clib::exit_signalled )
  {
    Core::pol_sleep_ms( 500 );
    i++;
    // if ( i % 2 && i < 10 )
    //{
    //  POLLOG_INFO << "making blocking call\n";
    //}

    if ( false && i % 5 == 0 )
    {
      POLLOG_INFO << "Trying require...\n";
      auto fut = require( "./script.js" );
      fut.wait();
      auto obj = fut.get();

      POLLOG_INFO << "Got value!\n";
      auto val = call( obj );
      val.wait();
      bool retval = val.get();
      POLLOG_INFO << "Got return " << retval << "\n";


      release( std::move( obj ) ).wait();
    }

    /*else if ( i >= 10 )
    {
      Clib::exit_signalled = true;
    }*/

    if ( i >= 1 )
    {
      Clib::exit_signalled = true;
    }
  }


  // nodeFuncs.tsfn.Release();
}

std::future<bool> start_node()
{
  threadhelp::start_thread( node_thread, "Node Thread" );

  threadhelp::start_thread( node_shutdown_thread, "Node Shutdown Listener" );
  //  POLLOG_INFO << "Node thread finished";
  return ready.get_future();
}

Napi::Value CreateTSFN( CallbackInfo& info )
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  try
  {
    requireRef = Napi::Persistent( info[1].As<Object>() );
    requireRef.Set( "_refId", String::New( env, "require" ) );

    tsfn = ThreadSafeFunction::New( env,

                                    info[0].As<Function>(),

                                    Object(), "work_name", info[2].As<Number>().Int32Value(), 1,
                                    (void*)nullptr,                    // data for finalize cb
                                    []( Napi::Env, void*, void* ) {},  // finalize cb
                                    (void*)nullptr );


    POLLOG_INFO << "setting..\n";
    ready.set_value( true );
    POLLOG_INFO << "set promise value!\n";
    return Boolean::New( env, true );
  }
  catch ( std::exception& ex )
  {
    POLLOG_ERROR << "Could not create tsfn: " << ex.what() << "\n";
    ready.set_exception( std::make_exception_ptr( ex ) );
    return Boolean::New( env, false );
  }
}


static Napi::Object InitializeNAPI( Napi::Env env, Napi::Object exports )
{
  POLLOG_INFO << "initializing";
  exports.Set( "start", Function::New( env, CreateTSFN ) );
  POLLOG_INFO << "inited";
  return exports;
}


NODE_API_MODULE_LINKED( tsfn, InitializeNAPI )

void RegisterBuiltinModules()
{
  _register_tsfn();
}

void cleanup()
{
  release( std::move( Node::requireRef ) ).wait();

  if ( !tsfn.Release() )
  {
    Error::Fatal( "SecondaryThread", "ThreadSafeFunction.Release() failed" );
  }
  else
  {
    POLLOG_INFO << "released\n";
  }
}


}  // namespace Node
}  // namespace Pol
