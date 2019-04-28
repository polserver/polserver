#ifndef NODE_POLOSMOD_H
#define NODE_POLOSMOD_H


#include "../../globals/script_internals.h"
#include "../../module/threadmod.h"
#include "../napi-wrap.h"

namespace Pol
{
namespace Node
{
namespace Module
{
using namespace Napi;

class PolOsModule : public Pol::Module::ThreadInterface
{
public:
  Core::UOExecutor& exec;  // for now...
  // thread interface
public:
  PolOsModule( Core::UOExecutor& uoexec );
  PolOsModule() = delete;
  ~PolOsModule();
  virtual void suspend( Core::polclock_t sleep_until ) override;
  virtual void revive() override;
  virtual bool signal_event( Bscript::BObjectImp* eventimp ) override;
  virtual bool signal_event( Bscript::BObjectImp* target, Bscript::BObjectImp* eventimp ) override;

  virtual bool critical() const override;
  virtual void critical( bool critical ) override;
  virtual bool warn_on_runaway() const override;
  virtual void warn_on_runaway( bool warn_on_runaway ) override;
  virtual unsigned char priority() const override;
  virtual void priority( unsigned char priority ) override;
  virtual Bscript::BObjectImp* SleepForMs( int msecs, Bscript::BObjectImp* returnValue ) override;
  virtual unsigned int pid() const override;
  virtual bool blocked() const override;
  virtual bool in_debugger_holdlist() const override;
  virtual void revive_debugged() override;
  virtual Core::polclock_t sleep_until_clock() const override;
  virtual void sleep_until_clock( Core::polclock_t sleep_until_clock ) override;
  virtual Core::TimeoutHandle hold_itr() const override;
  virtual void hold_itr( Core::TimeoutHandle hold_itr ) override;
  virtual Core::HoldListType in_hold_list() const override;
  virtual void in_hold_list( Core::HoldListType in_hold_list ) override;
  virtual Bscript::BObjectImp* clear_event_queue() override;

private:
  unsigned int pid_;
};
}  // namespace Module
}  // namespace Node
}  // namespace Pol

#endif
