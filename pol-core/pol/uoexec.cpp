#include "uoexec.h"

#include <stddef.h>

#include "../bscript/executor.h"
#include "../clib/logfacility.h"
#include "../plib/systemstate.h"
#include "globals/settings.h"
#include "module/osmod.h"
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
      pChild( nullptr )
{
  weakptr.set( this );
  os_module = new Module::OSExecutorModule( *this );
  addModule( os_module );
}

UOExecutor::~UOExecutor()
{
  // note, the os_module isn't deleted here because
  // the Executor deletes its ExecutorModules.
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
}  // namespace Core
}  // namespace Pol
