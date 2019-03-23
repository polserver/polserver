/** @file
 *
 * @par History
 */

#include "../../clib/logfacility.h"
#include "../polclock.h"
#include "../uoexec.h"
#include "jsprog.h"
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

std::atomic_uint nextRequestId( 0 );

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

//NodeObjectWrap NodeObjectWrap::New( Napi::Env env, Bscript::BObjectImp* impptr )
//{
//  (void)env;
//}

void Node::callProgram( Node::JavascriptProgram* prog, Core::UOExecutor* ex )
{
  auto call = Node::makeCall<int>( [prog, ex]( Napi::Env env, NodeRequest<int>* request ) {

    auto obj = prog->obj.Value();
    NODELOG.Format( "[{:04x}] [exec] call {} , argc {}\n" )
        << request->reqId() << obj.Get( "_refId" ).As<String>().Utf8Value()
        << ex->ValueStack.size();

    auto argv = std::vector<Napi::Value>();
    for ( size_t i = 0; !ex->ValueStack.empty(); )
    {
      Bscript::BObjectRef rightref = ex->ValueStack.back();
      ex->ValueStack.pop_back();

      // Lets do basic conversions here...
      Napi::Value convertedVal;
      auto* impptr = rightref.get()->impptr();
     // auto x = Napi::External<Bscript::BObjectImp>::New( env, impptr );

     // NODELOG << "To string: " << x.ToString().Utf8Value() << "\n";
      auto val = env.Global().Has( "Proxy" );
      if ( val )
      {
        NODELOG << "WE HAVE PROXY OBJECT!\n";
      }

      if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTLong ) )
      {
        NODELOG << "ITS A LONG!\n";

        auto longptr = impptr;
        //NodeObjectWrap longwrap = NodeObjectWrap::New( env, impptr );
        Bscript::BLong* aob = Clib::explicit_cast<Bscript::BLong*, Bscript::BObjectImp*>( impptr );
        convertedVal = Napi::Number::New( env, aob->value() );
      }
      else
      {
        NODELOG.Format( "[{:04x}] [exec] error converting arg {}\n" )
            << request->reqId() << i;
        convertedVal = env.Undefined();
      }
      argv.push_back( convertedVal );
      auto last = argv.back();
      NODELOG.Format( "[{:04x}] [exec] argv[{}] = {}\n" )
          << request->reqId() << i << Node::ToUtf8Value( last );
    }
    // TODO pass args
    auto ret = obj.Get( "default" ).As<Function>().Call( {} );

    // auto retString = Node::ToUtf8Value( ret );
    NODELOG.Format( "[{:04x}] [exec] returned {}\n" ) << request->reqId() << "";

    return clock();
  } );


  int theValue = call.getRef();
  NODELOG << "The clock was " << theValue << "\n";
}


std::future<bool> release( Napi::ObjectReference ref )
{
  std::shared_ptr<std::promise<bool>> promise = std::make_shared<std::promise<bool>>();

  /*
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
    }*/
  return promise->get_future();
}


std::future<bool> call( Napi::ObjectReference& ref )
{
  std::shared_ptr<std::promise<bool>> promise = std::make_shared<std::promise<bool>>();


  // auto callback = [promise]( Env /*env*/, Function /*jsCallback*/, NodeExecRequest* req ) {

  //  NODELOG.Format( "[{:04x}] [exec] call {} , args TODO\n" )
  //      << req->reqId << req->ref->Get( "_refId" ).As<String>().Utf8Value();

  //  auto ret = req->ref->Get( "default" ).As<Function>().Call( {} );

  //  auto retString =
  //      ret.ToObject().Get( "toString" ).As<Function>().Call( ret, {} ).As<String>().Utf8Value();


  //  req->done = Core::PolClock::now();
  //  NODELOG.Format( "[{:04x}] [exec] returned {} in {} time\n" )
  //      << req->reqId << retString
  //      << std::chrono::duration_cast<std::chrono::nanoseconds>( req->done - req->when ).count();

  //  ( promise )->set_value( true );
  //};

  // NodeExecRequest* req = new NodeExecRequest( &ref );

  // try
  //{
  //  request( req, callback );
  //}

  // catch ( std::exception& ex )
  //{
  //  NODELOG.Format( "[] [call] failed, {} \n" ) << ex.what();
  //  promise->set_exception( std::make_exception_ptr( ex ) );
  //}

  return promise->get_future();
}


std::future<Napi::ObjectReference> require( const std::string& name )
{
  auto promise = std::make_shared<std::promise<Napi::ObjectReference>>();

  // auto callback = [promise]( Env env, Function /*jsCallback*/, NodeRequireRequest* request ) {

  //  NODELOG.Format( "[{:04x}] [require] requesting {}\n" ) << request->reqId <<
  //  request->scriptName; try
  //  {
  //    auto req = requireRef.Value()
  //                   .As<Function>()
  //                   .Call( {Napi::String::New( env, request->scriptName )} )
  //                   .As<Object>();

  //    auto funct = req.Get( "default" );

  //    auto functCode = funct.As<Object>()
  //                         .Get( "toString" )
  //                         .As<Function>()
  //                         .Call( funct, {} )
  //                         .As<String>()
  //                         .Utf8Value();

  //    req.Set( "_refId", String::New( env, "require(" + request->scriptName + ")@" +
  //                                             std::to_string( request->reqId ) ) );

  //    NODELOG.Format( "[{:04x}] [require] resolved, {}\n" ) << request->reqId << functCode;
  promise->set_value( Napi::ObjectReference() );
  //  }
  //  catch ( std::exception& ex )
  //  {
  //    NODELOG.Format( "[{:04x}] [require] failed, {} \n" ) << request->reqId << ex.what();
  //    promise->set_exception( std::make_exception_ptr( ex ) );
  //  }

  //  delete request;

  //};

  // NodeRequireRequest* req = new NodeRequireRequest( name );

  // try
  //{
  //  request( req, callback );
  //}

  // catch ( std::exception& ex )
  //{
  //  POLLOG_ERROR << "Node error! " << ex.what() << "\n";
  //  if ( req )
  //    delete req;
  //  promise->set_exception( std::make_exception_ptr( ex ) );
  //}
  return promise->get_future();
}


}  // namespace Node
}  // namespace Pol
