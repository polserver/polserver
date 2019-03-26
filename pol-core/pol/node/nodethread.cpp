
#include "nodethread.h"
#include "../../clib/esignal.h"
#include "../../clib/logfacility.h"
#include "../../clib/threadhelp.h"
#include "../plib/systemstate.h"
#include "../polclock.h"
#include "module/objwrap.h"
#include "napi-wrap.h"
#include "node.h"
#include "nodecall.h"
#include "nodethread.h"
#include <vector>
using namespace Napi;

namespace Pol
{
namespace Node
{
ThreadSafeFunction tsfn;
Napi::ObjectReference requireRef;
std::promise<bool> ready;
std::atomic<bool> running;

// fwd decl
void RegisterBuiltinModules( Napi::Env env, Object exports );

void node_thread()
{
  running = false;
  POLLOG_INFO << "Starting node thread\n";

  // Workaround for node::Start requirement that
  // argv is sequential in memory.
  int argc = 0;
  char** carg;
  std::string argstr = Plib::systemstate.config.node_args.empty()
                           ? "node ./main.js"
                           : "node " + Plib::systemstate.config.node_args + " ./main.js";

  size_t len = argstr.size();
  char* argcstr = new char[len + 1];
  std::vector<char*> argv;

  std::memcpy( argcstr, argstr.c_str(), len + 1 );
  argv.push_back( &argcstr[0] );
  for ( size_t i = 0; i < len; ++i )
  {
    if ( argcstr[i] == ' ' )
    {
      argcstr[i] = 0;
      argv.push_back( &argcstr[i] + sizeof( char ) );
    }
  }
  argcstr[len + sizeof( char )] = 0;
  NODELOG << "Starting with argc = " << argv.size() << ", argv = ";
  for ( auto& arg : argv )
    NODELOG << arg << " ";
  NODELOG << "\n";

  try
  {
    int ret = node::Start( argv.size(), argv.data() );
    POLLOG_INFO << "Node thread finished with return value " << ret << "\n";
  }
  catch ( std::exception& ex )
  {
    POLLOG_INFO << "Node thread errored with message " << ex.what() << "\n";
  }
  // If the node thread is dead, we need to ensure the server is actually stopped.
  Clib::exit_signalled = 1;
  delete[] argcstr;
  running = false;
}

std::future<bool> start_node()
{
  threadhelp::start_thread( node_thread, "Node Thread" );
  return ready.get_future();
}


Napi::Value Configure( const CallbackInfo& info )
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  NODELOG << "[node] Received configure() call\n";

  try
  {
    if ( info.Length() < 1 )
    {
      throw Error::New( env, "Missing first argument to cofigure()" );
    }

    auto arg0 = info[0].As<Object>();
    if ( !arg0.Has( "require" ) )
    {
      throw Error::New( env, "First argument to configure() missing 'require' property" );
    }

    requireRef = Napi::Persistent( arg0 );
    requireRef.Set( "_refId", String::New( env, "configure" ) );
    NODELOG << "[node] Setting reference\n";
    return Boolean::New( env, true );
  }
  catch ( std::exception& ex )
  {
    POLLOG_ERROR << "Could not create tsfn: " << ex.what() << "\n";
    return Boolean::New( env, false );
  }
}

Napi::Value CreateTSFN( const CallbackInfo& info )
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  try
  {
    int queueSize = info[1].As<Number>().Int32Value();

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

bool cleanup()
{
  auto call = Node::makeCall<bool>( []( Napi::Env env, NodeRequest<bool>* request ) {
    requireRef.Unref();
    return true;
  } );
  call.getRef();
  return tsfn.Release();
}

/**
 * Called when the initial `pol` module gets loaded in JavaScript via
 * `process._linkedBinding`.
 */
static Napi::Object InitializeNAPI( Napi::Env env, Napi::Object exports )
{
  NODELOG << "Received call from process._linkedbinding(). Exporting modules\n";
  exports["start"] = Function::New( env, CreateTSFN );
  exports["configure"] = Function::New( env, Configure );

  // Register our additional modules
  RegisterBuiltinModules( env, exports );
  return exports;
}

void RegisterBuiltinModules( Napi::Env env, Object exports )
{
  NODELOG << "Reg'd built-in\n";
  Node::NodeObjectWrap::Init( env, exports );
}

NODE_API_MODULE_LINKED( pol, InitializeNAPI )

}  // namespace Node
}  // namespace Pol
