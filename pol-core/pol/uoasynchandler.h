
#ifndef __UOASYNCHANDLER_H
#define __UOASYNCHANDLER_H


#include <string>
#include <time.h>

#include "../clib/rawtypes.h"
#include "../clib/weakptr.h"
#include "./globals/script_internals.h"
#include "./mobile/charactr.h"
#include "./network/cgdata.h"
#include "./network/client.h"
#include "./network/pktboth.h"
#include "uoasync.h"
#include "uoexec.h"

namespace Pol
{
namespace Module
{
class OSExecutorModule;
}
namespace Core
{
/**
 * Asynchronous Request class. Returned using `UOExecutor::makeRequest()`.
 */
template <typename Callback, typename RequestData>
class AsyncRequestHandler : public UOAsyncRequest
{
private:
public:
  inline AsyncRequestHandler( UOExecutor& exec, Mobile::Character* chr, UOAsyncRequest::Type type,
                              Callback* cb, RequestData* data )
      : UOAsyncRequest( exec, chr, type ),
        cb_( cb ),
        data_( data ){

        };
  /**
   * Respond to the request. `args` must be the same types as declared when setting up
   * the request using `UOExecutor::makeRequest()`
   */
  template <typename... Ts>
  bool respond( Ts... args );
  Callback* cb_;
  RequestData* data_;
};

template <typename Callback, typename RequestData>
template <typename... Ts>
bool AsyncRequestHandler<Callback, RequestData>::respond( Ts... args )
{
  if ( handled_ )
  {
    return false;
  }

  if ( chr_ == nullptr )
  {
    // No chr..???
    return false;
  }

  auto impptr = cb_( data_, args... );
  // deleting data will delete any stack-allocated memory as well
  if ( data_ != nullptr )
    delete data_;

  this->resolved( impptr );

  return impptr != nullptr;
}


/**
 * Asynchronous Request class. Returned using `UOExecutor::makeRequest()` with no data attached
 * to the request (eg. text entry)
 */
template <typename Callback>
class AsyncRequestHandlerSansData : public UOAsyncRequest
{
private:
public:
  inline AsyncRequestHandlerSansData( UOExecutor& exec, Mobile::Character* chr,
                                      UOAsyncRequest::Type type, Callback* cb )
      : UOAsyncRequest( exec, chr, type ),
        cb_( cb ){

        };
  /**
   * Respond to the request. `args` must be the same types as declared when setting up
   * the request using `UOExecutor::makeRequest()`
   */
  template <typename... Ts>
  bool respond( Ts... args );
  Callback* cb_;
};

template <typename Callback>
template <typename... Ts>
bool AsyncRequestHandlerSansData<Callback>::respond( Ts... args )
{
  if ( handled_ )
  {
    return false;
  }

  if ( chr_ == nullptr )
  {
    // No chr..???
    return false;
  }

  auto impptr = cb_( args... );

  this->resolved( impptr );

  return impptr != nullptr;
}


// With data
template <typename Callback, typename RequestData>
ref_ptr<Core::UOAsyncRequest> UOAsyncRequest::makeRequest( Core::UOExecutor& exec,
                                                           Mobile::Character* chr, Type type,
                                                           Callback* callback, RequestData* data )
{
  using namespace Napi;
  using namespace Node;

  if ( !exec.suspend() )
  {
    if ( data != nullptr )
      delete data;
    return ref_ptr<Core::UOAsyncRequest>( nullptr );
  }


  ref_ptr<Core::UOAsyncRequest> req(
      new AsyncRequestHandler<Callback, RequestData>( exec, chr, type, callback, data ) );
  exec.addRequest( req );
  //.addRequest( type, req );
  chr->client->gd->requests.addRequest( type, req );
  return req;
}

// No data
template <typename Callback>
ref_ptr<Core::UOAsyncRequest> UOAsyncRequest::makeRequest( Core::UOExecutor& exec,
                                                           Mobile::Character* chr, Type type,
                                                           Callback* callback )
{
  if ( !exec.suspend() )
  {
    return ref_ptr<Core::UOAsyncRequest>( nullptr );
  }
  ref_ptr<Core::UOAsyncRequest> req(
      new AsyncRequestHandlerSansData<Callback>( exec, chr, type, callback ) );
  exec.addRequest( req );
  chr->client->gd->requests.addRequest( type, req );
  return req;
}


}  // namespace Core
}  // namespace Pol

#endif
