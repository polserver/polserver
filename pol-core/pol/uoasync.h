/** @file
 *
 * @par History
 * - 2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now
 */


#ifndef __UOASYNC_H
#define __UOASYNC_H


#include <string>
#include <time.h>

#include "../clib/rawtypes.h"
#include "../clib/weakptr.h"
#include "./globals/script_internals.h"
#include "./mobile/charactr.h"
#include "./network/pktboth.h"
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
 * Structure encapsulating data for requesting a target from a character.
 */
struct TargetRequestData
{
  Mobile::Character* target_cursor_chr;
  int target_options;
};

/**
 * We have two different Target callbacks: one for objects, and one for coords
 */

using TargetObjectCallback = Bscript::BObjectImp*( TargetRequestData* data, Mobile::Character* chr,
                                                   Core::UObject* obj );

using TargetCoordsCallback = Bscript::BObjectImp*( TargetRequestData* data, Mobile::Character* chr,
                                                   PKTBI_6C* msg );
/** Request number */
extern u32 nextAsyncRequestId;

/**
 * Asynchronous Request class. Returned using `UOExecutor::makeRequest()`.
 */
template <typename Callback, typename RequestData>
class UOAsyncRequest
{
private:
public:
  inline UOAsyncRequest( UOExecutor* exec, Callback* cb, RequestData* data )
      : reqId_( nextAsyncRequestId++ ), cb_( cb ), exec_( exec ), data_( data ){};


  /**
   * Abort the request by reviving the executor and deleting the request object.
   */
  bool abort();

  /**
   * Respond to the request. `args` must be the same types as declared when setting up
   * the request using `UOExecutor::makeRequest()`
   */
  template <typename... Ts>
  bool respond( Ts... args );

  u32 reqId_;
  Callback* cb_;
  UOExecutor* exec_;
  RequestData* data_;
};

template <typename Callback, typename RequestData>
bool UOAsyncRequest<Callback, RequestData>::abort()
{
  exec_->revive();
  delete this;
  return true;
}

template <typename Callback, typename RequestData>
template <typename... Ts>
bool UOAsyncRequest<Callback, RequestData>::respond( Ts... args )
{
  if ( exec_ == nullptr )
  {
    // No exec..???
    delete this;
    return false;
  }

  auto impptr = cb_( data_, args... );
  // deleting data will delete any stack-allocated memory as well
  if ( data_ != nullptr )
    delete data_;

  if ( impptr != nullptr )
  {
    // If the callback returned a value, send it back to the script.
    this->exec_->ValueStack.back().set( new Bscript::BObject( impptr ) );
    this->exec_->revive();
    delete this;
    return true;
  }
  else
  {
    this->exec_->revive();
    delete this;
    return false;
  }
}
}  // namespace Core
}  // namespace Pol

#endif
