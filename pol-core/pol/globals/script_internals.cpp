#include "script_internals.h"

#include <string.h>

#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../plib/systemstate.h"
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

ScriptScheduler::Memory ScriptScheduler::estimateSize( bool verbose ) const
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

  fmt::Writer verbose_w;
  if ( verbose )
    verbose_w << GET_LOG_FILESTAMP << "\n";
  usage.script_size += 3 * sizeof( UOExecutor** ) + runlist.size() * sizeof( UOExecutor* );
  if ( verbose )
    verbose_w << "runlist:\n";
  for ( const auto& exec : runlist )
  {
    if ( exec != nullptr )
    {
      usage.script_size += exec->sizeEstimate();
      if ( verbose )
        verbose_w << exec->scriptname() << " " << exec->sizeEstimate() << "\n";
    }
  }
  usage.script_count += runlist.size();

  usage.script_size += 3 * sizeof( UOExecutor** ) + ranlist.size() * sizeof( UOExecutor* );
  if ( verbose )
    verbose_w << "ranlist:\n";
  for ( const auto& exec : ranlist )
  {
    if ( exec != nullptr )
    {
      usage.script_size += exec->sizeEstimate();
      if ( verbose )
        verbose_w << exec->scriptname() << " " << exec->sizeEstimate() << "\n";
    }
  }
  usage.script_count += ranlist.size();

  if ( verbose )
    verbose_w << "holdlist:\n";
  for ( const auto& hold : holdlist )
  {
    if ( hold.second != nullptr )
    {
      usage.script_size += hold.second->sizeEstimate();
      if ( verbose )
        verbose_w << hold.second->scriptname() << " " << hold.second->sizeEstimate() << "\n";
    }
    usage.script_size += sizeof( Core::polclock_t ) + ( sizeof( void* ) * 3 + 1 ) / 2;
  }
  usage.script_count += holdlist.size();

  usage.script_size += 3 * sizeof( void* );
  if ( verbose )
    verbose_w << "notimeoutholdlist:\n";
  for ( const auto& hold : notimeoutholdlist )
  {
    if ( hold != nullptr )
    {
      usage.script_size += hold->sizeEstimate() + 3 * sizeof( void* );
      if ( verbose )
        verbose_w << hold->scriptname() << " " << hold->sizeEstimate() << "\n";
    }
  }
  usage.script_count += notimeoutholdlist.size();

  usage.script_size += 3 * sizeof( void* );
  if ( verbose )
    verbose_w << "debuggerholdlist:\n";
  for ( const auto& hold : debuggerholdlist )
  {
    if ( hold != nullptr )
    {
      usage.script_size += hold->sizeEstimate() + 3 * sizeof( void* );
      if ( verbose )
        verbose_w << hold->scriptname() << " " << hold->sizeEstimate() << "\n";
    }
  }
  usage.script_count += debuggerholdlist.size();
  if ( verbose )
  {
    auto log = OPEN_FLEXLOG( "log/memoryusagescripts.log", false );
    FLEXLOG( log ) << verbose_w.str();

    CLOSE_FLEXLOG( log );
  }

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

    Clib::scripts_thread_script = ex->scriptname();

    int inscount = 0;
    int totcount = 0;
    int insleft = ex->priority() / priority_divide;
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

      if ( ex->blocked() )
      {
        ex->warn_runaway_on_cycle =
            ex->instr_cycles + Plib::systemstate.config.runaway_script_threshold;
        ex->runaway_cycles = 0;
        break;
      }

      if ( ex->instr_cycles == ex->warn_runaway_on_cycle )
      {
        ex->runaway_cycles += Plib::systemstate.config.runaway_script_threshold;
        if ( ex->warn_on_runaway() )
        {
          fmt::Writer tmp;
          tmp << "Runaway script[" << ex->pid() << "]: " << ex->scriptname() << " ("
              << ex->runaway_cycles << " cycles)\n";
          ex->show_context( tmp, ex->PC );
          SCRIPTLOG << tmp.str();
        }
        ex->warn_runaway_on_cycle += Plib::systemstate.config.runaway_script_threshold;
      }

      if ( ex->critical() )
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

        if ( ( ex->pParent != nullptr ) && ex->pParent->runnable() )
        {
          ranlist.push_back( ex );
          ex->pParent->revive();
        }
        else
        {
          // Check if the script has a child script running
          // Set the parent of the child script nullptr to stop crashing when trying to return to
          // parent script
          if ( ex->pChild != nullptr )
            ex->pChild->pParent = nullptr;
          if ( !ex->keep_alive() )
          {
            delete ex;
          }
          else
          {
            ex->in_hold_list( Core::HoldListType::NOTIMEOUT_LIST );
            notimeoutholdlist.insert( ex );
          }
        }
        continue;
      }
      else if ( !ex->blocked() )
      {
        THREAD_CHECKPOINT( scripts, 115 );

        ex->in_hold_list( Core::HoldListType::DEBUGGER_LIST );
        debuggerholdlist.insert( ex );
        continue;
      }
    }

    if ( ex->blocked() )
    {
      THREAD_CHECKPOINT( scripts, 116 );

      if ( ex->sleep_until_clock() )
      {
        ex->in_hold_list( Core::HoldListType::TIMEOUT_LIST );
        ex->hold_itr( holdlist.insert( HoldList::value_type( ex->sleep_until_clock(), ex ) ) );
      }
      else
      {
        ex->in_hold_list( Core::HoldListType::NOTIMEOUT_LIST );
        notimeoutholdlist.insert( ex );
      }

      --ex->sleep_cycles;  // it'd get counted twice otherwise
      --stateManager.profilevars.sleep_cycles;

      THREAD_CHECKPOINT( scripts, 117 );
    }
    else
    {
      ranlist.push_back( ex );
    }
  }
  THREAD_CHECKPOINT( scripts, 118 );

  runlist.swap( ranlist );
  THREAD_CHECKPOINT( scripts, 119 );
}

