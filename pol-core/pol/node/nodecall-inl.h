// DO NOT INCLUDE THIS FILE DIRECTLY, INCLUDE nodecall.h

#ifndef NODECALL_INL_H
#define NODECALL_INL_H

////////////////////////////////////////////////////////////////////////////////
// N-API C++ Wrapper Classes
//
// Inline header-only implementations for "N-API" ABI-stable C APIs for Node.js.
////////////////////////////////////////////////////////////////////////////////

// Note: Do not include this file directly! Include "napi.h" instead.
namespace Pol
{
namespace Node
{
/*
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
}*/
//
// template <typename Return, typename Callable, typename DataType>
// std::future<Return> makeCall( Callable callable, DataType* data )
//{
//  Request* request = new Request();  // create a new request, begins the clock
//  auto promise = std::make_shared<std::promise<Return>>();
//  tsfn.BlockingCall(
//      data, [promise, callable, request]( Napi::Env env, Function jsFunc, DataType* data ) {
//        (void)jsFunc;  // we do not need to call into the tsfn's registered callback
//        promise->set_value( callable( env, request ) );
//      } );
//  return promise->get_future();
//}


template <typename ReturnType>
NodeRequest<ReturnType>::NodeRequest()
    : reqId_( nextRequestId++ ), timer_(), points_(), env_( nullptr )
{
}

template <typename ReturnType>
NodeRequest<ReturnType>::NodeRequest( Napi::Env env )
    : reqId_( nextRequestId++ ), timer_(), points_(), env_( env )
{
}

template <typename ReturnType>
unsigned int NodeRequest<ReturnType>::reqId() const
{
  return reqId_;
}

template <typename ReturnType>
void NodeRequest<ReturnType>::ref( ReturnType &&r)
{
  ref_ = std::move(r);
}

template <typename ReturnType>
ReturnType NodeRequest<ReturnType>::getRef( )
{
  return std::move( ref_ );
}


template <typename ReturnType>
Node::timestamp NodeRequest<ReturnType>::checkpoint( const std::string& key )
{
  points_.push_back( std::make_tuple( key, timer_.ellapsed() ) );
  return std::get<1>( points_.back() );
}


template <typename ReturnType, typename Callable>
NodeRequest<ReturnType> makeCall( Callable callable )
{
  // ReturnDetails<ReturnType> dets(
  // NodeRequest<ReturnType> request()/*;*/  // create a new request, begins the clock


  // auto request = std::make_shared<NodeRequest<ReturnType>>();
  //promise = std::make_unique<std::promise<NodeRequest<ReturnType>>>();
  auto promise2 = std::make_shared<std::promise<void>>();
  NodeRequest<ReturnType>* req = new NodeRequest<ReturnType>();
  tsfn.BlockingCall( [req, promise2, callable]( Napi::Env env, Function jsFunc ) {
    (void)jsFunc;  // we do not need to call into the tsfn's registered callback
    req->checkpoint( "enter js thread" );
    // promise resolve, and let caller (ie, the core) decide what to do

    //std::promise<ReturnValue> retVal;
    //request->val = retVal.get_future();;
    req->ref( callable( env, req ) );
    //req.promise->set_value( request.get() );
    promise2->set_value();
  });
  // We will block until we have entered the js thread.
  // Then, it is up to the core to decide what to do.
  auto fut2 = promise2->get_future();
  fut2.wait();

  return std::move(*req);
}
}
}

#endif
