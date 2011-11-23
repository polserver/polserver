/*
History
=======

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include <time.h>

#include "../../bscript/berror.h"
#include "../../bscript/eprog.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"
#include "../../clib/endian.h"
#include "../../clib/logfile.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"
#include "../../clib/unicode.h"
#include "../../clib/sckutil.h"
#include "../../clib/socketsvc.h"
#include "../mobile/attribute.h"
#include "../mobile/charactr.h"
#include "../network/client.h"
#include "../network/packets.h"
#include "../network/clienttransmit.h"
#include "../exscrobj.h"
#include "../logfiles.h"
#include "../npc.h"
#include "../party.h"
#include "../polcfg.h"
#include "../polclock.h"
#include "../poldbg.h"
#include "../polsig.h"
#include "../profile.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../skills.h"
#include "../ufunc.h"
#include "../uoexec.h"
#include "../watch.h"
#include "attributemod.h"
#include "basiciomod.h"
#include "basicmod.h"
#include "boatmod.h"
#include "cfgmod.h"
#include "clmod.h"
#include "datastore.h"
#include "filemod.h"
#include "guildmod.h"
#include "mathmod.h"
#include "npcmod.h"
#include "osmod.h"
#include "polsystemmod.h"
#include "storagemod.h"
#include "unimod.h"
#include "uomod.h"
#include "utilmod.h"
#include "vitalmod.h"
#include "../network/auxclient.h"

PidList pidlist;
unsigned int next_pid = 0;

unsigned int getnewpid( UOExecutor* uoexec )
{
	for(;;)
	{
		unsigned int newpid = ++next_pid;
		if (newpid != 0 && 
			pidlist.find(newpid) == pidlist.end())
		{
			pidlist[newpid] = uoexec;
			return newpid;
		}
	}
}
void freepid( unsigned int pid )
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

unsigned int OSExecutorModule::pid() const
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
	{ "start_skill_script",			&OSExecutorModule::start_skill_script },
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
	{ "OpenURL",					&OSExecutorModule::mf_OpenURL },
	{ "OpenConnection",				&OSExecutorModule::mf_OpenConnection }
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

std::string OSExecutorModule::functionName( unsigned idx )
{
	return function_table[idx].funcname;
}

BObjectImp* OSExecutorModule::create_debug_context()
{
	return ::create_debug_context();
}

BObjectImp* OSExecutorModule::getprocess()
{
	int pid;
	if (getParam( 0, pid ))
	{
		UOExecutor* uoexec;
		if (find_uoexec( static_cast<unsigned int>(pid), &uoexec ))
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
	if (!events_.empty())
	{
		BObjectImp* imp = events_.front();
		events_.pop();
		return imp;
	}
	else
	{
		int nsecs = (int) exec.paramAsLong(0);

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


BObjectImp* OSExecutorModule::start_skill_script()
{
	Character* chr;
	const Attribute* attr;

	if (getCharacterParam( 0, chr ) && getAttributeParam(exec, 1, attr))
	{
		if (!attr->disable_core_checks && !CanUseSkill(chr->client))
			return new BLong(0);
		else
		{
			const String* script_name;
			ScriptDef script;

			if ( exec.getStringParam(2, script_name) )
			{
				if (!script.config_nodie( script_name->value(), exec.prog()->pkg, "scripts/skills/" ))
				{
					return new BError( "Error in script name" );
				}
				if (!script.exists())
				{
					return new BError( "Script " + script.name() + " does not exist." );
				}
			}
			else
			{
				if (!attr->script_.empty())
					script = attr->script_;
				else
					return new BError( "No script defined for attribute " + attr->name + "." );
			}

			ref_ptr<EScriptProgram> prog = find_script2( script, true, /* complain if not found */ config.cache_interactive_scripts );

			if (prog.get() != NULL)
			{
				UObject* object;
				if (getUObjectParam( exec, 3, object ))
				{
					if ( chr->start_script( prog.get(), true, object->make_ref() ) )
					{ 
						if ( chr->hidden() && attr->unhides )
							chr->unhide();
						if ( attr->delay_seconds )
							chr->disable_skills_until = poltime() + attr->delay_seconds;
					}
				}
				else
				{
					if ( chr->start_script( prog.get(), true ) )
					{ 
						if ( chr->hidden() && attr->unhides )
							chr->unhide();
						if ( attr->delay_seconds )
							chr->disable_skills_until = poltime() + attr->delay_seconds;
					}
				}
			}
			else
			{
				string msg = "Unable to start skill script:";
				msg += script.c_str();
				send_sysmessage(chr->client, msg.c_str());

				return new BLong(0);
			}
			return new BLong(1);
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* OSExecutorModule::set_critical()
{
	int crit;
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
	int dbg;
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
	int newpri;
	if (getParam( 0, newpri, 1, 255 ))
	{
		int oldpri = priority;
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
const int SCRIPTOPT_AUXSVC_ASSUME_STRING = 5;

BObjectImp* OSExecutorModule::mf_set_script_option()
{
	int optnum;
	int optval;
	if (getParam( 0, optnum ) && getParam( 1, optval ))
	{
        int oldval;
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
		case SCRIPTOPT_AUXSVC_ASSUME_STRING:
			{
				UOExecutor& uoexec = static_cast<UOExecutor&>(exec);
				oldval = uoexec.auxsvc_assume_string?1:0;
				uoexec.auxsvc_assume_string = optval?true:false;
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
			PktOut_A5* msg = REQUESTPACKET(PktOut_A5,PKTOUT_A5_ID);
			unsigned urllen;
			const char *url = str->data();

			urllen = strlen(url);
			if (urllen > URL_MAX_LEN)
				urllen = URL_MAX_LEN;

			msg->WriteFlipped(static_cast<u16>(urllen+4));
			msg->Write(url,static_cast<u16>(urllen+1));
			ADDTOSENDQUEUE(chr->client, &msg->buffer, msg->offset );
			READDPACKET(msg);
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


BObjectImp* OSExecutorModule::mf_OpenConnection() {
	const String* host;
	const String* scriptname_str;
	unsigned short port;

	UOExecutorModule* this_uoemod = static_cast<UOExecutorModule*>(exec.findModule( "uo" ));
	UOExecutor* this_uoexec = static_cast<UOExecutor*>(&this_uoemod->exec);

	if(this_uoexec->pChild == NULL)
	{
	if (  (host = getStringParam( 0 )) != NULL && getParam( 1, port ) && (scriptname_str = getStringParam( 2 )) != NULL)
		{
			// FIXME needs to inherit available modules?
			ScriptDef sd;// = new ScriptDef();
			cout<<"Starting connection script "<<scriptname_str->value()<<endl;
			if (!sd.config_nodie( scriptname_str->value(), exec.prog()->pkg, "scripts/" ))
			{
				return new BError( "Error in script name" );
			}
			if (!sd.exists())
			{
				return new BError( "Script " + sd.name() + " does not exist." );
			}

			//Socket* s = new Socket();
			//bool success_open = s->open(host->value().c_str(),30);
			Socket s;
			bool success_open = s.open(host->value().c_str(),port);

			if (!success_open) {
				//delete s;
				return new BError("Error connecting to client");
			}
			SocketClientThread* clientthread = new AuxClientThread( sd, s );
            clientthread->start();

			return new BLong( 1 );
		}
		else
		{
			return new BError( "Invalid parameter type" );
		}
	}

	return new BError( "Invalid parameter type" );
}

