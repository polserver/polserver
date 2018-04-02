#include "script_internals.h"

#include <string.h>

#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../plib/systemstate.h"
#include "../module/osmod.h"
#include "../polsig.h"
#include "../uoexec.h"
#include "state.h"

namespace Pol
{
namespace Core
{
ScriptScheduler scriptScheduler;

// This number is intended so that PID and custom GUMPIDS will never clash together
// and to avoid breaking the old assumption that gumpid == pid when gumpid has been
// automatically generated (for backward compatibility).
// Custom gumpids must always be < PID_MIN.
const unsigned int ScriptScheduler::PID_MIN = 0x01000000;

ScriptScheduler::ScriptScheduler()
    : priority_divide( 1 ),
      scrstore(),
      runlist(),
      ranlist(),
      holdlist(),
      notimeoutholdlist(),
      debuggerholdlist(),
      pidlist(),
      next_pid( PID_MIN )
{
}

ScriptScheduler::~ScriptScheduler() {}

// Note, when the program exits, each executor in these queues
// will be deleted by cleanup_scripts()
// Therefore, any object that owns an executor must be destroyed
// before cleanup_scripts() is called.
void ScriptScheduler::deinitialize()
{
  scrstore.clear();
  Clib::delete_all( runlist );
  while ( !holdlist.empty() )
  {
    delete ( ( *holdlist.begin() ).second );
    holdlist.erase( holdlist.begin() );
  }
  while ( !notimeoutholdlist.empty() )
  {
    delete ( *notimeoutholdlist.begin() );
    notimeoutholdlist.erase( notimeoutholdlist.begin() );
  }
  while ( !debuggerholdlist.empty() )
  {
    delete ( *debuggerholdlist.begin() );
    debuggerholdlist.erase( debuggerholdlist.begin() );
  }
}

ScriptScheduler::Memory ScriptScheduler::estimateSize() const
{
  Memory usage;
  memset( &usage, 0, sizeof( usage ) );

  usage.script_size =
      sizeof( int )            /*priority_divide*/
      + sizeof( unsigned int ) /*next_pid*/
      + ( sizeof( unsigned int ) + sizeof( UOExecutor* ) + ( sizeof( void* ) * 3 + 1 ) / 2 ) *
            pidlist.size();

  for ( const auto& script : scrstore )
  {
    usage.scriptstorage_size += ( sizeof( void* ) * 3 + 1 ) / 2;
    usage.scriptstorage_size += script.first.capacity();
    if ( script.second.get() != nullptr )
      usage.scriptstorage_size += script.second->sizeEstimate();
  }
  usage.scriptstorage_count = scrstore.size();


  usage.script_size += 3 * sizeof( UOExecutor** ) + runlist.size() * sizeof( UOExecutor* );
  for ( const auto& exec : runlist )
  {
    if ( exec != nullptr )
      usage.script_size += exec->sizeEstimate();
  }
  usage.script_count += runlist.size();

  usage.script_size += 3 * sizeof( UOExecutor** ) + ranlist.size() * sizeof( UOExecutor* );
  for ( const auto& exec : ranlist )
  {
    if ( exec != nullptr )
      usage.script_size += exec->sizeEstimate();
  }
  usage.script_count += ranlist.size();

  for ( const auto& hold : holdlist )
  {
    if ( hold.second != nullptr )
      usage.script_size += hold.second->sizeEstimate();
    usage.script_size += sizeof( Core::polclock_t ) + ( sizeof( void* ) * 3 + 1 ) / 2;
  }
  usage.script_count += holdlist.size();

  usage.script_size += 3 * sizeof( void* );
  for ( const auto& hold : notimeoutholdlist )
  {
    if ( hold != nullptr )
      usage.script_size += hold->sizeEstimate() + 3 * sizeof( void* );
  }
  usage.script_count += notimeoutholdlist.size();

  usage.script_size += 3 * sizeof( void* );
  for ( const auto& hold : debuggerholdlist )
  {
    if ( hold != nullptr )
      usage.script_size += hold->sizeEstimate() + 3 * sizeof( void* );
  }
  usage.script_count += debuggerholdlist.size();


  return usage;
}


void ScriptScheduler::run_ready()
{
  THREAD_CHECKPOINT( scripts, 110 );
  while ( !runlist.empty() )
  {
    ExecList::iterator itr = runlist.begin();
    UOExecutor* ex = *itr;
    passert_paranoid( ex != nullptr );
    runlist.pop_front();  // remove it directly, since itr can get invalid during execution

    Module::OSExecutorModule* os_module = ex->os_module;
    Clib::scripts_thread_script = ex->scriptname();

    int inscount = 0;
    int totcount = 0;
    int insleft = os_module->priority / priority_divide;
    if ( insleft == 0 )
      insleft = 1;

    THREAD_CHECKPOINT( scripts, 111 );

    while ( ex->runnable() )
    {
      ++ex->instr_cycles;
      THREAD_CHECKPOINT( scripts, 112 );
      Clib::scripts_thread_scriptPC = ex->PC;
      ex->execInstr();

      THREAD_CHECKPOINT( scripts, 113 );

      if ( os_module->blocked() )
      {
        ex->warn_runaway_on_cycle =
            ex->instr_cycles + Plib::systemstate.config.runaway_script_threshold;
        ex->runaway_cycles = 0;
        break;
      }

      if ( ex->instr_cycles == ex->warn_runaway_on_cycle )
      {
        ex->runaway_cycles += Plib::systemstate.config.runaway_script_threshold;
        if ( os_module->warn_on_runaway )
        {
          fmt::Writer tmp;
          tmp << "Runaway script[" << os_module->pid() << "]: " << ex->scriptname() << " ("
              << ex->runaway_cycles << " cycles)\n";
          ex->show_context( tmp, ex->PC );
          SCRIPTLOG << tmp.str();
        }
        ex->warn_runaway_on_cycle += Plib::systemstate.config.runaway_script_threshold;
      }

      if ( os_module->critical )
      {
        ++inscount;
        ++totcount;
        if ( inscount > 1000 )
        {
          inscount = 0;
          if ( Plib::systemstate.config.report_critical_scripts )
          {
            fmt::Writer tmp;
            tmp << "Critical script " << ex->scriptname() << " has run for " << totcount
                << " instructions\n";
            ex->show_context( tmp, ex->PC );
            ERROR_PRINT << tmp.str();
          }
        }
        continue;
      }

      if ( !--insleft )
      {
        break;
      }
    }

    // hmm, this new terminology (runnable()) is confusing
    // in this case.  Technically, something that is blocked
    // isn't runnable.
    if ( !ex->runnable() )
    {
      if ( ex->error() || ex->done )
      {
        THREAD_CHECKPOINT( scripts, 114 );

        if ( ( ex->pParent != NULL ) && ex->pParent->runnable() )
        {
          ranlist.push_back( ex );
          // ranlist.splice( ranlist.end(), runlist, itr );
          ex->pParent->os_module->revive();
        }
        else
        {
          // runlist.erase( itr );
          // Check if the script has a child script running
          // Set the parent of the child script NULL to stop crashing when trying to return to
          // parent script
          if ( ex->pChild != NULL )
            ex->pChild->pParent = NULL;

          delete ex;
        }
        continue;
      }
      else if ( !ex->os_module->blocked() )
      {
        THREAD_CHECKPOINT( scripts, 115 );

        // runlist.erase( itr );
        ex->os_module->in_hold_list_ = Module::OSExecutorModule::DEBUGGER_LIST;
        debuggerholdlist.insert( ex );
        continue;
      }
    }

    if ( ex->os_module->blocked() )
    {
      THREAD_CHECKPOINT( scripts, 116 );

      if ( ex->os_module->sleep_until_clock_ )
      {
        ex->os_module->in_hold_list_ = Module::OSExecutorModule::TIMEOUT_LIST;
        ex->os_module->hold_itr_ =
            holdlist.insert( HoldList::value_type( ex->os_module->sleep_until_clock_, ex ) );
      }
      else
      {
        ex->os_module->in_hold_list_ = Module::OSExecutorModule::NOTIMEOUT_LIST;
        notimeoutholdlist.insert( ex );
      }

      // runlist.erase( itr );
      --ex->sleep_cycles;  // it'd get counted twice otherwise
      --stateManager.profilevars.sleep_cycles;

      THREAD_CHECKPOINT( scripts, 117 );
    }
    else
    {
      ranlist.push_back( ex );
      // ranlist.splice( ranlist.end(), runlist, itr );
    }
  }
  THREAD_CHECKPOINT( scripts, 118 );

  runlist.swap( ranlist );
  THREAD_CHECKPOINT( scripts, 119 );
}

void ScriptScheduler::schedule( UOExecutor* exec )
{
  exec->setDebugLevel( Bscript::Executor::NONE );
  enqueue( exec );
}

unsigned int ScriptScheduler::get_new_pid( UOExecutor* exec )
{
  for ( ;; )
  {
    unsigned int newpid = next_pid++;
    if ( newpid < PID_MIN )
      newpid = PID_MIN;

    // NOTE: The code below is pessimistic, is there a way to avoid checking the pidlist every time?
    // (Nando, 06-Nov-2016)

    // newpid=0 should now never happen but leaving this
    // check in place for extra code robustness
    if ( newpid != 0 && ( pidlist.find( newpid ) == pidlist.end() ) )
    {
      pidlist[newpid] = exec;
      return newpid;
    }
  }
}

bool ScriptScheduler::find_exec( unsigned int pid, UOExecutor** exec )
{
  auto itr = pidlist.find( pid );
  if ( itr != pidlist.end() )
  {
    *exec = ( *itr ).second;
    return true;
  }
  else
  {
    *exec = NULL;
    return false;
  }
}
}
}
