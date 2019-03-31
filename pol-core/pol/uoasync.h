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
template <typename Callback, typename RequestData>
class UOAsyncRequest;

class ScriptRequest : public ref_counted
{
public:
  inline ScriptRequest() : ref_counted() {};
  /**
   * Structure encapsulating data for requesting a target from a character.
   */
  struct TargetData
  {
    //   Mobile::Character* target_cursor_chr;
    int target_options;
  };

  enum Type
  {
    TARGET_OBJECT,
    TARGET_CURSOR
  };

private:
  /**
   * We have two different Target callbacks: one for objects, and one for coords
   */
  using TargetObjectCallback = Bscript::BObjectImp*( TargetData* data, Mobile::Character* chr,
                                                     Core::UObject* obj );

  using TargetCoordsCallback = Bscript::BObjectImp*( TargetData* data, Mobile::Character* chr,
                                                     PKTBI_6C* msg );


public:
  using TargetObject = Core::UOAsyncRequest<TargetObjectCallback, TargetData>;
  using TargetCoords = Core::UOAsyncRequest<TargetCoordsCallback, TargetData>;

  virtual bool abort() = 0;
};  //

/** Request number */
extern u32 nextAsyncRequestId;

/**
 * Asynchronous Request class. Returned using `UOExecutor::makeRequest()`.
 */
template <typename Callback, typename RequestData>
class UOAsyncRequest : public ScriptRequest
{
private:
public:
  inline UOAsyncRequest( UOExecutor* exec, Mobile::Character* chr, Callback* cb, RequestData* data )
      : reqId_( nextAsyncRequestId++ ),
        cb_( cb ),
        exec_( exec ),
        chr_( chr ),
        data_( data ),
        handled_( false ){

        };
  inline ~UOAsyncRequest()
  {
    if ( !handled_ )
    {
      POLLOG_ERROR.Format(
          "Script Error! Request destroyed and unhandled! id = {}, chr = {}, script = {}, PC = "
          "{}\n" )
          << reqId_ << chr_->name() << exec_->scriptname() << exec_->PC;
    }
  }

  static std::vector<UOAsyncRequest<Callback, RequestData>*> requests;

  /**
   * Abort the request by reviving the executor and deleting the request object.
   */
  virtual bool abort() override;

  /**
   * Respond to the request. `args` must be the same types as declared when setting up
   * the request using `UOExecutor::makeRequest()`
   */
  template <typename... Ts>
  bool respond( Ts... args );

  u32 reqId_;
  Callback* cb_;
  UOExecutor* exec_;
  Mobile::Character* chr_;
  RequestData* data_;
  bool handled_;
};

template <typename Callback, typename RequestData>
bool UOAsyncRequest<Callback, RequestData>::abort()
{
  if ( !handled_ )
    exec_->revive();
  return handled_ = true;
}

template <typename Callback, typename RequestData>
template <typename... Ts>
bool UOAsyncRequest<Callback, RequestData>::respond( Ts... args )
{
  if ( handled_ )
  {
    return false;
  }

  if ( exec_ == nullptr )
  {
    // No exec..???
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
  }

  this->exec_->revive();
  return impptr != nullptr;
}


}  // namespace Core
}  // namespace Pol

#endif
