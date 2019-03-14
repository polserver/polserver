/** @file
 *
 * @par History
 */

#ifdef HAVE_NODEJS
#include "node.h"
#endif

#include "../bscript/eprog.h"
#include "../clib/esignal.h"
#include "../clib/threadhelp.h"
#include "../polclock.h"
#include "nodethread.h"
#include <thread>


using namespace Napi;

namespace Pol
{
namespace Node
{
ThreadSafeFunction tsfn;

std::promise<bool> ready;

void node_thread()
{
  POLLOG_INFO << "Starting node thread";
  char *argv[] = {"node", "./main.js"}, argc = 2;
  RegisterBuiltinModules();
  int ret;
  try {

    ret = node::Start( argc, argv );
    POLLOG_INFO << "Node thread finished with val " << ret << "\n";

  }
  catch ( std::exception& ex )
  {
    POLLOG_INFO << "Node threa errored with message " << ex.what() << "\n";


  }
}

Emitter* Emitter::INSTANCE;


void require(const std::string& name) 
{
  auto callback = [name]( Env env, Function jsCallback ) {
    POLLOG_INFO << "Call to require " << name << "\n";
    auto ret = Emitter::INSTANCE->requireRef.Value().As<Function>().Call( {Napi::String::New( env, name )} );
    auto funct = ret.As<Object>().Get( "default" );
    auto ret2 = funct
                    .As<Object>()
                    .Get( "toString" )
                    .As<Function>()
                    .Call( funct,  {} )
                    .As<String>()
                    .Utf8Value();

    POLLOG_INFO << "We got value " << ret2 << "\n";
    jsCallback.Call( {Number::New( env, clock() )} );
  };

  ThreadSafeFunction::Status status = tsfn.BlockingCall( callback );
  switch ( status )
  {
  case ThreadSafeFunction::FULL:
    Error::Fatal( "DataSourceThread", "ThreadSafeFunction.*Call() queue is full" );


  case ThreadSafeFunction::OK:
    POLLOG_INFO << "made blocking call\n";
    break;

  case ThreadSafeFunction::CLOSE:
    Error::Fatal( "DataSourceThread", "ThreadSafeFunction.*Call() is closed" );

  default:
    Error::Fatal( "DataSourceThread", "ThreadSafeFunction.*Call() failed" );
  }
}
void node_shutdown_thread()
{
  int i = 0;
  while ( !Clib::exit_signalled )
  {
    Core::pol_sleep_ms( 500 );
    i++;
    //if ( i % 2 && i < 10 )
    //{
    //  POLLOG_INFO << "making blocking call\n";
    //}

    if ( i == 5 )
    {
      POLLOG_INFO << "Trying require...\n";
      require( "./mod.js" );
    }

    /*else if ( i >= 10 )
    {
      Clib::exit_signalled = true;
    }*/

    else if ( i == 10 )
    {
      if ( !tsfn.Release() )
      {
        Error::Fatal( "SecondaryThread", "ThreadSafeFunction.Release() failed" );
      }
      else
      {
        POLLOG_INFO << "released\n";
      }
    }
    else if ( i >= 12 )
    {
      Clib::exit_signalled = true;
    }
  }
  // if ( nodeFuncs.tsfn.Acquire() )
  if ( false )
  {
    Core::pol_sleep_ms( 5000 );

    POLLOG_INFO << "release\n";

    // nodeFuncs.tsfn.BlockingCall( []( Env env, Function jsCallback ) {
    //  jsCallback.Call( {String::New( env, "shutdown" )} );
    //  // Emitter::E
    //  POLLOG_INFO << "tsfn releasing\n";


    if ( !tsfn.Release() )
    {
      Error::Fatal( "SecondaryThread", "ThreadSafeFunction.Release() failed" );
    }
    else
    {
      POLLOG_INFO << "released\n";
    }
    nodeFuncs = {};
    //  POLLOG_INFO << "released\n";

    //} );
  }
  else
  {
    POLLOG_INFO << "nothing to release tsfn\n";
  }

  // nodeFuncs.tsfn.Release();
}

std::future<bool> start_node()
{
#ifdef HAVE_NODEJS
  threadhelp::start_thread( node_thread, "Node Thread" );

  threadhelp::start_thread( node_shutdown_thread, "Node Shutdown Listener" );
  //  POLLOG_INFO << "Node thread finished";
#else
  POLLOG_INFO << "Nodejs not compiled";
  ready.set_value( false );

#endif
  return ready.get_future();
}

JavascriptProgram::JavascriptProgram() : Program() {}

Bscript::Program* JavascriptProgram::create()
{
  return new JavascriptProgram;
}

bool JavascriptProgram::hasProgram() const
{
  return false;
}

int JavascriptProgram::read( const char* fname )
{
  try
  {
    throw std::runtime_error( std::string( "Javascript read not implemented: cannot read " ) +
                              fname );
    return 0;
  }
  catch ( std::exception& ex )
  {
    ERROR_PRINT << "Exception caught while loading node script " << fname << ": " << ex.what()
                << "\n";
  }
  return -1;
}

void JavascriptProgram::package( const Plib::Package* package )
{
  pkg = package;
}

std::string JavascriptProgram::scriptname() const
{
  return name;
}

Bscript::Program::ProgramType JavascriptProgram::type() const
{
  return Bscript::Program::JAVASCRIPT;
}


enum
{
  NM_F_BUILTIN = 1 << 0,
  NM_F_LINKED = 1 << 1,
  NM_F_INTERNAL = 1 << 2,
};

Napi::FunctionReference Emitter::constructor;

Napi::Object Emitter::Init( Napi::Env env, Napi::Object exports )
{
  Napi::HandleScope scope( env );

  Napi::Function func = DefineClass(
      env, "Emitter",
      {InstanceMethod( "start", &Emitter::Start ), InstanceMethod( "stop", &Emitter::Stop )} );

  constructor = Napi::Persistent( func );
  constructor.SuppressDestruct();

  exports.Set( "Emitter", func );
  return exports;
}

// ThreadSafeFunction tsfn;

Emitter::Emitter( const Napi::CallbackInfo& info ) : Napi::ObjectWrap<Emitter>( info )
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  requireRef = Napi::Persistent( info[1].As<Object>() );

