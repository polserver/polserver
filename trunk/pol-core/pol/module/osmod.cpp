/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"

#include <time.h>

#include "basiciomod.h"
#include "basicmod.h"
#include "../../bscript/berror.h"
#include "../../bscript/eprog.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"

#include "../../clib/logfile.h"
#include "../../clib/endian.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"
#include "../../clib/unicode.h"

#include "attributemod.h"
#include "boatmod.h"
#include "cfgmod.h"
#include "clmod.h"
#include "../mobile/charactr.h"
#include "datastore.h"
#include "../exscrobj.h"
#include "filemod.h"
#include "guildmod.h"
#include "../logfiles.h"
#include "mathmod.h"
#include "../npc.h"
#include "npcmod.h"
#include "osmod.h"
#include "../polcfg.h"
#include "../polclock.h"
#include "../poldbg.h"
#include "../polsig.h"
#include "polsystemmod.h"
#include "../profile.h"
#include "../scrdef.h"
#include "../scrstore.h"
#include "../storagemod.h"
#include "../uniemod.h"
#include "../uoemod.h"
#include "../uoexec.h"
#include "utilmod.h"
#include "vitalmod.h"
#include "../watch.h"

#include "../mobile/charactr.h"
#include "../client.h"

#include "../scrsched.h"

#include "../party.h"

PidList pidlist;
unsigned long next_pid = 0;

unsigned long getnewpid( UOExecutor* uoexec )
{
	for(;;)
	{
		unsigned long newpid = ++next_pid;
		if (newpid != 0 && 
			pidlist.find(newpid) == pidlist.end())
		{
			pidlist[newpid] = uoexec;
			return newpid;
		}
	}
}
void freepid( unsigned long pid )
{
	pidlist.erase( pid );
}

OSExecutorModule::OSExecutorModule( Executor& exec ) :
ExecutorModule( "OS", exec ),
critical(false),
priority(1),
warn_on_runaway(true),
blocked_(false),
sleep_until_clock_(0),
in_hold_list_(NO_LIST),
hold_itr_(),
pid_(getnewpid( static_cast<UOExecutor*>(&exec) )),
max_eventqueue_size(MAX_EVENTQUEUE_SIZE),
events_()
{
}

OSExecutorModule::~OSExecutorModule()
{
	freepid( pid_ );
	pid_ = 0;
	while (!events_.empty())
	{
		BObject ob( events_.front() );
		events_.pop();
	}
}

unsigned long OSExecutorModule::pid() const
{
	return pid_;
}

bool OSExecutorModule::blocked() const
{
	return blocked_;
}

OSFunctionDef OSExecutorModule::function_table[] =
{
	{ "create_debug_context",       &OSExecutorModule::create_debug_context },
	{ "getprocess",                 &OSExecutorModule::getprocess },
	{ "get_process",                &OSExecutorModule::getprocess },
	{ "getpid",                     &OSExecutorModule::getpid },
	{ "sleep",                      &OSExecutorModule::sleep },
	{ "sleepms",                    &OSExecutorModule::sleepms },
	{ "wait_for_event",             &OSExecutorModule::wait_for_event },
	{ "events_waiting",             &OSExecutorModule::events_waiting },
	{ "start_script",               &OSExecutorModule::start_script },
	{ "set_critical",               &OSExecutorModule::set_critical },
	{ "is_critical",				&OSExecutorModule::is_critical },
	{ "run_script_to_completion",   &OSExecutorModule::run_script_to_completion },
	{ "run_script",					&OSExecutorModule::run_script },
	{ "set_debug",                  &OSExecutorModule::mf_set_debug },
	{ "syslog",                     &OSExecutorModule::mf_Log },
	{ "system_rpm",                 &OSExecutorModule::mf_system_rpm },
	{ "set_priority",               &OSExecutorModule::mf_set_priority },
	{ "unload_scripts",             &OSExecutorModule::mf_unload_scripts },
	{ "set_script_option",          &OSExecutorModule::mf_set_script_option },
	{ "clear_event_queue",			&OSExecutorModule::mf_clear_event_queue },
	{ "set_event_queue_size",		&OSExecutorModule::mf_set_event_queue_size },
	{ "OpenURL",					&OSExecutorModule::mf_OpenURL }
};

int OSExecutorModule::functionIndex( const char *name )
{
	for( unsigned idx = 0; idx < arsize(function_table); idx++ )
	{
		if (stricmp( name, function_table[idx].funcname ) == 0)
			return idx;
	}
	return -1;
}

BObjectImp* OSExecutorModule::execFunc( unsigned funcidx )
{
	return callMemberFunction(*this, function_table[funcidx].fptr)();
};

BObjectImp* OSExecutorModule::create_debug_context()
{
	return ::create_debug_context();
}

