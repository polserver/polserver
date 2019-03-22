
#include "../clib/esignal.h"
#include "../clib/threadhelp.h"
#include "../polclock.h"
#include "napi-wrap.h"
#include "node.h"
#include "nodecall.h"
#include "nodethread.h"

using namespace Napi;

namespace Pol
{
namespace Node
{
ThreadSafeFunction tsfn;
Napi::ObjectReference requireRef;
std::promise<bool> ready;
std::atomic<bool> running;

void node_thread()
{
  running = false;
  POLLOG_INFO << "Starting node thread\n";

  // Workaround for node::Start requirement that
  // argv is sequential in memory.

  char* args = new char[20];
  strcpy( args, "node" );
  args[4] = '\0';
  strcpy( args + 5 * sizeof( char ), "./main.js" );
  char* argv[2] = {args, args + sizeof( char ) * 5};
  int argc = 2;

  RegisterBuiltinModules();
  try
  {
    int ret = node::Start( argc, argv );
    POLLOG_INFO << "Node thread finished with return value " << ret << "\n";
  }
  catch ( std::exception& ex )
  {
    POLLOG_INFO << "Node thread errored with message " << ex.what() << "\n";
  }
  delete args;
  running = false;
}

std::future<bool> start_node()
{
  threadhelp::start_thread( node_thread, "Node Thread" );
  return ready.get_future();
}

Napi::Value CreateTSFN( const CallbackInfo& info )
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  try
  {
    requireRef = Napi::Persistent( info[1].As<Object>() );
    requireRef.Set( "_refId", String::New( env, "require" ) );

    int queueSize = info[2].As<Number>().Int32Value();

    tsfn = ThreadSafeFunction::New( env,

                                    info[0].As<Function>(),

                                    Object(), "work_name", queueSize, 1,
                                    (void*)nullptr,                    // data for finalize cb
                                    []( Napi::Env, void*, void* ) {},  // finalize cb
                                    (void*)nullptr );

    running = true;
    ready.set_value( true );
    NODELOG << "Created TSFN with max queue " << queueSize << "\n";
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
  exports.Set( "start", Function::New( env, CreateTSFN ) );
  return exports;
}


NODE_API_MODULE_LINKED( tsfn, InitializeNAPI )

void RegisterBuiltinModules()
{
  _register_tsfn();
}

bool cleanup()
{
  release( std::move( Node::requireRef ) ).wait();

  return tsfn.Release();
}


}  // namespace Node
}  // namespace Pol