  tsfn = ThreadSafeFunction::New(
      env,

      info[0].As<Function>(),

      Object(), "work_name", 0, 1,
      (void*)nullptr,                                       // data for finalize cb
      []( Napi::Env, void*, Emitter* ) {},  // finalize cb
      this );


  POLLOG_INFO << "setting..\n";
  ready.set_value( true );
  POLLOG_INFO << "set promise value!\n";

  Emitter::INSTANCE = this;

  auto callback = []( Napi::Env env, Napi::Function jsCallback ) {
    POLLOG_INFO << "callback from blocking call called!\n";
    jsCallback.Call( {Number::New( env, clock() )} );
  };

  ThreadSafeFunction::Status status = tsfn.BlockingCall( callback );
  switch ( status )
  {
  case ThreadSafeFunction::FULL:
    Error::Fatal( "DataSourceThread", "ThreadSafeFunction.*Call() queue is full" );


  case ThreadSafeFunction::OK:
    break;

  case ThreadSafeFunction::CLOSE:
    Error::Fatal( "DataSourceThread", "ThreadSafeFunction.*Call() is closed" );

  default:
    Error::Fatal( "DataSourceThread", "ThreadSafeFunction.*Call() failed" );
  }

  POLLOG_INFO << "made first blocking call\n";
}


Napi::Value Emitter::Start( const Napi::CallbackInfo& info )
{
  Napi::Env env = info.Env();
  POLLOG_INFO << "Start() called\n";
  return Napi::Boolean::New( info.Env(), true );
}

Napi::Value Emitter::Stop( const Napi::CallbackInfo& info )
{
  Napi::Env env = info.Env();
  POLLOG_INFO << "Stop called\n";
  return env.Undefined();
}

#define NODE_API_MODULE_LINKED( modname, regfunc )                \
  napi_value __napi_##regfunc( napi_env env, napi_value exports ) \
  {                                                               \
    return Napi::RegisterModule( env, exports, regfunc );         \
  }                                                               \
  NAPI_MODULE_X( modname, __napi_##regfunc, nullptr,              \
                 NM_F_LINKED )  // NOLINT (readability/null_usage)


Napi::Object InitializeNAPI( Napi::Env env, Napi::Object exports )
{
  POLLOG_INFO << "initializing";
  Emitter::Init( env, exports );
  POLLOG_INFO << "inited";
  return exports;
}

NODE_API_MODULE_LINKED( tsfn, InitializeNAPI )

void RegisterBuiltinModules()
{
  _register_tsfn();
}


}  // namespace Node
}  // namespace Pol