BObjectImp* OSExecutorModule::getprocess()
{
	long pid;
	if (getParam( 0, pid ))
	{
		UOExecutor* uoexec;
		if (find_uoexec( static_cast<unsigned long>(pid), &uoexec ))
			return new ScriptExObjImp( uoexec );
		else
			return new BError( "Process not found" );

	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* OSExecutorModule::getpid()
{
	return new BLong( pid_ );
}
/*	Ok, this is going to be fun.  In the case where we block,
the caller is going to take our return value and push
it on the value stack.  

What we'll do is push the value that should be returned
if a timeout occurs.  THis way, for timeouts, all we have
to do is move the script back into the run queue.

When we actually complete something, we'll have to
pop the value off the stack, and replace it with
the real result.

Whew!
*/
BObjectImp* OSExecutorModule::sleep()
{
	int nsecs;

	nsecs = (int) exec.paramAsLong(0);

	SleepFor( nsecs );

	return new BLong( 0 );
}

BObjectImp* OSExecutorModule::sleepms()
{
	int msecs;

	msecs = (int) exec.paramAsLong(0);

	SleepForMs( msecs );

	return new BLong( 0 );
}

BObjectImp* OSExecutorModule::wait_for_event()
{
	int nsecs;
	if (!events_.empty())
	{
		BObjectImp* imp = events_.front();
		events_.pop();
		return imp;
	}
	else
	{
		nsecs = (int) exec.paramAsLong(0);

		if (nsecs)
		{
			wait_type = WAIT_EVENT;
			blocked_ = true;
			sleep_until_clock_ = polclock() + nsecs * POLCLOCKS_PER_SEC;
		}
		return new BLong( 0 );
	}
}

BObjectImp* OSExecutorModule::events_waiting()
{
	return new BLong( events_.size() );
}

BObjectImp* OSExecutorModule::start_script()
{
	const String* scriptname_str;
	if (exec.getStringParam( 0, scriptname_str ))
	{
		BObjectImp* imp = exec.getParamImp( 1 );

		// FIXME needs to inherit available modules?
		ScriptDef sd;
		if (!sd.config_nodie( scriptname_str->value(), exec.prog()->pkg, "scripts/" ))
		{
			return new BError( "Error in script name" );
		}
		if (!sd.exists())
		{
			return new BError( "Script " + sd.name() + " does not exist." );
		}
		UOExecutorModule* new_uoemod = ::start_script( sd, imp->copy() );
		if (new_uoemod == NULL)
		{
			return new BError( "Unable to start script" );
		}
		UOExecutorModule* this_uoemod = static_cast<UOExecutorModule*>(exec.findModule( "uo" ));
		if (new_uoemod && this_uoemod)
		{
			new_uoemod->controller_ = this_uoemod->controller_;
		}
		UOExecutor* uoexec = static_cast<UOExecutor*>(&new_uoemod->exec);
		return new ScriptExObjImp( uoexec );
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* OSExecutorModule::set_critical()
{
	long crit;
	if (exec.getParam( 0, crit ))
	{
		critical = (crit != 0);
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* OSExecutorModule::is_critical()
{
	if(critical)
		return new BLong(1);
	else
		return new BLong(0);
}

BObjectImp* OSExecutorModule::run_script_to_completion()
{
	const char* scriptname = exec.paramAsString(0);
	if (scriptname == NULL)
		return new BLong(0);

	// FIXME needs to inherit available modules?
	ScriptDef script;

	if (!script.config_nodie( scriptname, exec.prog()->pkg, "scripts/" ))
		return new BError( "Script descriptor error" );

	if (!script.exists())
		return new BError( "Script does not exist" );

	return ::run_script_to_completion( script, getParamImp(1) ); 
}

BObjectImp* OSExecutorModule::run_script()
{
	UOExecutorModule* this_uoemod = static_cast<UOExecutorModule*>(exec.findModule( "uo" ));
	UOExecutor* this_uoexec = static_cast<UOExecutor*>(&this_uoemod->exec);

	if(this_uoexec->pChild == NULL)
	{
		const String* scriptname_str;
		if (exec.getStringParam( 0, scriptname_str ))
		{
			BObjectImp* imp = exec.getParamImp( 1 );

			// FIXME needs to inherit available modules?
			ScriptDef sd;
			if (!sd.config_nodie( scriptname_str->value(), exec.prog()->pkg, "scripts/" ))
			{
				return new BError( "Error in script name" );
			}
			if (!sd.exists())
			{
				return new BError( "Script " + sd.name() + " does not exist." );
			}
			UOExecutorModule* new_uoemod = ::start_script( sd, imp->copy() );
			if (new_uoemod == NULL)
			{
				return new BError( "Unable to run script" );
			}
			if (new_uoemod && this_uoemod)
			{
				new_uoemod->controller_ = this_uoemod->controller_;
			}
			UOExecutor* new_uoexec = static_cast<UOExecutor*>(&new_uoemod->exec);
			//			OSExecutorModule* osemod = uoexec->os_module;
			new_uoexec->pParent = this_uoexec;
			this_uoexec->pChild = new_uoexec;

			// we want to forcefully do this instruction over again:
			this_uoexec->PC--;																// run_script(
			this_uoexec->ValueStack.push(BObjectRef(new BObject(UninitObject::create())));	//   script_name,
			// No need to push on "param" since the new BLong(0) below will take care of it.//   param )

			// Put me on hold until my child is done.
			suspend();

			return new BLong( 0 );
		}
		else
		{
			return new BError( "Invalid parameter type" );
		}
	}

	// else I am running a child script, and its ended
	BObjectImp* ret;

	if (this_uoexec->pChild->ValueStack.empty())
		ret = new BLong( 1 );
	else
		ret = this_uoexec->pChild->ValueStack.top().get()->impptr()->copy();

	this_uoexec->pChild->pParent = NULL;
	this_uoexec->pChild = NULL;

	return ret;
}

BObjectImp* OSExecutorModule::mf_set_debug()
{
	long dbg;
	if (getParam( 0, dbg ))
	{
		if (dbg)
			exec.setDebugLevel( Executor::SOURCELINES );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* OSExecutorModule::mf_Log()
{
	BObjectImp* imp = exec.getParamImp( 0 );
	if (imp->isa( BObjectImp::OTString ))
	{
		String* str = static_cast<String*>(imp);
		Log( "[%s]: %s\n", exec.scriptname().c_str(), str->data() );
		cout << "syslog [" << exec.scriptname() << "]: " << str->data() << endl;
		return new BLong(1);
	}
	else
	{
		std::string strval = imp->getStringRep();
		Log( "[%s]: %s\n", exec.scriptname().c_str(), strval.c_str() );
		cout << "syslog: [" << exec.scriptname() << "]: " << strval << endl;
		return new BLong(1);
	}
}

BObjectImp* OSExecutorModule::mf_system_rpm()
{
	return new BLong( last_rpm );
}

BObjectImp* OSExecutorModule::mf_set_priority()
{
	long newpri;
	if (getParam( 0, newpri, 1, 255 ))
	{
		long oldpri = priority;
		priority = static_cast<unsigned char>(newpri);
		return new BLong( oldpri );
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}




BObjectImp* OSExecutorModule::mf_unload_scripts()
{
	const String* str;
	if (getStringParam( 0, str ))
	{
		int n;
		if (str->length() == 0)
			n = unload_all_scripts();
		else
			n = unload_script( str->data() );
		return new BLong(n);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

const int SCRIPTOPT_NO_INTERRUPT    = 1;
const int SCRIPTOPT_DEBUG           = 2;
const int SCRIPTOPT_NO_RUNAWAY      = 3;
const int SCRIPTOPT_CAN_ACCESS_OFFLINE_MOBILES = 4;

BObjectImp* OSExecutorModule::mf_set_script_option()
{
	long optnum;
	long optval;
	long oldval;
	if (getParam( 0, optnum ) && getParam( 1, optval ))
	{
		switch( optnum )
		{
		case SCRIPTOPT_NO_INTERRUPT:
			oldval = critical?1:0;
			critical = optval?true:false; 
			break;
		case SCRIPTOPT_DEBUG:
			oldval = exec.getDebugLevel();
			if (optval)
				exec.setDebugLevel( Executor::SOURCELINES );
			else
				exec.setDebugLevel( Executor::NONE );
			break;
		case SCRIPTOPT_NO_RUNAWAY:
			oldval = warn_on_runaway?1:0;
			warn_on_runaway = !optval;
			break;
		case SCRIPTOPT_CAN_ACCESS_OFFLINE_MOBILES:
			{
				UOExecutor& uoexec = static_cast<UOExecutor&>(exec);
				oldval = uoexec.can_access_offline_mobiles?1:0;
				uoexec.can_access_offline_mobiles = optval?true:false;
			}
			break;
		default:
			return new BError( "Unknown Script Option" );
		}
		return new BLong(oldval);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* OSExecutorModule::mf_clear_event_queue() //DAVE
{ 
	return( clear_event_queue() );
}

int max_eventqueue_size = 0;

BObjectImp* OSExecutorModule::clear_event_queue() //DAVE
{
	while (!events_.empty())
	{
		BObject ob( events_.front() );
		events_.pop();
	}
	return new BLong(1);
}

BObjectImp* OSExecutorModule::mf_set_event_queue_size() //DAVE 11/24
{
	unsigned short param;
	if (getParam( 0, param ))
	{
		unsigned short oldsize = max_eventqueue_size;
		max_eventqueue_size = param;
		return new BLong(oldsize);
	}
	else
		return new BError( "Invalid parameter type" );
}

BObjectImp* OSExecutorModule::mf_OpenURL()
{
	Character* chr;
	const String* str;
	if (getCharacterParam( 0, chr ) &&
		( (str = getStringParam( 1 )) != NULL ) )
	{
		if (chr->has_active_client())
		{
			PKTOUT_A5 msg;
			unsigned urllen;
			const char *url = str->data();

			urllen = strlen(url);
			if (urllen > sizeof msg.address)
				urllen = sizeof msg.address;

			msg.msgtype = PKTOUT_A5_ID;
			msg.msglen = ctBEu16( (urllen+3) );
			memcpy( msg.address, url, urllen );
			msg.null_term = 0;
			chr->client->transmit( &msg, (urllen+3) );
			return new BLong(1);
		}
		else
		{
			return new BError( "No client attached" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}