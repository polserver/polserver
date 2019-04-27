
#include "polosmod.h"
#include "../../../clib/logfacility.h"
#include "../../globals/script_internals.h"
#include "../../uoexec.h"
#include "../napi-wrap.h"
#include "../nodecall.h"


using namespace Pol;
using namespace Napi;

namespace Pol
{
namespace Node
{
namespace Module
{
unsigned int getnewpid( Core::UOExecutor* uoexec )
{
  return Core::scriptScheduler.get_new_pid( uoexec );
}
void freepid( unsigned int pid )
{
  Core::scriptScheduler.free_pid( pid );
}

PolOsModule::PolOsModule( Core::UOExecutor& uoexec )
    : exec( uoexec ), pid_( getnewpid( static_cast<Core::UOExecutor*>( &exec ) ) )
{
}

PolOsModule::~PolOsModule()
{
  freepid( pid_ );
  pid_ = 0;
}


void PolOsModule::suspend( Core::polclock_t sleep_until ) {}

void PolOsModule::revive() {}

bool PolOsModule::signal_event( Bscript::BObjectImp* eventimp )
{
  return Node::emitEvent( &exec, eventimp );
}

bool PolOsModule::critical() const
{
  return true;
}

void PolOsModule::critical( bool critical ) {}

bool PolOsModule::warn_on_runaway() const
{
  return false;
}

void PolOsModule::warn_on_runaway( bool warn_on_runaway ) {}

unsigned char PolOsModule::priority() const
{
  return 100;
}

void PolOsModule::priority( unsigned char priority ) {}

Bscript::BObjectImp* PolOsModule::SleepForMs( int msecs, Bscript::BObjectImp* returnValue )
{
  return nullptr;
}

unsigned int PolOsModule::pid() const
{
  return pid_;
}

bool PolOsModule::blocked() const
{
  return false;
}

bool PolOsModule::in_debugger_holdlist() const
{
  return false;
}

void PolOsModule::revive_debugged() {}

Core::polclock_t PolOsModule::sleep_until_clock() const
{
  return Core::polclock_t();
}

void PolOsModule::sleep_until_clock( Core::polclock_t sleep_until_clock ) {}

Core::TimeoutHandle PolOsModule::hold_itr() const
{
  return Core::TimeoutHandle();
}

void PolOsModule::hold_itr( Core::TimeoutHandle hold_itr ) {}

Core::HoldListType PolOsModule::in_hold_list() const
{
  return Core::HoldListType::NOTIMEOUT_LIST;
}

void PolOsModule::in_hold_list( Core::HoldListType in_hold_list ) {}

Bscript::BObjectImp* PolOsModule::clear_event_queue()
{
  return nullptr;
}

}  // namespace Module
}  // namespace Node
}  // namespace Pol
