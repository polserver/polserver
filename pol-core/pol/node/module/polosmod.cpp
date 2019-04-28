
#include "polosmod.h"
#include "../../../bscript/berror.h"
#include "../../../clib/logfacility.h"
#include "../../globals/script_internals.h"
#include "../../uoasynchandler.h"
#include "../../uoexec.h"
#include "../napi-wrap.h"
#include "../nodecall.h"
#include "objwrap.h"


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


void PolOsModule::suspend( Core::polclock_t sleep_until )
{
  /* A node script cannot be suspended really. */
}

void PolOsModule::revive() {}


bool PolOsModule::signal_event( Bscript::BObjectImp* eventimp, Core::ULWObject* target )
{
  return Node::emitEvent( &exec, eventimp, target );
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

using ResolvedDelayedObject = std::pair<Bscript::DelayedObject*, Bscript::BObjectImp*>;

// Returns a new DelayedObject of a promise that resolves in msecs milliseconds with the
// wrapped value of returnValue
Bscript::BObjectImp* PolOsModule::SleepForMs( int msecs, Bscript::BObjectImp* returnValue )
{
  auto sleep_until = Core::polclock() + msecs * Core::POLCLOCKS_PER_SEC / 1000;
  auto script = Node::GetRunningScript( &exec );
  Napi::Env env = script.Env();
  if ( env != nullptr )
  {
    auto delayedObj = new Bscript::DelayedObject( Core::UOAsyncRequest::nextRequestId++ );

    auto timeoutId =
        env.Global()
            .Get( "setTimeout" )
            .As<Function>()
            .Call( env.Global(),
                   {Napi::Function::New( env,
                                         [=]( const CallbackInfo& cbinfo ) {
                                           NODELOG.Format( "[{:04x}] [exec] resolving with {}\n" )
                                               << delayedObj->reqId()
                                               << returnValue->getStringRep();
                                           NodeObjectWrap::resolveDelayedObject(
                                               delayedObj->reqId(),
                                               Bscript::BObjectRef( returnValue ), true );
                                         },
                                         "ResolveDelayTimeoutCallback" ),
                    Number::New( env, msecs )} );
    NODELOG.Format( "[{:04x}] [exec] sleeping for {} ms, timeout = {}\n" )
        << delayedObj->reqId() << msecs << timeoutId.ToNumber().Int32Value();

    return delayedObj;
  }

  else
  {
    return new Bscript::BError( "Script is not running?" );
  }
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
