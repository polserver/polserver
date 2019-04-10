#include "uoexec.h"

#include <stddef.h>

#include "../bscript/executor.h"
#include "../clib/logfacility.h"
#include "../plib/systemstate.h"
#include "globals/settings.h"
#include "module/osmod.h"
#include "node/jsprog.h"
#include "node/nodecall.h"
#include "polcfg.h"
#include "polclock.h"

namespace Pol
{
namespace Core
{
UOExecutor::UOExecutor()
    : Executor(),
      os_module( nullptr ),
      instr_cycles( 0 ),
      sleep_cycles( 0 ),
      start_time( poltime() ),
      warn_runaway_on_cycle( Plib::systemstate.config.runaway_script_threshold ),
      runaway_cycles( 0 ),
      eventmask( 0 ),
      area_size( 0 ),
      speech_size( 1 ),
      can_access_offline_mobiles( false ),
      auxsvc_assume_string( false ),
      pParent( nullptr ),
      pChild( nullptr ),
      requests()
{
  weakptr.set( this );
  os_module = new Module::OSExecutorModule( *this );
  addModule( os_module );
}

bool UOExecutor::setProgram( Bscript::Program* i_prog )
{
  prog_.set( i_prog );
  if ( i_prog->type() == Bscript::Program::ProgramType::ESCRIPT )
    return Executor::setProgram( static_cast<Bscript::EScriptProgram*>( i_prog ) );
  else
  {
    // return Executor::setProgram( static_cast<Bscript::EScriptProgram*>( i_prog ) );
    return true;
  }
}

bool UOExecutor::runnable() const
{
  if ( prog_->type() == Bscript::Program::ProgramType::ESCRIPT )
    return Executor::runnable();
  else
  {
    return prog_->hasProgram();
  }
}

template <typename R>
bool is_ready( std::future<R> const& f )
{
  return f.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready;
}

std::string UOExecutor::scriptname() const
{
  return prog_->scriptname();
}


UOExecutor::~UOExecutor()
{
  // note, the os_module isn't deleted here because
  // the Executor deletes its ExecutorModules.
  for ( auto& req : requests )
  {
    std::get<0>( req.second )->abort();
    ObjectReference& ref = std::get<1>( req.second );

    if (!ref.IsEmpty())
    {
      auto call = Node::makeCall<bool>( [&]( Napi::Env env, Node::NodeRequest<bool>* request ) {
        ref.Value().As<Promise>();
        ref.Unref();
        return true;
      });
      bool success = call.getRef();

    }

    
  }
  requests.clear();

  if ( ( instr_cycles >= 500 ) && settingsManager.watch.profile_scripts )
  {
    int elapsed = static_cast<int>(
        poltime() - start_time );  // Doh! A script can't run more than 68 years, for this to work.
    POLLOG_ERROR.Format( "Script {}: {} instr cycles, {} sleep cycles, {} seconds\n" )
        << scriptname() << instr_cycles << sleep_cycles << elapsed;
  }

  pParent = nullptr;
  pChild = nullptr;
}

void UOExecutor::addRequest( ref_ptr<Core::UOAsyncRequest> req )
{
//  requests.emplace( reqreq );
  if ( programType() == Bscript::Program::ProgramType::JAVASCRIPT )
  {
    //auto call = Node::makeCall<Napi::Promise::Deferred>(
    //    []( Napi::Env env, Node::NodeRequest<Napi::Promise::Deferred>* /*request*/ ) {
    //      Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New( env );
    //      // return Persistent<Promise>( deferred.Promise() );
    //      return deferred;
    //    } );

    //Napi::Promise::Deferred ref = call.getRef();
  }
}

void UOExecutor::handleRequest( Core::UOAsyncRequest* req, Bscript::BObjectImp* resp )
{
  if ( resp != nullptr )
    ValueStack.back().set( new Bscript::BObject( resp ) );
  revive();
 // auto iter = requests.find( req );
}

bool UOExecutor::suspend()
{
  // Run to completion scripts can't be suspended
  if ( running_to_completion() )
    return false;

  os_module->suspend();
  return true;
}

bool UOExecutor::revive()
{
  os_module->revive();
  return true;
}

std::string UOExecutor::state()
{
  if ( halt() )
    return "Debugging";
  else if ( os_module->blocked() )
    return "Sleeping";
  else
    return "Running";
}


bool UOExecutor::signal_event( Bscript::BObjectImp* eventimp )
{
  passert_r( os_module != nullptr, "Object cannot receive events but is receiving them!" );
  return os_module->signal_event( eventimp );
}

size_t UOExecutor::sizeEstimate() const
{
  return sizeof( UOExecutor ) + base::sizeEstimate();
}

bool UOExecutor::critical() const
{
  return os_module->critical();
}
void UOExecutor::critical( bool critical )
{
  os_module->critical( critical );
}

bool UOExecutor::warn_on_runaway() const
{
  return os_module->warn_on_runaway();
}
void UOExecutor::warn_on_runaway( bool warn_on_runaway )
{
  os_module->warn_on_runaway( warn_on_runaway );
}

unsigned char UOExecutor::priority() const
{
  return os_module->priority();
}
void UOExecutor::priority( unsigned char priority )
{
  os_module->priority( priority );
}

void UOExecutor::SleepFor( int secs )
{
  os_module->SleepFor( secs );
}
void UOExecutor::SleepForMs( int msecs )
{
  os_module->SleepForMs( msecs );
}
unsigned int UOExecutor::pid() const
{
  return os_module->pid();
}
bool UOExecutor::blocked() const
{
  return os_module->blocked();
}
bool UOExecutor::in_debugger_holdlist() const
{
  return os_module->in_debugger_holdlist();
}
void UOExecutor::revive_debugged()
{
  os_module->revive_debugged();
}

Core::polclock_t UOExecutor::sleep_until_clock() const
{
  return os_module->sleep_until_clock();
}

void UOExecutor::sleep_until_clock( Core::polclock_t sleep_until_clock )
{
  os_module->sleep_until_clock( sleep_until_clock );
}

Core::TimeoutHandle UOExecutor::hold_itr() const
{
  return os_module->hold_itr();
}

void UOExecutor::hold_itr( Core::TimeoutHandle hold_itr )
{
  os_module->hold_itr( hold_itr );
}

Core::HoldListType UOExecutor::in_hold_list() const
{
  return os_module->in_hold_list();
}
void UOExecutor::in_hold_list( Core::HoldListType in_hold_list )
{
  return os_module->in_hold_list( in_hold_list );
}

Bscript::BObjectImp* UOExecutor::clear_event_queue()
{
  return os_module->clear_event_queue();
}  // DAVE
}  // namespace Core
}  // namespace Pol
