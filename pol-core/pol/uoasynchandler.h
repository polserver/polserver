
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


}  // namespace Core
}  // namespace Pol

#endif
