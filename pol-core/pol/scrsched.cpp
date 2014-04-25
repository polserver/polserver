/*
History
=======
2005/09/16 Shinigami: added scripts_thread_script* to support better debugging
2006/05/11 Shinigami: better logging in ::signal_event()
2006/05/27 Shinigami: fixed a smaller cout-bug
2006/09/17 Shinigami: ::signal_event() will return error on full evene queue
2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now
2009/07/28 MuadDib:   Packet Struct Refactoring
2009/09/03 MuadDib:   Relocation of boat related cpp/h
2010/02/04 Turley:    "Event queue full" cerr only if loglevel>=11
                      polcfg.discard_old_events discards oldest event if queue is full

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <time.h>

#include "module/basiciomod.h"
#include "module/basicmod.h"
#include "../bscript/bobject.h"
#include "../bscript/berror.h"
#include "../bscript/eprog.h"
#include "../bscript/executor.h"
#include "../bscript/impstr.h"

#include "../clib/logfacility.h"
#include "../clib/endian.h"
#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/unicode.h"

#include "module/attributemod.h"
#include "module/boatmod.h"
#include "module/cfgmod.h"
#include "module/clmod.h"
#include "mobile/charactr.h"
#include "module/datastore.h"
#include "module/sqlmod.h"
#include "exscrobj.h"
#include "module/filemod.h"
#include "module/guildmod.h"
#include "module/mathmod.h"
#include "npc.h"
#include "module/npcmod.h"
#include "module/osmod.h"
#include "polcfg.h"
#include "polclock.h"
#include "poldbg.h"
#include "polsig.h"
#include "module/polsystemmod.h"
#include "profile.h"
#include "scrdef.h"
#include "scrstore.h"
#include "module/storagemod.h"
#include "module/unimod.h"
#include "module/uomod.h"
#include "uoexec.h"
#include "module/utilmod.h"
#include "module/vitalmod.h"
#include "watch.h"

#include "mobile/charactr.h"
#include "network/client.h"

#include "scrsched.h"

#include "module/partymod.h"
namespace Pol {
  namespace Core {
    bool find_uoexec( unsigned int pid, UOExecutor** pp_uoexec )
    {
      std::map<unsigned int, UOExecutor*>::iterator itr = Module::pidlist.find( pid );
      if ( itr != Module::pidlist.end() )
      {
        *pp_uoexec = ( *itr ).second;
        return true;
      }
      else
      {
        *pp_uoexec = NULL;
        return false;
      }
    }
  }
  namespace Module {
    using namespace Bscript;
    bool OSExecutorModule::signal_event( BObjectImp* imp )
    {
      INC_PROFILEVAR( events );

      if ( blocked_ && ( wait_type == WAIT_EVENT ) ) // already being waited for
      {
        /* Now, the tricky part.  The value to return on an error or
        completion condition has already been pushed onto the value
        stack - so, we need to replace it with the real result.
        */
        exec.ValueStack.top().set( new BObject( imp ) );
        /* This executor will get moved to the run queue at the
        next step_scripts(), where blocked is checked looking
        for timed out or completed operations. */

        revive();
      }
      else        // not being waited for, so queue for later.
      {
        if ( events_.size() < max_eventqueue_size )
        {
          events_.push( imp );
        }
        else
        {
          if ( Core::config.discard_old_events )
          {
            BObject ob( events_.front() );
            events_.pop();
            events_.push( imp );
          }
          else
          {
            BObject ob( imp );
            if ( Core::config.loglevel >= 11 )
            {
              INFO_PRINT << "Event queue for " << exec.scriptname() << " is full, discarding event.\n";
              ExecutorModule* em = exec.findModule( "npc" );
              if ( em )
              {
                NPCExecutorModule* npcemod = static_cast<NPCExecutorModule*>( em );
                INFO_PRINT << "NPC Serial: " << fmt::hexu( npcemod->npc.serial ) <<
                  " (" << npcemod->npc.x << " " << npcemod->npc.y << " " << npcemod->npc.z << ")\n";
              }

              INFO_PRINT << "Event: " << ob->getStringRep() << "\n";
            }
            return false;
          }
        }
      }

      return true;
    }

    void OSExecutorModule::SleepFor( int nsecs )
    {
      if ( nsecs )
      {
        blocked_ = true;
        wait_type = WAIT_SLEEP;
        sleep_until_clock_ = Core::polclock() + nsecs * Core::POLCLOCKS_PER_SEC;
      }
    }

    void OSExecutorModule::SleepForMs( int msecs )
    {
      if ( msecs )
      {
        blocked_ = true;
        wait_type = WAIT_SLEEP;
        sleep_until_clock_ = Core::polclock() + msecs * Core::POLCLOCKS_PER_SEC / 1000;
      }
    }

    void OSExecutorModule::suspend()
    {
      blocked_ = true;
      wait_type = WAIT_SLEEP;
      sleep_until_clock_ = 0; // wait forever
    }

    void OSExecutorModule::revive()
    {
      blocked_ = false;
      if ( in_hold_list_ == TIMEOUT_LIST )
      {
        Core::holdlist.erase( hold_itr_ );
        in_hold_list_ = NO_LIST;
        Core::runlist.push_back( static_cast<Core::UOExecutor*>( &exec ) );
      }
      else if ( in_hold_list_ == NOTIMEOUT_LIST )
      {
        Core::notimeoutholdlist.erase( static_cast<Core::UOExecutor*>( &exec ) );
        in_hold_list_ = NO_LIST;
        Core::runlist.push_back( static_cast<Core::UOExecutor*>( &exec ) );
      }
      else if ( in_hold_list_ == DEBUGGER_LIST )
      {
        // stays right where it is.
      }
    }
    bool OSExecutorModule::in_debugger_holdlist() const
    {
      return ( in_hold_list_ == DEBUGGER_LIST );
    }
    void OSExecutorModule::revive_debugged()
    {
      Core::debuggerholdlist.erase( static_cast<Core::UOExecutor*>( &exec ) );
      in_hold_list_ = NO_LIST;
      Core::runlist.push_back( static_cast<Core::UOExecutor*>( &exec ) );
    }
  }
  namespace Core {

	UOExecutor::UOExecutor() :
	  Executor(),
	  os_module( NULL ),
	  instr_cycles( 0 ),
	  sleep_cycles( 0 ),
	  start_time( poltime() ),
	  warn_runaway_on_cycle( config.runaway_script_threshold ),
	  runaway_cycles( 0 ),
	  eventmask( 0 ),
	  area_size( 0 ),
	  speech_size( 1 ),
	  can_access_offline_mobiles( false ),
	  auxsvc_assume_string( false ),
	  pParent( NULL ),
	  pChild( NULL )
	{
	  weakptr.set( this );
	  os_module = new Module::OSExecutorModule( *this );
	  addModule( os_module );
	}

	UOExecutor::~UOExecutor()
	{
	  // note, the os_module isn't deleted here because
	  // the Executor deletes its ExecutorModules.
	  if ( ( instr_cycles >= 500 ) && watch.profile_scripts )
	  {
		int elapsed = static_cast<int>( poltime() - start_time ); // Doh! A script can't run more than 68 years, for this to work.
        POLLOG_ERROR.Format( "Script {}: {} instr cycles, {} sleep cycles, {} seconds\n" )
          << scriptname() << instr_cycles << sleep_cycles << elapsed;
	  }

	  pParent = NULL;
	  pChild = NULL;
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


	// Note, when the program exits, each executor in these queues
	// will be deleted by cleanup_scripts()
	// Therefore, any object that owns an executor must be destroyed 
	// before cleanup_scripts() is called.

	ExecList runlist;
	ExecList ranlist;


	Module::HoldList holdlist;
	NoTimeoutHoldList notimeoutholdlist;
	NoTimeoutHoldList debuggerholdlist;

	int priority_divide = 1;

	void cleanup_scripts()
	{
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


	void run_ready()
	{
	  THREAD_CHECKPOINT( scripts, 110 );
	  while ( !runlist.empty() )
	  {
		ExecList::iterator itr = runlist.begin();
		UOExecutor* ex = *itr;
		runlist.pop_front(); // remove it directly, since itr can get invalid during execution
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

		  if ( os_module->blocked_ )
		  {
			ex->warn_runaway_on_cycle = ex->instr_cycles + config.runaway_script_threshold;
			ex->runaway_cycles = 0;
			break;
		  }

		  if ( ex->instr_cycles == ex->warn_runaway_on_cycle )
		  {
			ex->runaway_cycles += config.runaway_script_threshold;
			if ( os_module->warn_on_runaway )
			{
              fmt::Writer tmp;
              tmp << "Runaway script[" << os_module->pid( ) << "]: " << ex->scriptname( )
                << " (" << ex->runaway_cycles << " cycles)\n";
              ex->show_context( tmp, ex->PC );
              SCRIPTLOG << tmp.c_str();
			}
			ex->warn_runaway_on_cycle += config.runaway_script_threshold;
		  }

		  if ( os_module->critical )
		  {
			++inscount;
			++totcount;
			if ( inscount > 1000 )
			{
			  inscount = 0;
              if ( config.report_critical_scripts )
              {
                fmt::Writer tmp;
                tmp << "Critical script " << ex->scriptname() << " has run for " << totcount << " instructions\n";
                ex->show_context( tmp, ex->PC );
                ERROR_PRINT << tmp.c_str();
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
			  //ranlist.splice( ranlist.end(), runlist, itr );
			  ex->pParent->os_module->revive();
			}
			else
			{
			  //runlist.erase( itr );
				// Check if the script has a child script running
				// Set the parent of the child script NULL to stop crashing when trying to return to parent script
				if ( ex->pChild != NULL )
				ex->pChild->pParent = NULL;

			    delete ex;
			}
			continue;
		  }
		  else if ( !ex->os_module->blocked_ )
		  {
			THREAD_CHECKPOINT( scripts, 115 );

			//runlist.erase( itr );
			ex->os_module->in_hold_list_ = Module::OSExecutorModule::DEBUGGER_LIST;
			debuggerholdlist.insert( ex );
			continue;
		  }
		}

		if ( ex->os_module->blocked_ )
		{
		  THREAD_CHECKPOINT( scripts, 116 );

		  if ( ex->os_module->sleep_until_clock_ )
		  {
            ex->os_module->in_hold_list_ = Module::OSExecutorModule::TIMEOUT_LIST;
			ex->os_module->hold_itr_ = holdlist.insert( Module::HoldList::value_type( ex->os_module->sleep_until_clock_, ex ) );
		  }
		  else
		  {
            ex->os_module->in_hold_list_ = Module::OSExecutorModule::NOTIMEOUT_LIST;
			notimeoutholdlist.insert( ex );
		  }

		  //runlist.erase( itr );
		  --ex->sleep_cycles; // it'd get counted twice otherwise
		  --sleep_cycles;

		  THREAD_CHECKPOINT( scripts, 117 );
		}
		else
		{
		  ranlist.push_back( ex );
		  //ranlist.splice( ranlist.end(), runlist, itr );
		}
	  }
	  THREAD_CHECKPOINT( scripts, 118 );

	  runlist.swap( ranlist );
	  THREAD_CHECKPOINT( scripts, 119 );
	}


	void check_blocked( polclock_t* pclocksleft )
	{
	  polclock_t now_clock = polclock();
	  sleep_cycles += holdlist.size() + notimeoutholdlist.size();
	  polclock_t clocksleft = POLCLOCKS_PER_SEC * 60;
	  for ( ;; )
	  {
		THREAD_CHECKPOINT( scripts, 131 );

        Module::HoldList::iterator itr = holdlist.begin( );
		if ( itr == holdlist.end() )
		  break;

		UOExecutor* ex = ( *itr ).second;
		// ++ex->sleep_cycles;

		passert( ex->os_module->blocked_ );
		passert( ex->os_module->sleep_until_clock_ != 0 );
		clocksleft = ex->os_module->sleep_until_clock_ - now_clock;
		if ( clocksleft <= 0 )
		{
		  if ( clocksleft == 0 )
			INC_PROFILEVAR( scripts_ontime );
		  else
			INC_PROFILEVAR( scripts_late );
		  // wakey-wakey
		  // read comment above to understand what goes on here.
		  // the return value is already on the stack.
		  THREAD_CHECKPOINT( scripts, 132 );
		  ex->os_module->revive();
		}
		else
		{
		  break;
		}
	  }
	  *pclocksleft = clocksleft;
	}

	polclock_t calc_script_clocksleft( polclock_t now )
	{
	  if ( !runlist.empty() )
	  {
		return 0; // we want to run immediately
	  }
	  else if ( !holdlist.empty() )
	  {
        Module::HoldList::iterator itr = holdlist.begin( );
		UOExecutor* ex = ( *itr ).second;
		polclock_t clocksleft = ex->os_module->sleep_until_clock_ - now;
		if ( clocksleft >= 0 )
		  return clocksleft;
		else
		  return 0;

	  }
	  else
	  {
		return -1;
	  }
	}

	void step_scripts( polclock_t* clocksleft, bool* pactivity )
	{
	  THREAD_CHECKPOINT( scripts, 102 );
	  *pactivity = ( !runlist.empty() );
	  THREAD_CHECKPOINT( scripts, 103 );

	  run_ready();

	  THREAD_CHECKPOINT( scripts, 104 );

	  check_blocked( clocksleft );
	  THREAD_CHECKPOINT( scripts, 105 );
	  if ( !runlist.empty() )
		*clocksleft = 0;
	  THREAD_CHECKPOINT( scripts, 106 );
	}

	void start_script( const char *filename,
					   Bscript::BObjectImp* param0,
                       Bscript::BObjectImp* param1 )
	{
      Bscript::BObject bobj0( param0 ); // just to delete if it doesn't go somewhere else
      Bscript::BObject bobj1( param1 ? param1 : Bscript::UninitObject::create( ) );

      ref_ptr<Bscript::EScriptProgram> program = find_script( filename );
	  if ( program.get() == NULL )
	  {
        ERROR_PRINT << "Error reading script " << filename << "\n";
		throw runtime_error( "Error starting script" );
	  }

	  UOExecutor* ex = create_script_executor();
	  if ( program->haveProgram )
	  {
		if ( param1 )
		  ex->pushArg( param1 );
		if ( param0 )
		  ex->pushArg( param0 );
	  }
	  //ex->addModule( new FileExecutorModule( *ex ) );	
	  ex->addModule( new Module::UOExecutorModule( *ex ) );

	  if ( !ex->setProgram( program.get() ) )
		throw runtime_error( "Error starting script." );

	  ex->setDebugLevel( Bscript::Executor::NONE );


	  runlist.push_back( ex );
	}
	// EXACTLY the same as start_script, except uses find_script2
    Module::UOExecutorModule* start_script( const ScriptDef& script, Bscript::BObjectImp* param ) throw( )
	{
      Bscript::BObject bobj( param ? param : Bscript::UninitObject::create( ) ); // just to delete if it doesn't go somewhere else
      ref_ptr<Bscript::EScriptProgram> program = find_script2( script );
	  if ( program.get() == NULL )
	  {
        ERROR_PRINT << "Error reading script " << script.name( ) << "\n";
		// throw runtime_error( "Error starting script" );
		return NULL;
	  }

	  std::unique_ptr<UOExecutor> ex( create_script_executor() );
	  if ( program->haveProgram && ( param != NULL ) )
	  {
		ex->pushArg( param );
	  }
	  //ex->addModule( new FileExecutorModule( *ex ) );	
	  Module::UOExecutorModule* uoemod = new Module::UOExecutorModule( *ex );
	  ex->addModule( uoemod );

	  if ( !ex->setProgram( program.get() ) )
	  {
		return NULL;
		//throw runtime_error( "Error starting script." );
	  }

      ex->setDebugLevel( Bscript::Executor::NONE );


	  runlist.push_back( ex.release() );

	  return uoemod;
	}

	Module::UOExecutorModule* start_script( const ScriptDef& script,
                                            Bscript::BObjectImp* param0,
                                            Bscript::BObjectImp* param1,
                                            Bscript::BObjectImp* param2,
                                            Bscript::BObjectImp* param3 ) throw( )
	{
      Bscript::BObject bobj0( param0 ); // just to delete if it doesn't go somewhere else
      Bscript::BObject bobj1( param1 );
      Bscript::BObject bobj2( param2 ? param2 : Bscript::UninitObject::create( ) );
      Bscript::BObject bobj3( param3 ? param3 : Bscript::UninitObject::create( ) );

      ref_ptr<Bscript::EScriptProgram> program = find_script2( script );
	  if ( program.get() == NULL )
	  {
        ERROR_PRINT << "Error reading script " << script.name( ) << "\n";
		// throw runtime_error( "Error starting script" );
		return NULL;
	  }

	  std::unique_ptr<UOExecutor> ex( create_script_executor() );
	  if ( program->haveProgram )
	  {
		if ( param3 != NULL )
		  ex->pushArg( param3 );
		if ( param2 != NULL )
		  ex->pushArg( param2 );
		if ( param1 != NULL )
		  ex->pushArg( param1 );
		if ( param0 != NULL )
		  ex->pushArg( param0 );
	  }
	  //ex->addModule( new FileExecutorModule( *ex ) );	
	  Module::UOExecutorModule* uoemod = new Module::UOExecutorModule( *ex );
	  ex->addModule( uoemod );

	  if ( !ex->setProgram( program.get() ) )
	  {
		return NULL;
		//throw runtime_error( "Error starting script." );
	  }

      ex->setDebugLevel( Bscript::Executor::NONE );


	  runlist.push_back( ex.release() );

	  return uoemod;
	}

	// EXACTLY the same as start_script, except uses find_script2
    Module::UOExecutorModule* start_script( ref_ptr<Bscript::EScriptProgram> program, Bscript::BObjectImp* param )
	{
      Bscript::BObject bobj( param ? param : Bscript::UninitObject::create( ) ); // just to delete if it doesn't go somewhere else

	  UOExecutor* ex = create_script_executor();
	  if ( program->haveProgram && ( param != NULL ) )
	  {
		ex->pushArg( param );
	  }
	  //ex->addModule( new FileExecutorModule( *ex ) );	
	  Module::UOExecutorModule* uoemod = new Module::UOExecutorModule( *ex );
	  ex->addModule( uoemod );

	  if ( !ex->setProgram( program.get() ) )
		throw runtime_error( "Error starting script." );

      ex->setDebugLevel( Bscript::Executor::NONE );

	  runlist.push_back( ex );

	  return uoemod;
	}

    void add_common_exmods( Bscript::Executor& ex )
	{
      using namespace Module;
	  ex.addModule( new BasicExecutorModule( ex ) );
	  ex.addModule( new BasicIoExecutorModule( ex ) );
	  ex.addModule( new ClilocExecutorModule( ex ) );
	  ex.addModule( new MathExecutorModule( ex ) );
	  ex.addModule( new UtilExecutorModule( ex ) );
	  //ex.addModule( new FileExecutorModule( ex ) );	
	  ex.addModule( new ConfigFileExecutorModule( ex ) );
	  ex.addModule( new UBoatExecutorModule( ex ) );
	  ex.addModule( new DataFileExecutorModule( ex ) );
	  ex.addModule( new PolSystemExecutorModule( ex ) );
	  ex.addModule( new AttributeExecutorModule( ex ) );
	  ex.addModule( new VitalExecutorModule( ex ) );
	  ex.addModule( new StorageExecutorModule( ex ) );
	  ex.addModule( new GuildExecutorModule( ex ) );
	  ex.addModule( new UnicodeExecutorModule( ex ) );
	  ex.addModule( new PartyExecutorModule( ex ) );
	  ex.addModule( new SQLExecutorModule( ex ) );
	  ex.addModule( CreateFileAccessExecutorModule( ex ) );
	}

    bool run_script_to_completion_worker( UOExecutor& ex, Bscript::EScriptProgram* prog )
	{
	  add_common_exmods( ex );
      ex.addModule( new Module::UOExecutorModule( ex ) );

	  ex.setProgram( prog );

      ex.setDebugLevel( Bscript::Executor::NONE );

	  Clib::scripts_thread_script = ex.scriptname();

	  if ( config.report_rtc_scripts )
        INFO_PRINT << "Script " << ex.scriptname( ) << " running..";

	  while ( ex.runnable() )
	  {
        INFO_PRINT << ".";
		for ( int i = 0; ( i < 1000 ) && ex.runnable(); i++ )
		{
		  Clib::scripts_thread_scriptPC = ex.PC;
		  ex.execInstr();
		}
	  }
      INFO_PRINT << "\n";
	  return ( ex.error_ == false );
	}

    bool run_script_to_completion( const char *filename, Bscript::BObjectImp* parameter )
	{
	  passert_always( parameter );
      Bscript::BObject bobj( parameter );
      ref_ptr<Bscript::EScriptProgram> program = find_script( filename );
	  if ( program.get() == NULL )
	  {
        ERROR_PRINT << "Error reading script " << filename << "\n";
		return false;
	  }

	  UOExecutor ex;
	  if ( program->haveProgram )
	  {
		ex.pushArg( parameter );
	  }
	  return run_script_to_completion_worker( ex, program.get() );
	}

	bool run_script_to_completion( const char *filename )
	{
      ref_ptr<Bscript::EScriptProgram> program = find_script( filename );
	  if ( program.get() == NULL )
	  {
        ERROR_PRINT << "Error reading script " << filename << "\n";
		return false;
	  }

	  UOExecutor ex;

	  return run_script_to_completion_worker( ex, program.get() );
	}


    Bscript::BObjectImp* run_executor_to_completion( UOExecutor& ex, const ScriptDef& script )
	{
      ref_ptr<Bscript::EScriptProgram> program = find_script2( script );
	  if ( program.get() == NULL )
	  {
        ERROR_PRINT << "Error reading script " << script.name( ) << "\n";
        return new Bscript::BError( "Unable to read script" );
	  }

	  add_common_exmods( ex );
	  ex.addModule( new Module::UOExecutorModule( ex ) );

	  ex.setProgram( program.get() );

      ex.setDebugLevel( Bscript::Executor::NONE );

	  Clib::scripts_thread_script = ex.scriptname();

	  int i = 0;
	  bool reported = false;
	  while ( ex.runnable() )
	  {
		Clib::scripts_thread_scriptPC = ex.PC;
		ex.execInstr();
		if ( ++i == 1000 )
		{
		  if ( reported )
		  {
            INFO_PRINT << ".." << ex.PC;
		  }
		  else
		  {
			if ( config.report_rtc_scripts )
			{
              INFO_PRINT << "Script " << script.name( ) << " running.." << ex.PC;
			  reported = true;
			}
		  }
		  i = 0;
		}
	  }
	  if ( reported )
        INFO_PRINT << "\n";
	  if ( ex.error_ )
        return new Bscript::BError( "Script exited with an error condition" );

	  if ( ex.ValueStack.empty() )
        return new Bscript::BLong( 1 );
	  else
		return ex.ValueStack.top().get()->impptr()->copy();
	}

    Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script )
	{
	  UOExecutor ex;

	  return run_executor_to_completion( ex, script );
	}


    Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script,
                                          Bscript::BObjectImp* param1 )
	{
	  //??    BObject bobj1( param1 );

	  UOExecutor ex;

	  ex.pushArg( param1 );

	  return run_executor_to_completion( ex, script );
	}


    Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script,
                                                   Bscript::BObjectImp* param1,
                                                   Bscript::BObjectImp* param2 )
	{
	  //?? BObject bobj1( param1 ), bobj2( param2 );

	  UOExecutor ex;

	  ex.pushArg( param2 );
	  ex.pushArg( param1 );

	  return run_executor_to_completion( ex, script );
	}

    Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script,
                                                   Bscript::BObjectImp* param1,
                                                   Bscript::BObjectImp* param2,
                                                   Bscript::BObjectImp* param3 )
	{
	  //??    BObject bobj1( param1 ), bobj2( param2 ), bobj3( param3 );

	  UOExecutor ex;

	  ex.pushArg( param3 );
	  ex.pushArg( param2 );
	  ex.pushArg( param1 );

	  return run_executor_to_completion( ex, script );
	}

    Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script,
                                                   Bscript::BObjectImp* param1,
                                                   Bscript::BObjectImp* param2,
                                                   Bscript::BObjectImp* param3,
                                                   Bscript::BObjectImp* param4 )
	{
	  //??BObject bobj1( param1 ), bobj2( param2 ), bobj3( param3 ), bobj4( param4 );

	  UOExecutor ex;

	  ex.pushArg( param4 );
	  ex.pushArg( param3 );
	  ex.pushArg( param2 );
	  ex.pushArg( param1 );

	  return run_executor_to_completion( ex, script );
	}

    Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script,
                                                   Bscript::BObjectImp* param1,
                                                   Bscript::BObjectImp* param2,
                                                   Bscript::BObjectImp* param3,
                                                   Bscript::BObjectImp* param4,
                                                   Bscript::BObjectImp* param5 )
	{
	  //?? BObject bobj1( param1 ), bobj2( param2 ), bobj3( param3 ),
	  //??		bobj4( param4 ), bobj5( param5 );

	  UOExecutor ex;

	  ex.pushArg( param5 );
	  ex.pushArg( param4 );
	  ex.pushArg( param3 );
	  ex.pushArg( param2 );
	  ex.pushArg( param1 );

	  return run_executor_to_completion( ex, script );
	}

    Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script,
                                                   Bscript::BObjectImp* param1,
                                                   Bscript::BObjectImp* param2,
                                                   Bscript::BObjectImp* param3,
                                                   Bscript::BObjectImp* param4,
                                                   Bscript::BObjectImp* param5,
                                                   Bscript::BObjectImp* param6 )
	{
	  //?? BObject bobj1( param1 ), bobj2( param2 ), bobj3( param3 ),
	  //??		bobj4( param4 ), bobj5( param5 );

	  UOExecutor ex;

	  ex.pushArg( param6 );
	  ex.pushArg( param5 );
	  ex.pushArg( param4 );
	  ex.pushArg( param3 );
	  ex.pushArg( param2 );
	  ex.pushArg( param1 );

	  return run_executor_to_completion( ex, script );
	}

    Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script,
                                                   Bscript::BObjectImp* param1,
                                                   Bscript::BObjectImp* param2,
                                                   Bscript::BObjectImp* param3,
                                                   Bscript::BObjectImp* param4,
                                                   Bscript::BObjectImp* param5,
                                                   Bscript::BObjectImp* param6,
                                                   Bscript::BObjectImp* param7 )
	{
	  UOExecutor ex;

	  ex.pushArg( param7 );
	  ex.pushArg( param6 );
	  ex.pushArg( param5 );
	  ex.pushArg( param4 );
	  ex.pushArg( param3 );
	  ex.pushArg( param2 );
	  ex.pushArg( param1 );

	  return run_executor_to_completion( ex, script );
	}

	bool call_script( const ScriptDef& script,
                      Bscript::BObjectImp* param0 )
	{
	  try
	  {
        Bscript::BObject ob( run_script_to_completion( script, param0 ) );
		return ob.isTrue();
	  }
	  catch ( std::exception& )//...
	  {
		return false;
	  }
	}
	bool call_script( const ScriptDef& script,
                      Bscript::BObjectImp* param0,
                      Bscript::BObjectImp* param1 )
	{
	  try
	  {
        Bscript::BObject ob( run_script_to_completion( script, param0, param1 ) );
		return ob.isTrue();
	  }
	  catch ( std::exception& )//...
	  {
		return false;
	  }
	}
	bool call_script( const ScriptDef& script,
                      Bscript::BObjectImp* param0,
                      Bscript::BObjectImp* param1,
                      Bscript::BObjectImp* param2 )
	{
	  try
	  {
        Bscript::BObject ob( run_script_to_completion( script, param0, param1, param2 ) );
		return ob.isTrue();
	  }
	  catch ( std::exception& )//...
	  {
		return false;
	  }
	}
	bool call_script( const ScriptDef& script,
                      Bscript::BObjectImp* param0,
                      Bscript::BObjectImp* param1,
                      Bscript::BObjectImp* param2,
                      Bscript::BObjectImp* param3 )
	{
	  try
	  {
        Bscript::BObject ob( run_script_to_completion( script, param0, param1, param2, param3 ) );
		return ob.isTrue();
	  }
	  catch ( std::exception& )//...
	  {
		return false;
	  }
	}
	bool call_script( const ScriptDef& script,
                      Bscript::BObjectImp* param0,
                      Bscript::BObjectImp* param1,
                      Bscript::BObjectImp* param2,
                      Bscript::BObjectImp* param3,
                      Bscript::BObjectImp* param4 )
	{
	  try
	  {
        Bscript::BObject ob( run_script_to_completion( script, param0, param1, param2, param3, param4 ) );
		return ob.isTrue();
	  }
	  catch ( std::exception& )//...
	  {
		return false;
	  }
	}
	bool call_script( const ScriptDef& script,
                      Bscript::BObjectImp* param0,
                      Bscript::BObjectImp* param1,
                      Bscript::BObjectImp* param2,
                      Bscript::BObjectImp* param3,
                      Bscript::BObjectImp* param4,
                      Bscript::BObjectImp* param5 )
	{
	  try
	  {
        Bscript::BObject ob( run_script_to_completion( script, param0, param1, param2, param3, param4, param5 ) );
		return ob.isTrue();
	  }
	  catch ( std::exception& )//...
	  {
		return false;
	  }
	}
	bool call_script( const ScriptDef& script,
                      Bscript::BObjectImp* param0,
                      Bscript::BObjectImp* param1,
                      Bscript::BObjectImp* param2,
                      Bscript::BObjectImp* param3,
                      Bscript::BObjectImp* param4,
                      Bscript::BObjectImp* param5,
                      Bscript::BObjectImp* param6 )
	{
	  try
	  {
        Bscript::BObject ob( run_script_to_completion( script, param0, param1, param2, param3, param4, param5, param6 ) );
		return ob.isTrue();
	  }
	  catch ( std::exception& )//...
	  {
		return false;
	  }
	}

	UOExecutor *create_script_executor()
	{
	  UOExecutor *ex = new UOExecutor();

	  add_common_exmods( *ex );
	  //ex->addModule( new UOExecutorModule( *ex ) );	
	  return ex;
	}

	UOExecutor *create_full_script_executor()
	{
	  std::unique_ptr<UOExecutor> ex( new UOExecutor );

	  add_common_exmods( *ex );
	  ex->addModule( new Module::UOExecutorModule( *ex ) );
	  return ex.release();
	}

	void schedule_executor( UOExecutor* ex )
	{
      ex->setDebugLevel( Bscript::Executor::NONE );
	  // ex->initExec();

	  if ( ex->runnable() )
	  {
		runlist.push_back( ex );
	  }
	  else
	  {
		delete ex;
		/*
		  deadlist.push_back( ex );
		  script_done( ex );
		  */
	  }
	}

	void deschedule_executor( UOExecutor* ex )
	{
	  for ( ExecList::iterator itr = runlist.begin(), itrend = runlist.end(); itr != itrend; ++itr )
	  {
		if ( *itr == ex )
		{
		  runlist.erase( itr );
		  break;
		}
	  }
	  for ( ExecList::iterator itr = ranlist.begin(), itrend = ranlist.end(); itr != itrend; ++itr )
	  {
		if ( *itr == ex )
		{
		  ranlist.erase( itr );
		  break;
		}
	  }
	  if ( ex->os_module->blocked_ )
	  {
		if ( ex->os_module->in_hold_list_ == Module::OSExecutorModule::TIMEOUT_LIST )
		{
		  holdlist.erase( ex->os_module->hold_itr_ );
          ex->os_module->in_hold_list_ = Module::OSExecutorModule::NO_LIST;
		}
        else if ( ex->os_module->in_hold_list_ == Module::OSExecutorModule::NOTIMEOUT_LIST )
		{
		  notimeoutholdlist.erase( ex );
          ex->os_module->in_hold_list_ = Module::OSExecutorModule::NO_LIST;
		}
	  }
      if ( ex->os_module->in_hold_list_ == Module::OSExecutorModule::DEBUGGER_LIST )
	  {
		debuggerholdlist.erase( ex );
        ex->os_module->in_hold_list_ = Module::OSExecutorModule::NO_LIST;
	  }
	}

	void list_script( UOExecutor* uoexec )
	{
      fmt::Writer tmp;
      tmp << uoexec->prog_->name;
      if ( !uoexec->Globals2.empty() )
        tmp << " Gl=" << uoexec->Globals2.size();
      if ( uoexec->Locals2 && !uoexec->Locals2->empty() )
        tmp << " Lc=" << uoexec->Locals2->size();
	  if ( !uoexec->ValueStack.empty() )
        tmp << " VS=" << uoexec->ValueStack.size( );
	  if ( !uoexec->upperLocals2.empty() )
        tmp << " UL=" << uoexec->upperLocals2.size( );
	  if ( !uoexec->ControlStack.empty() )
        tmp << " CS=" << uoexec->ControlStack.size( );
	  INFO_PRINT << tmp.str() << "\n";
	}

	void list_scripts( const char* desc, ExecList& ls )
	{
      INFO_PRINT << desc << " scripts:\n";
      for ( auto &exec : ls )
      {
        list_script( exec );
      }
	}

	void list_scripts()
	{
	  list_scripts( "running", runlist );
	  // list_scripts( "holding", holdlist );
	  list_scripts( "ran", ranlist );
	}

	void list_crit_script( UOExecutor* uoexec )
	{
	  if ( uoexec->os_module->critical )
		list_script( uoexec );
	}
	void list_crit_scripts( const char* desc, ExecList& ls )
	{
      INFO_PRINT << desc << " scripts:\n";
      for ( auto &exec : ls )
      {
        list_crit_script( exec );
      }
	}

	void list_crit_scripts()
	{
	  list_crit_scripts( "running", runlist );
	  //list_crit_scripts( "holding", holdlist );
	  list_crit_scripts( "ran", ranlist );
	}
  }
}