/** @file
 *
 * @par History
 */

#ifdef HAVE_NODEJS

#include "../bscript/eprog.h"
#include "../clib/esignal.h"
#include "../clib/threadhelp.h"
#include "../polclock.h"
#include "node.h"
#include "nodethread.h"
#include <future>


using namespace Napi;

namespace Pol
{
namespace Node
{
ThreadSafeFunction tsfn;
Napi::ObjectReference requireRef;
std::promise<bool> ready;
std::atomic<bool> running = false;

#ifdef HAVE_NODEJS
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
#endif


template <typename RequestData, typename Callback>
inline void request( RequestData* req, Callback callback )
{
  ThreadSafeFunction::Status status = tsfn.BlockingCall( req, callback );
  switch ( status )
  {
  case ThreadSafeFunction::OK:
    break;

  case ThreadSafeFunction::FULL:
    throw std::runtime_error( "Attempt to call node when thread is closed" );

  case ThreadSafeFunction::CLOSE:
    throw std::runtime_error( "Attempt to call node when thread is closed" );

  default:
    throw std::runtime_error( "Attempt to call node failed" );
  }
}

unsigned long requestNumber = 0;
struct NodeRequest
{
  NodeRequest() : reqId( requestNumber++ ), when( Core::PolClock::now() ), done(){};
  unsigned long reqId;
  Core::PolClock::time_point when;
  Core::PolClock::time_point done;
};

struct NodeRequireRequest : NodeRequest
{
  NodeRequireRequest( const std::string& name ) : NodeRequest(), scriptName( name ){};
  std::string scriptName;
};

struct NodeReleaseRequest : NodeRequest
{
  NodeReleaseRequest( ObjectReference&& ref ) : NodeRequest(), ref( std::move( ref ) ){};
  ObjectReference ref;
};

struct NodeExecRequest : NodeRequest
{
  NodeExecRequest( ObjectReference* ref ) : NodeRequest(), ref( ref ){};
  ObjectReference* ref;
};
std::future<bool> release( Napi::ObjectReference ref )
{
  std::shared_ptr<std::promise<bool>> promise = std::make_shared<std::promise<bool>>();


  auto callback = [promise]( Env env, Function jsCallback, NodeReleaseRequest* req ) {

    NODELOG.Format( "[{:04x}] [release] releasing {}\n" )
        << req->reqId << req->ref.Get( "_refId" ).As<String>().Utf8Value();

    try
    {
      req->ref.Unref();
      ( promise )->set_value( true );
      jsCallback.Call( {Number::New( env, clock() )} );
    }
    catch ( std::exception& ex )
    {
      POLLOG_ERROR << "[{:04x}] [release] exception when attempting to unref obj:" << ex.what()
                   << "\n";
      promise->set_exception( std::make_exception_ptr( ex ) );
    }
    delete req;
  };

  NodeReleaseRequest* req = new NodeReleaseRequest( std::move( ref ) );

  try
  {
    request( req, callback );
  }

  catch ( std::exception& ex )
  {
    POLLOG_ERROR << "Node error! " << ex.what() << "\n";
    if ( req )
      delete req;
    promise->set_exception( std::make_exception_ptr( ex ) );
  }
  return promise->get_future();
}


//.Get( "toString" ).As<Function>().Call( funct, {} ).As<String>().Utf8Value();
std::future<bool> call( Napi::ObjectReference& ref )
{
  std::shared_ptr<std::promise<bool>> promise = std::make_shared<std::promise<bool>>();


  auto callback = [promise]( Env env, Function jsCallback, NodeExecRequest* req ) {

    NODELOG.Format( "[{:04x}] [exec] call {} , args TODO\n" )
        << req->reqId << req->ref->Get( "_refId" ).As<String>().Utf8Value();

    auto ret = req->ref->Get( "default" ).As<Function>().Call( {} );

    auto retString =
        ret.ToObject().Get( "toString" ).As<Function>().Call( ret, {} ).As<String>().Utf8Value();


    req->done = Core::PolClock::now();
    NODELOG.Format( "[{:04x}] [exec] returned {} in {} time\n" ) << req->reqId << retString << 
         std::chrono::duration_cast<std::chrono::nanoseconds>( req->done - req->when ).count();

    ( promise )->set_value( true );
  };

  NodeExecRequest* req = new NodeExecRequest( &ref );

  try
  {
    request( req, callback );
  }

  catch ( std::exception& ex )
  {
    NODELOG.Format( "[] [call] failed, {} \n" ) << ex.what();
    promise->set_exception( std::make_exception_ptr( ex ) );
  }

  return promise->get_future();
}


std::future<Napi::ObjectReference> require( const std::string& name )
{
  auto promise = std::make_shared<std::promise<Napi::ObjectReference>>();

  auto callback = [promise]( Env env, Function jsCallback, NodeRequireRequest* request ) {

    NODELOG.Format( "[{:04x}] [require] requesting {}\n" ) << request->reqId << request->scriptName;
    try
    {
      auto req = requireRef.Value()
                     .As<Function>()
                     .Call( {Napi::String::New( env, request->scriptName )} )
                     .As<Object>();

      auto funct = req.Get( "default" );

      auto functCode = funct.As<Object>()
                           .Get( "toString" )
                           .As<Function>()
                           .Call( funct, {} )
                           .As<String>()
                           .Utf8Value();

      req.Set( "_refId", String::New( env, "require(" + request->scriptName + ")@" +
                                               std::to_string( request->reqId ) ) );

      NODELOG.Format( "[{:04x}] [require] resolved, {}\n" ) << request->reqId << functCode;
      promise->set_value( Napi::ObjectReference::New( req, 1 ) );
    }
    catch ( std::exception& ex )
    {
      NODELOG.Format( "[{:04x}] [require] failed, {} \n" ) << request->reqId << ex.what();
      promise->set_exception( std::make_exception_ptr( ex ) );
    }

    delete request;

  };

  NodeRequireRequest* req = new NodeRequireRequest( name );

  try
  {
    request( req, callback );
  }

  catch ( std::exception& ex )
  {
    POLLOG_ERROR << "Node error! " << ex.what() << "\n";
    if ( req )
      delete req;
    promise->set_exception( std::make_exception_ptr( ex ) );
  }
  return promise->get_future();
}

Napi::ObjectReference Node::obj;

template <typename Callback>
void run_in_node( Callback callback )
{
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
      Node::obj = fut.get();

      POLLOG_INFO << "Got value!\n";
      auto val = call( obj );
      val.wait();
      bool retval = val.get();
      POLLOG_INFO << "Got return " << retval << "\n";


      release( std::move( Node::obj ) ).wait();
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
    auto fut = Node::require( std::string( "./" ) + fname );
    fut.wait();
    obj = fut.get();
    POLLOG_INFO << "Got a successful read for " << fname << "\n";
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

JavascriptProgram::~JavascriptProgram()
{
  Node::release( std::move( obj ) );
}

static Napi::Value CreateTSFN( CallbackInfo& info )
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

#endif