void ScriptScheduler::schedule( UOExecutor* exec )
{
  passert_always( exec->in_hold_list() == NO_LIST );
  enqueue( exec );
}

unsigned int ScriptScheduler::get_new_pid( UOExecutor* exec )
{
  for ( ;; )
  {
    unsigned int newpid = next_pid;

    // increase next_pid and wrap so it's always within the positive values that fit an int
    if ( next_pid == INT_MAX )
      next_pid = PID_MIN;
    else
      ++next_pid;

    // NOTE: The code below is pessimistic, is there a way to avoid checking the pidlist every time?
    // (Nando, 06-Nov-2016)

    if ( pidlist.find( newpid ) == pidlist.end() )
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
    *exec = nullptr;
    return false;
  }
}

bool ScriptScheduler::logScriptVariables( const std::string& name ) const
{
  fmt::Writer log;
  log << GET_LOG_FILESTAMP << " " << name << "\n";
  std::vector<Bscript::Executor*> scripts;
  for ( const auto& exec : runlist )
  {
    if ( exec != nullptr && stricmp( exec->scriptname().c_str(), name.c_str() ) == 0 )
      scripts.push_back( exec );
  }
  for ( const auto& exec : ranlist )
  {
    if ( exec != nullptr && stricmp( exec->scriptname().c_str(), name.c_str() ) == 0 )
      scripts.push_back( exec );
  }
  for ( const auto& exec : holdlist )
  {
    if ( exec.second != nullptr && stricmp( exec.second->scriptname().c_str(), name.c_str() ) == 0 )
      scripts.push_back( exec.second );
  }
  for ( const auto& exec : notimeoutholdlist )
  {
    if ( exec != nullptr && stricmp( exec->scriptname().c_str(), name.c_str() ) == 0 )
      scripts.push_back( exec );
  }
  for ( const auto& exec : scripts )
  {
    log << "Size: " << exec->sizeEstimate();
    auto prog = const_cast<Bscript::EScriptProgram*>( exec->prog() );
    if ( prog->read_dbg_file() != 0 )
    {
      log << " failed to load debug info\n";
      break;
    }
    size_t i = 0;
    log << "\nGlobals\n";
    for ( const auto& global : exec->Globals2 )
    {
      log << "  ";
      if ( prog->globalvarnames.size() > i )
        log << prog->globalvarnames[i];
      else
        log << i;
      ++i;
      log << " (" << global->impref().typeOf() << ") " << global->impref().sizeEstimate() << "\n";
    }
    log << "Locals\n";
    auto log_stack = [&]( unsigned PC, Bscript::BObjectRefVec* locals ) {
      log << "  " << prog->dbg_filenames[prog->dbg_filenum[PC]] << ": " << prog->dbg_linenum[PC]
          << "\n";

      unsigned block = prog->dbg_ins_blocks[PC];
      size_t left = locals->size();
      while ( left )
      {
        while ( left <= prog->blocks[block].parentvariables )
        {
          block = prog->blocks[block].parentblockidx;
        }
        const Bscript::EPDbgBlock& progblock = prog->blocks[block];
        size_t varidx = left - 1 - progblock.parentvariables;
        left--;
        Bscript::BObjectImp* ptr = ( *locals )[varidx]->impptr();
        log << "  " << progblock.localvarnames[varidx] << " (" << ptr->typeOf() << ") "
            << ptr->sizeEstimate() << "\n";
      }
    };
    log_stack( exec->PC, exec->Locals2 );
    for ( int stack_i = static_cast<int>( exec->ControlStack.size() ) - 1; stack_i >= 0; --stack_i )
    {
      log << "Stack " << stack_i << "\n";
      log_stack( exec->ControlStack[stack_i].PC, exec->upperLocals2[stack_i] );
    }
  }
  auto logf = OPEN_FLEXLOG( "log/scriptmemory.log", false );
  FLEXLOG( logf ) << log.str();
  CLOSE_FLEXLOG( logf );
  return true;
}

void ScriptScheduler::revive_debugged( UOExecutor* exec )
{
  debuggerholdlist.erase( exec );
  enqueue( exec );
}

void ScriptScheduler::revive_timeout( UOExecutor* exec, TimeoutHandle hold_itr )
{
  holdlist.erase( hold_itr );
  enqueue( exec );
}

void ScriptScheduler::revive_notimeout( UOExecutor* exec )
{
  notimeoutholdlist.erase( exec );
  enqueue( exec );
}

void ScriptScheduler::enqueue( UOExecutor* exec )
{
  runlist.push_back( exec );
}

void ScriptScheduler::free_pid( unsigned int pid )
{
  pidlist.erase( pid );
}
}  // namespace Core
}  // namespace Pol
