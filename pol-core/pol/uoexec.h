/** @file
 *
 * @par History
 * - 2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now
 */


#ifndef __UOEXEC_H
#define __UOEXEC_H

#ifndef __BSCRIPT_EXECUTOR_H
#include "../bscript/executor.h"
#endif

#include <string>
#include <time.h>

#include "../clib/rawtypes.h"
#include "../clib/weakptr.h"
#include "./globals/script_internals.h"
#include "./mobile/charactr.h"

namespace Pol
{
namespace Module
{
class OSExecutorModule;
}
namespace Core
{
// const int SCRIPT_RUNAWAY_INTERVAL = 5000;

class UOExecutor;

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
      : reqId_( nextAsyncRequestId++ ), cb_( cb ), data_( data ), exec_( exec ){};


  /**
   * Abort the request by reviving the executor and deleting the request object.
   */
  inline bool abort() {
    exec_->revive();
    delete this;
    return true;
  }

  /**
   * Respond to the request. `args` must be the same types as declared when setting up
   * the request using `UOExecutor::makeRequest()` 
   */
  template <typename... Ts>
  inline bool respond( Ts... args )
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

  u32 reqId_;
  Callback* cb_;
  UOExecutor* exec_;
  RequestData* data_;
};

class UOExecutor final : public Bscript::Executor
{
  typedef Bscript::Executor base;

private:
  Module::OSExecutorModule* os_module;

public:
  UOExecutor();
  virtual ~UOExecutor();
  virtual size_t sizeEstimate() const override;

  bool suspend();
  bool revive();

  std::string state();

  u64 instr_cycles;
  u64 sleep_cycles;
  time_t start_time;

  u64 warn_runaway_on_cycle;
  u64 runaway_cycles;

  bool listens_to( unsigned int eventflag ) const;
  bool signal_event( Bscript::BObjectImp* eventimp );

  unsigned int eventmask;
  unsigned short area_size;
  unsigned short speech_size;

  bool can_access_offline_mobiles;
  bool auxsvc_assume_string;
  weak_ptr_owner<UOExecutor> weakptr;

  UOExecutor *pParent, *pChild;

public:
  bool critical() const;
  void critical( bool critical );

  unsigned char priority() const;
  void priority( unsigned char priority );

  bool warn_on_runaway() const;
  void warn_on_runaway( bool warn_on_runaway );

  void SleepFor( int secs );
  void SleepForMs( int msecs );
  unsigned int pid() const;
  bool blocked() const;
  bool in_debugger_holdlist() const;
  void revive_debugged();

  Core::polclock_t sleep_until_clock() const;
  void sleep_until_clock( Core::polclock_t sleep_until_clock );

  Core::TimeoutHandle hold_itr() const;
  void hold_itr( Core::TimeoutHandle hold_itr );

  Core::HoldListType in_hold_list() const;
  void in_hold_list( Core::HoldListType in_hold_list );


  Bscript::BObjectImp* clear_event_queue();

  /**
   * Create a new request. Will return `nullptr` if a request cannot be made, eg.
   * the executor cannot be suspended. Callback must implement BObjectImp*(RequestData*, ...T args).
   * The UOAsyncRequest will become the owner of requestData, deleting it when the the the request
   * is responded to or aborted.
   */
  template <typename Callback, typename RequestData>
  UOAsyncRequest<Callback, RequestData>* makeRequest( Callback* callback,
                                                      RequestData* requestData );
};

template <typename Callback, typename RequestData>
Core::UOAsyncRequest<Callback, RequestData>* UOExecutor::makeRequest( Callback* callback,
                                                                      RequestData* data )
{
  if ( !suspend() )
  {
    if ( data != nullptr )
      delete data;
    return nullptr;
  }
  return new UOAsyncRequest<Callback, RequestData>( this, callback, data );
}

inline bool UOExecutor::listens_to( unsigned int eventflag ) const
{
  return ( eventmask & eventflag ) ? true : false;
}

}  // namespace Core
}  // namespace Pol
#endif
