/** @file
 *
 * @par History
 * - 2006/09/17 Shinigami: OSExecutorModule::signal_event() will return error on full evene queue
 */


#ifndef BSCRIPT_THREADEMOD_H
#define BSCRIPT_THREADEMOD_H

#ifndef BSCRIPT_EXECMODL_H
#include "../../bscript/execmodl.h"
#endif

#include "../globals/script_internals.h"

namespace Pol
{

namespace Module
{

class ThreadInterface
{
public:
  virtual void suspend( Core::polclock_t sleep_until ) = 0;
  virtual void revive() = 0;
  virtual bool signal_event( Bscript::BObjectImp* eventimp ) = 0;
  virtual bool critical() const = 0;
  virtual void critical( bool critical ) = 0;
  virtual bool warn_on_runaway() const = 0;
  virtual void warn_on_runaway( bool warn_on_runaway ) = 0;
  virtual unsigned char priority() const = 0;
  virtual void priority( unsigned char priority ) = 0;
  virtual Bscript::BObjectImp* SleepForMs( int msecs, Bscript::BObjectImp* returnValue ) = 0;
  virtual unsigned int pid() const = 0;
  virtual bool blocked() const = 0;
  virtual bool in_debugger_holdlist() const = 0;
  virtual void revive_debugged() = 0;
  virtual Core::polclock_t sleep_until_clock() const = 0;
  virtual void sleep_until_clock( Core::polclock_t sleep_until_clock ) = 0;
  virtual Core::TimeoutHandle hold_itr() const = 0;
  virtual void hold_itr( Core::TimeoutHandle hold_itr ) = 0;
  virtual Core::HoldListType in_hold_list() const = 0;
  virtual void in_hold_list( Core::HoldListType in_hold_list ) = 0;
  virtual Bscript::BObjectImp* clear_event_queue() = 0;
};
}  // namespace Module
}  // namespace Pol


#endif
