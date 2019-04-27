// DO NOT INCLUDE THIS FILE DIRECTLY, INCLUDE nodecall.h

#ifndef NODECALL_INL_H
#define NODECALL_INL_H

namespace Pol
{
namespace Node
{
template <typename ReturnType>
NodeRequest<ReturnType>::NodeRequest( Core::UOExecutor* uoexec )
    : reqId_( nextRequestId++ ), timer_(), points_(), uoexec_( uoexec )
{
}

template <typename ReturnType>
unsigned int NodeRequest<ReturnType>::reqId() const
{
  return reqId_;
}

template <typename ReturnType>
void NodeRequest<ReturnType>::ref( ReturnType&& r )
{
  ref_ = std::move( r );
}

template <typename ReturnType>
ReturnType NodeRequest<ReturnType>::getRef()
{
  return std::move( ref_ );
}


template <typename ReturnType>
Node::timestamp NodeRequest<ReturnType>::checkpoint( const std::string& key )
{
  points_.push_back( std::make_tuple( key, timer_.ellapsed() ) );
  return std::get<1>( points_.back() );
}

/**
 * Make a call into Node.
 */

template <typename ReturnType, typename Callable>
NodeRequest<ReturnType> makeCall( Callable callable, Core::UOExecutor* uoexec, bool blocking )
{
  auto promise2 = std::make_shared<std::promise<void>>();
  NodeRequest<ReturnType>* req = new NodeRequest<ReturnType>( uoexec );


  ThreadSafeFunction::Status status =
      tsfn.BlockingCall( [blocking, req, promise2, callable]( Napi::Env env, Function jsFunc ) {
        (void)jsFunc;  // we do not need to call into the tsfn's registered callback
        req->checkpoint( "enter js thread" );
        try
        {
          req->ref( callable( env, req ) );
        }
        catch ( std::exception& ex )
        {
        }
        if ( blocking )
          promise2->set_value();
      } );

  try
  {
    switch ( status )
    {
    case ThreadSafeFunction::OK:
      if ( !blocking )
        promise2->set_value();
      break;

    case ThreadSafeFunction::FULL:
      throw std::runtime_error( "Attempt to call node when thread is closed" );

    case ThreadSafeFunction::CLOSE:
      throw std::runtime_error( "Attempt to call node when thread is closed" );

    default:
      throw std::runtime_error( "Attempt to call node failed" );
    }
  }
  catch ( std::exception& ex )
  {
    promise2->set_exception( std::make_exception_ptr( ex ) );
  }
  // We will block until we have entered the js thread.
  // Then, it is up to the core to decide what to do with the return value.
  auto fut2 = promise2->get_future();
  fut2.wait();

  return std::move( *req );
}


}  // namespace Node

}  // namespace Pol

#endif
