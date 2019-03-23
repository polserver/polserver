/** @file
 *
 * @par History
 */

#include "../../clib/logfacility.h"
#include "../polclock.h"
#include "napi-wrap.h"
#include "nodecall.h"
#include "nodethread.h"
#include <future>

using namespace Napi;

namespace Pol
{
namespace Node
{
unsigned long requestNumber = 0;

std::atomic_uint Request::nextRequestId( 0 );


Request::Request( Napi::Env env ) : reqId_( nextRequestId++ ), timer_(), points_(), env_( env ) {}

unsigned int Request::reqId() const
{
  return reqId_;
}

Request::timestamp Request::checkpoint( const std::string& key )
{
  points_.push_back( std::make_tuple( key, timer_.ellapsed() ) );
  return std::get<1>( points_.back() );
}

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


std::future<bool> call( Napi::ObjectReference& ref )
{
  std::shared_ptr<std::promise<bool>> promise = std::make_shared<std::promise<bool>>();


  auto callback = [promise]( Env /*env*/, Function /*jsCallback*/, NodeExecRequest* req ) {

    NODELOG.Format( "[{:04x}] [exec] call {} , args TODO\n" )
        << req->reqId << req->ref->Get( "_refId" ).As<String>().Utf8Value();

    auto ret = req->ref->Get( "default" ).As<Function>().Call( {} );

    auto retString =
        ret.ToObject().Get( "toString" ).As<Function>().Call( ret, {} ).As<String>().Utf8Value();


    req->done = Core::PolClock::now();
    NODELOG.Format( "[{:04x}] [exec] returned {} in {} time\n" )
        << req->reqId << retString
        << std::chrono::duration_cast<std::chrono::nanoseconds>( req->done - req->when ).count();

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

  auto callback = [promise]( Env env, Function /*jsCallback*/, NodeRequireRequest* request ) {

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


}  // namespace Node
}  // namespace Pol
