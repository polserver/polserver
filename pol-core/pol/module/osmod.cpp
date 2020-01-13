/** @file
 *
 * @par History
 */

#include "osmod.h"
#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/bstruct.h"
#include "../../bscript/impstr.h"
#include "../../clib/esignal.h"
#include "../../clib/logfacility.h"
#include "../../clib/make_unique.hpp"
#include "../../clib/network/sckutil.h"
#include "../../clib/rawtypes.h"
#include "../../clib/refptr.h"
#include "../../clib/threadhelp.h"
#include "../../clib/weakptr.h"
#include "../../plib/systemstate.h"
#include "../exscrobj.h"
#include "../globals/script_internals.h"
#include "../globals/state.h"
#include "../item/item.h"
#include "../mobile/attribute.h"
#include "../mobile/charactr.h"
#include "../mobile/npc.h"
#include "../network/auxclient.h"
#include "../network/packethelper.h"
#include "../network/packets.h"
#include "../network/pktdef.h"
#include "../polcfg.h"
#include "../poldbg.h"
#include "../polsem.h"
#include "../profile.h"
#include "../schedule.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../skills.h"
#include "../ufunc.h"
#include "../uoexec.h"
#include "npcmod.h"
#include "uomod.h"

#include <module_defs/os.h>

#ifdef _WIN32
#pragma comment( lib, "crypt32.lib" )
#endif
#include <ctime>
#include <curl/curl.h>
#include <memory>
#include <string.h>
#include <string>
#include <unordered_map>

#include <boost/process.hpp>
namespace bp = boost::process;

namespace Pol
{
namespace Module
{
using namespace Bscript;

unsigned int getnewpid( Core::UOExecutor* uoexec )
{
  return Core::scriptScheduler.get_new_pid( uoexec );
}
void freepid( unsigned int pid )
{
  Core::scriptScheduler.free_pid( pid );
}

OSExecutorModule::OSExecutorModule( Bscript::Executor& exec )
    : TmplExecutorModule<OSExecutorModule>( exec ),
      critical_( false ),
      priority_( 1 ),
      warn_on_runaway_( true ),
      blocked_( false ),
      sleep_until_clock_( 0 ),
      hold_itr_(),
      in_hold_list_( Core::HoldListType::NO_LIST ),
      wait_type( Core::WAIT_TYPE::WAIT_UNKNOWN ),
      pid_( getnewpid( static_cast<Core::UOExecutor*>( &exec ) ) ),
      max_eventqueue_size( Core::MAX_EVENTQUEUE_SIZE ),
      events_()
{
}

OSExecutorModule::~OSExecutorModule()
{
  freepid( pid_ );
  pid_ = 0;
  while ( !events_.empty() )
  {
    Bscript::BObject ob( events_.front() );
    events_.pop_front();
  }
  if ( childProcess_ )
  {
    childProcess_.reset();
  }
}

unsigned int OSExecutorModule::pid() const
{
  return pid_;
}

BObjectImp* OSExecutorModule::mf_Create_Debug_Context()
{
  return Core::create_debug_context();
}

BObjectImp* OSExecutorModule::mf_Debugger()
{
  Core::UOExecutor* uoexec;
  if ( find_uoexec( pid_, &uoexec ) )
  {
    uoexec->attach_debugger();
    return new BLong( 1 );
  }
  else
    return new BError( "Could not find UOExecutor for current process." );
}

BObjectImp* OSExecutorModule::mf_GetProcess()
{
  int pid;  // note that while pid's are unsigned, valid values are forced to fit within a signed
            // range
  if ( !getParam( 0, pid ) )
    return new BError( "Invalid parameter type" );

  if ( pid == -1 )
  {
    pid = pid_;  // get executor's own pid
  }

  Core::UOExecutor* uoexec;
  if ( find_uoexec( pid, &uoexec ) )
    return new Core::ScriptExObjImp( uoexec );
  else
    return new BError( "Process not found" );
}

BObjectImp* OSExecutorModule::mf_GetPid()
{
  return new BLong( pid_ );
}
/*  Ok, this is going to be fun.  In the case where we block,
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
BObjectImp* OSExecutorModule::mf_Sleep()
{
  int nsecs;

  nsecs = (int)exec.paramAsLong( 0 );

  SleepFor( nsecs );

  return new BLong( 0 );
}

BObjectImp* OSExecutorModule::mf_Sleepms()
{
  int msecs;

  msecs = (int)exec.paramAsLong( 0 );

  SleepForMs( msecs );

  return new BLong( 0 );
}

BObjectImp* OSExecutorModule::mf_Wait_For_Event()
{
  if ( !events_.empty() )
  {
    BObjectImp* imp = events_.front();
    events_.pop_front();
    return imp;
  }
  else
  {
    int nsecs = (int)exec.paramAsLong( 0 );

    if ( nsecs )
    {
      wait_type = Core::WAIT_TYPE::WAIT_EVENT;
      blocked_ = true;
      sleep_until_clock_ = Core::polclock() + nsecs * Core::POLCLOCKS_PER_SEC;
    }
    return new BLong( 0 );
  }
}

BObjectImp* OSExecutorModule::mf_Events_Waiting()
{
  return new BLong( static_cast<int>( events_.size() ) );
}

BObjectImp* OSExecutorModule::mf_Start_Script()
{
  const String* scriptname_str;
  if ( exec.getStringParam( 0, scriptname_str ) )
  {
    BObjectImp* imp = exec.getParamImp( 1 );

    // FIXME needs to inherit available modules?
    Core::ScriptDef sd;
    if ( !sd.config_nodie( scriptname_str->value(), exec.prog()->pkg, "scripts/" ) )
    {
      return new BError( "Error in script name" );
    }
    if ( !sd.exists() )
    {
      return new BError( "Script " + sd.name() + " does not exist." );
    }
    UOExecutorModule* new_uoemod = Core::start_script( sd, imp->copy() );
    if ( new_uoemod == nullptr )
    {
      return new BError( "Unable to start script" );
    }
    UOExecutorModule* this_uoemod = static_cast<UOExecutorModule*>( exec.findModule( "uo" ) );
    if ( new_uoemod != nullptr && this_uoemod != nullptr )
    {
      new_uoemod->controller_ = this_uoemod->controller_;
    }
    Core::UOExecutor* uoexec = static_cast<Core::UOExecutor*>( &new_uoemod->exec );
    return new Core::ScriptExObjImp( uoexec );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}


BObjectImp* OSExecutorModule::mf_Start_Skill_Script()
{
  Mobile::Character* chr;
  const Mobile::Attribute* attr;

  if ( getCharacterParam( 0, chr ) && Core::getAttributeParam( exec, 1, attr ) )
  {
    if ( !attr->disable_core_checks && !Core::CanUseSkill( chr->client ) )
      return new BLong( 0 );
    else
    {
      const String* script_name;
      Core::ScriptDef script;

      if ( exec.getStringParam( 2, script_name ) && ( script_name->length() > 0 ) )
      {
        if ( !script.config_nodie( script_name->value(), exec.prog()->pkg, "scripts/skills/" ) )
        {
          return new BError( "Error in script name" );
        }
        if ( !script.exists() )
        {
          return new BError( "Script " + script.name() + " does not exist." );
        }
      }
      else
      {
        if ( !attr->script_.empty() )
          script = attr->script_;
        else
          return new BError( "No script defined for attribute " + attr->name + "." );
      }

      ref_ptr<EScriptProgram> prog = find_script2(
          script, true,
          /* complain if not found */ Plib::systemstate.config.cache_interactive_scripts );

      if ( prog.get() != nullptr )
      {
        BObjectImp* imp = exec.getParamImp( 3 );
        if ( imp )
        {
          if ( chr->start_script( prog.get(), true, imp->copy() ) )
          {
            if ( chr->hidden() && attr->unhides )
              chr->unhide();
            if ( attr->delay_seconds )
              chr->disable_skills_until( Core::poltime() + attr->delay_seconds );
          }
        }
        else
        {
          if ( chr->start_script( prog.get(), true ) )
          {
            if ( chr->hidden() && attr->unhides )
              chr->unhide();
            if ( attr->delay_seconds )
              chr->disable_skills_until( Core::poltime() + attr->delay_seconds );
          }
        }
      }
      else
      {
        std::string msg = "Unable to start skill script:";
        msg += script.c_str();
        Core::send_sysmessage( chr->client, msg.c_str() );

        return new BLong( 0 );
      }
      return new BLong( 1 );
    }
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* OSExecutorModule::mf_Set_Critical()
{
  int crit;
  if ( exec.getParam( 0, crit ) )
  {
    critical_ = ( crit != 0 );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* OSExecutorModule::mf_Is_Critical()
{
  if ( critical_ )
    return new BLong( 1 );
  else
    return new BLong( 0 );
}

BObjectImp* OSExecutorModule::mf_Run_Script_To_Completion()
{
  const char* scriptname = exec.paramAsString( 0 );
  if ( scriptname == nullptr )
    return new BLong( 0 );

  // FIXME needs to inherit available modules?
  Core::ScriptDef script;

  if ( !script.config_nodie( scriptname, exec.prog()->pkg, "scripts/" ) )
    return new BError( "Script descriptor error" );

  if ( !script.exists() )
    return new BError( "Script does not exist" );

  return Core::run_script_to_completion( script, getParamImp( 1 ) );
}

BObjectImp* OSExecutorModule::mf_Run_Script()
{
  UOExecutorModule* this_uoemod = static_cast<UOExecutorModule*>( exec.findModule( "uo" ) );
  Core::UOExecutor* this_uoexec = static_cast<Core::UOExecutor*>( &this_uoemod->exec );

  if ( this_uoexec->pChild == nullptr )
  {
    const String* scriptname_str;
    if ( exec.getStringParam( 0, scriptname_str ) )
    {
      BObjectImp* imp = exec.getParamImp( 1 );

      // FIXME needs to inherit available modules?
      Core::ScriptDef sd;
      if ( !sd.config_nodie( scriptname_str->value(), exec.prog()->pkg, "scripts/" ) )
      {
        return new BError( "Error in script name" );
      }
      if ( !sd.exists() )
      {
        return new BError( "Script " + sd.name() + " does not exist." );
      }
      UOExecutorModule* new_uoemod = Core::start_script( sd, imp->copy() );
      if ( new_uoemod == nullptr )
      {
        return new BError( "Unable to run script" );
      }
      if ( new_uoemod )
      {
        new_uoemod->controller_ = this_uoemod->controller_;
      }
      Core::UOExecutor* new_uoexec = static_cast<Core::UOExecutor*>( &new_uoemod->exec );
      //      OSExecutorModule* osemod = uoexec->os_module;
      new_uoexec->pParent = this_uoexec;
      this_uoexec->pChild = new_uoexec;

      // we want to forcefully do this instruction over again:
      this_uoexec->PC--;  // run_script(
      this_uoexec->ValueStack.push_back(
          BObjectRef( new BObject( UninitObject::create() ) ) );  //   script_name,
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

  if ( this_uoexec->pChild->ValueStack.empty() )
    ret = new BLong( 1 );
  else
    ret = this_uoexec->pChild->ValueStack.back().get()->impptr()->copy();

  this_uoexec->pChild->pParent = nullptr;
  this_uoexec->pChild = nullptr;

  return ret;
}

BObjectImp* OSExecutorModule::mf_Set_Debug()
{
  int dbg;
  if ( getParam( 0, dbg ) )
  {
    if ( dbg )
      exec.setDebugLevel( Executor::SOURCELINES );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* OSExecutorModule::mf_SysLog()
{
  BObjectImp* imp = exec.getParamImp( 0 );
  if ( imp->isa( BObjectImp::OTString ) )
  {
    String* str = static_cast<String*>( imp );
    POLLOG << "[" << exec.scriptname() << "]: " << str->data() << "\n";
    INFO_PRINT << "syslog [" << exec.scriptname() << "]: " << str->data() << "\n";
    return new BLong( 1 );
  }
  else
  {
    std::string strval = imp->getStringRep();
    POLLOG << "[" << exec.scriptname() << "]: " << strval << "\n";
    INFO_PRINT << "syslog [" << exec.scriptname() << "]: " << strval << "\n";
    return new BLong( 1 );
  }
}

BObjectImp* OSExecutorModule::mf_Set_Priority()
{
  int newpri;
  if ( getParam( 0, newpri, 1, 255 ) )
  {
    int oldpri = priority_;
    priority_ = static_cast<unsigned char>( newpri );
    return new BLong( oldpri );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}


BObjectImp* OSExecutorModule::mf_Unload_Scripts()
{
  const String* str;
  if ( getStringParam( 0, str ) )
  {
    int n;
    if ( str->length() == 0 )
      n = Core::unload_all_scripts();
    else
      n = Core::unload_script( str->data() );
    return new BLong( n );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* OSExecutorModule::clear_event_queue()  // DAVE
{
  while ( !events_.empty() )
  {
    BObject ob( events_.front() );
    events_.pop_front();
  }
  return new BLong( 1 );
}

BObjectImp* OSExecutorModule::mf_Set_Event_Queue_Size()  // DAVE 11/24
{
  unsigned short param;
  if ( getParam( 0, param ) )
  {
    unsigned short oldsize = max_eventqueue_size;
    max_eventqueue_size = param;
    return new BLong( oldsize );
  }
  else
    return new BError( "Invalid parameter type" );
}

BObjectImp* OSExecutorModule::mf_OpenURL()
{
  Mobile::Character* chr;
  const String* str;
  if ( getCharacterParam( 0, chr ) && ( ( str = getStringParam( 1 ) ) != nullptr ) )
  {
    if ( chr->has_active_client() )
    {
      Network::PktHelper::PacketOut<Network::PktOut_A5> msg;
      unsigned urllen;
      const char* url = str->data();

      urllen = static_cast<unsigned int>( strlen( url ) );
      if ( urllen > URL_MAX_LEN )
        urllen = URL_MAX_LEN;

      msg->WriteFlipped<u16>( urllen + 4u );
      msg->Write( url, static_cast<u16>( urllen + 1 ) );
      msg.Send( chr->client );
      return new BLong( 1 );
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


BObjectImp* OSExecutorModule::mf_OpenConnection()
{
  UOExecutorModule* this_uoemod = static_cast<UOExecutorModule*>( exec.findModule( "uo" ) );
  Core::UOExecutor* this_uoexec = static_cast<Core::UOExecutor*>( &this_uoemod->exec );

  if ( this_uoexec->pChild == nullptr )
  {
    const String* host;
    const String* scriptname_str;
    BObjectImp* scriptparam;
    unsigned short port;
    int assume_string_int;
    if ( getStringParam( 0, host ) && getParam( 1, port ) && getStringParam( 2, scriptname_str ) &&
         getParamImp( 3, scriptparam ) && getParam( 4, assume_string_int ) )
    {
      // FIXME needs to inherit available modules?
      Core::ScriptDef sd;
      if ( !sd.config_nodie( scriptname_str->value(), exec.prog()->pkg, "scripts/" ) )
      {
        return new BError( "Error in script name" );
      }
      if ( !sd.exists() )
      {
        return new BError( "Script " + sd.name() + " does not exist." );
      }
      if ( !this_uoexec->suspend() )
      {
        DEBUGLOG << "Script Error in '" << this_uoexec->scriptname() << "' PC=" << this_uoexec->PC
                 << ": \n"
                 << "\tThe execution of this script can't be blocked!\n";
        return new Bscript::BError( "Script can't be blocked" );
      }

      weak_ptr<Core::UOExecutor> uoexec_w = this_uoexec->weakptr;
      std::string hostname( host->value() );
      bool assume_string = assume_string_int != 0;
      Core::networkManager.auxthreadpool->push(
          [uoexec_w, sd, hostname, port, scriptparam, assume_string]() {
            Clib::Socket s;
            bool success_open = s.open( hostname.c_str(), port );
            {
              Core::PolLock lck;
              if ( !uoexec_w.exists() )
              {
                DEBUGLOG << "OpenConnection Script has been destroyed\n";
                s.close();
                return;
              }
              if ( !success_open )
              {
                uoexec_w.get_weakptr()->ValueStack.back().set(
                    new BObject( new BError( "Error connecting to client" ) ) );
              }
              else
              {
                uoexec_w.get_weakptr()->ValueStack.back().set( new BObject( new BLong( 1 ) ) );
              }
              uoexec_w.get_weakptr()->revive();
            }
            std::unique_ptr<Network::AuxClientThread> client( new Network::AuxClientThread(
                sd, std::move( s ), scriptparam->copy(), assume_string ) );
            client->run();
          } );

      return new BLong( 0 );
    }
    else
    {
      return new BError( "Invalid parameter type" );
    }
  }

  return new BError( "Invalid parameter type" );
}

size_t curlWriteCallback( void* contents, size_t size, size_t nmemb, void* userp )
{
  ( static_cast<std::string*>( userp ) )->append( static_cast<char*>( contents ), size * nmemb );
  return size * nmemb;
}

BObjectImp* OSExecutorModule::mf_HTTPRequest()
{
  UOExecutorModule* this_uoemod = static_cast<UOExecutorModule*>( exec.findModule( "uo" ) );
  Core::UOExecutor* this_uoexec = static_cast<Core::UOExecutor*>( &this_uoemod->exec );

  if ( this_uoexec->pChild == nullptr )
  {
    const String *url, *method;
    BObjectImp* options;
    if ( getStringParam( 0, url ) && getStringParam( 1, method ) && getParamImp( 2, options ) )
    {
      if ( !this_uoexec->suspend() )
      {
        DEBUGLOG << "Script Error in '" << this_uoexec->scriptname() << "' PC=" << this_uoexec->PC
                 << ": \n"
                 << "\tThe execution of this script can't be blocked!\n";
        return new Bscript::BError( "Script can't be blocked" );
      }

      weak_ptr<Core::UOExecutor> uoexec_w = this_uoexec->weakptr;

      std::shared_ptr<CURL> curl_sp( curl_easy_init(), curl_easy_cleanup );
      CURL* curl = curl_sp.get();
      if ( curl )
      {
        curl_easy_setopt( curl, CURLOPT_URL, url->data() );
        curl_easy_setopt( curl, CURLOPT_CUSTOMREQUEST, method->data() );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curlWriteCallback );
        curl_easy_setopt( curl, CURLOPT_ACCEPT_ENCODING,
                          "" );  // allow plaintext and compressed (with automatic deflate)

        struct curl_slist* chunk = nullptr;
        if ( options->isa( Bscript::BObjectImp::OTStruct ) )
        {
          Bscript::BStruct* opts = static_cast<Bscript::BStruct*>( options );
          const BObjectImp* data = opts->FindMember( "data" );
          if ( data != nullptr )
          {
            curl_easy_setopt( curl, CURLOPT_COPYPOSTFIELDS, data->getStringRep().c_str() );
          }

          const BObjectImp* headers_ = opts->FindMember( "headers" );

          if ( headers_ != nullptr && headers_->isa( BObjectImp::OTStruct ) )
          {
            const BStruct* headers = static_cast<const BStruct*>( headers_ );

            for ( const auto& content : headers->contents() )
            {
              BObjectImp* ref = content.second->impptr();
              std::string header = content.first + ": " + ref->getStringRep();
              chunk = curl_slist_append( chunk, header.c_str() );
            }
            curl_easy_setopt( curl, CURLOPT_HTTPHEADER, chunk );
          }
        }

        Core::networkManager.auxthreadpool->push( [uoexec_w, curl_sp, chunk]() {
          CURL* curl = curl_sp.get();
          CURLcode res;
          std::string readBuffer;
          curl_easy_setopt( curl, CURLOPT_WRITEDATA, &readBuffer );

          /* Perform the request, res will get the return code */
          res = curl_easy_perform( curl );
          if ( chunk != nullptr )
            curl_slist_free_all( chunk );
          {
            Core::PolLock lck;

            if ( !uoexec_w.exists() )
            {
              DEBUGLOG << "OpenConnection Script has been destroyed\n";
              return;
            }
            /* Check for errors */
            if ( res != CURLE_OK )
            {
              uoexec_w.get_weakptr()->ValueStack.back().set(
                  new BObject( new BError( curl_easy_strerror( res ) ) ) );
            }
            else
            {
              // TODO: no sanitize happens, optional function param iso/utf8 encoding, or parse the
              // header of the http answer?
              uoexec_w.get_weakptr()->ValueStack.back().set(
                  new BObject( new String( readBuffer ) ) );
            }

            uoexec_w.get_weakptr()->revive();
          }

          /* always cleanup */
          // curl_easy_cleanup() is performed when the shared pointer deallocates
        } );
      }
      else
      {
        return new BError( "curl_easy_init() failed" );
      }
    }
    else
    {
      return new BError( "Invalid parameter type" );
    }
  }

  return new BError( "Invalid parameter type" );
}

// signal_event() takes ownership of the pointer which is passed to it.
// Objects must not be touched or deleted after being sent here!
// TODO: Find a better way to enforce this in the codebase.
bool OSExecutorModule::signal_event( BObjectImp* imp )
{
  INC_PROFILEVAR( events );

  if ( blocked_ && ( wait_type == Core::WAIT_TYPE::WAIT_EVENT ) )  // already being waited for
  {
    /* Now, the tricky part.  The value to return on an error or
    completion condition has already been pushed onto the value
    stack - so, we need to replace it with the real result.
    */
    exec.ValueStack.back().set( new BObject( imp ) );
    /* This executor will get moved to the run queue at the
    next step_scripts(), where blocked is checked looking
    for timed out or completed operations. */

    revive();
  }
  else  // not being waited for, so queue for later.
  {
    if ( events_.size() < max_eventqueue_size )
    {
      events_.push_back( imp );
    }
    else
    {
      if ( Plib::systemstate.config.discard_old_events )
      {
        BObject ob( events_.front() );
        events_.pop_front();
        events_.push_back( imp );
      }
      else
      {
        BObject ob( imp );
        if ( Plib::systemstate.config.loglevel >= 11 )
        {
          INFO_PRINT << "Event queue for " << exec.scriptname() << " is full, discarding event.\n";
          ExecutorModule* em = exec.findModule( "npc" );
          if ( em )
          {
            NPCExecutorModule* npcemod = static_cast<NPCExecutorModule*>( em );
            INFO_PRINT << "NPC Serial: " << fmt::hexu( npcemod->npc.serial ) << " ("
                       << npcemod->npc.x << " " << npcemod->npc.y << " " << npcemod->npc.z << ")\n";
          }

          INFO_PRINT << "Event: " << ob->getStringRep() << "\n";
        }
        return false;  // Event-queue is full
      }
    }
  }

  return true;  // Event was successfully sent (perhaps by discarding old events)
}

void OSExecutorModule::SleepFor( int nsecs )
{
  if ( nsecs )
  {
    blocked_ = true;
    wait_type = Core::WAIT_TYPE::WAIT_SLEEP;
    sleep_until_clock_ = Core::polclock() + nsecs * Core::POLCLOCKS_PER_SEC;
  }
}

void OSExecutorModule::SleepForMs( int msecs )
{
  if ( msecs )
  {
    blocked_ = true;
    wait_type = Core::WAIT_TYPE::WAIT_SLEEP;
    sleep_until_clock_ = Core::polclock() + msecs * Core::POLCLOCKS_PER_SEC / 1000;
  }
}

void OSExecutorModule::suspend()
{
  blocked_ = true;
  wait_type = Core::WAIT_TYPE::WAIT_SLEEP;
  sleep_until_clock_ = 0;  // wait forever
}

void OSExecutorModule::revive()
{
  blocked_ = false;
  if ( in_hold_list_ == Core::HoldListType::TIMEOUT_LIST )
  {
    in_hold_list_ = Core::HoldListType::NO_LIST;
    Core::scriptScheduler.revive_timeout( static_cast<Core::UOExecutor*>( &exec ), hold_itr_ );
  }
  else if ( in_hold_list_ == Core::HoldListType::NOTIMEOUT_LIST )
  {
    in_hold_list_ = Core::HoldListType::NO_LIST;
    Core::scriptScheduler.revive_notimeout( static_cast<Core::UOExecutor*>( &exec ) );
  }
  else if ( in_hold_list_ == Core::HoldListType::DEBUGGER_LIST )
  {
    // stays right where it is.
  }
}
bool OSExecutorModule::in_debugger_holdlist() const
{
  return ( in_hold_list_ == Core::HoldListType::DEBUGGER_LIST );
}
void OSExecutorModule::revive_debugged()
{
  in_hold_list_ = Core::HoldListType::NO_LIST;
  Core::scriptScheduler.revive_debugged( static_cast<Core::UOExecutor*>( &exec ) );
}

bool OSExecutorModule::critical() const
{
  return critical_;
}
void OSExecutorModule::critical( bool critical )
{
  critical_ = critical;
}

bool OSExecutorModule::warn_on_runaway() const
{
  return warn_on_runaway_;
}
void OSExecutorModule::warn_on_runaway( bool warn_on_runaway )
{
  warn_on_runaway_ = warn_on_runaway;
}

unsigned char OSExecutorModule::priority() const
{
  return priority_;
}
void OSExecutorModule::priority( unsigned char priority )
{
  priority_ = priority;
}

Core::polclock_t OSExecutorModule::sleep_until_clock() const
{
  return sleep_until_clock_;
}
void OSExecutorModule::sleep_until_clock( Core::polclock_t sleep_until_clock )
{
  sleep_until_clock_ = sleep_until_clock;
}

Core::TimeoutHandle OSExecutorModule::hold_itr() const
{
  return hold_itr_;
}
void OSExecutorModule::hold_itr( Core::TimeoutHandle hold_itr )
{
  hold_itr_ = hold_itr;
}

Core::HoldListType OSExecutorModule::in_hold_list() const
{
  return in_hold_list_;
}
void OSExecutorModule::in_hold_list( Core::HoldListType in_hold_list )
{
  in_hold_list_ = in_hold_list;
}

const int SCRIPTOPT_NO_INTERRUPT = 1;
const int SCRIPTOPT_DEBUG = 2;
const int SCRIPTOPT_NO_RUNAWAY = 3;
const int SCRIPTOPT_CAN_ACCESS_OFFLINE_MOBILES = 4;
const int SCRIPTOPT_AUXSVC_ASSUME_STRING = 5;

BObjectImp* OSExecutorModule::mf_Set_Script_Option()
{
  int optnum;
  int optval;
  if ( getParam( 0, optnum ) && getParam( 1, optval ) )
  {
    int oldval;
    switch ( optnum )
    {
    case SCRIPTOPT_NO_INTERRUPT:
      oldval = critical_ ? 1 : 0;
      critical_ = optval ? true : false;
      break;
    case SCRIPTOPT_DEBUG:
      oldval = exec.getDebugLevel();
      if ( optval )
        exec.setDebugLevel( Executor::SOURCELINES );
      else
        exec.setDebugLevel( Executor::NONE );
      break;
    case SCRIPTOPT_NO_RUNAWAY:
      oldval = warn_on_runaway_ ? 1 : 0;
      warn_on_runaway_ = !optval;
      break;
    case SCRIPTOPT_CAN_ACCESS_OFFLINE_MOBILES:
    {
      Core::UOExecutor& uoexec = static_cast<Core::UOExecutor&>( exec );
      oldval = uoexec.can_access_offline_mobiles ? 1 : 0;
      uoexec.can_access_offline_mobiles = optval ? true : false;
    }
    break;
    case SCRIPTOPT_AUXSVC_ASSUME_STRING:
    {
      Core::UOExecutor& uoexec = static_cast<Core::UOExecutor&>( exec );
      oldval = uoexec.auxsvc_assume_string ? 1 : 0;
      uoexec.auxsvc_assume_string = optval ? true : false;
    }
    break;
    default:
      return new BError( "Unknown Script Option" );
    }
    return new BLong( oldval );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* OSExecutorModule::mf_Clear_Event_Queue()  // DAVE
{
  return ( clear_event_queue() );
}

namespace
{
struct ScriptDiffData
{
  std::string name;
  u64 instructions;
  u64 pid;
  ScriptDiffData( Core::UOExecutor* ex )
      : name( ex->scriptname() ), instructions( ex->instr_cycles ), pid( ex->pid() )
  {
  }
  ScriptDiffData( Core::UOExecutor* ex, u64 instr ) : ScriptDiffData( ex )
  {
    instructions -= instr;
    auto uoemod = static_cast<Module::UOExecutorModule*>( ex->findModule( "uo" ) );
    if ( uoemod->attached_chr_ != nullptr )
      name += " (" + uoemod->attached_chr_->name() + ")";
    else if ( uoemod->attached_npc_ != nullptr )
      name += " (" + static_cast<Mobile::NPC*>( uoemod->attached_npc_ )->templatename() + ")";
    else if ( uoemod->attached_item_ )
      name += " (" + uoemod->attached_item_->name() + ")";
  }

  bool operator>( const ScriptDiffData& other ) const { return instructions > other.instructions; }
};
struct PerfData
{
  std::unordered_map<u64, ScriptDiffData> data;
  weak_ptr<Core::UOExecutor> uoexec_w;
  size_t max_scripts;
  PerfData( weak_ptr<Core::UOExecutor> weak_ex, size_t max_count )
      : data(), uoexec_w( weak_ex ), max_scripts( max_count )
  {
  }
  static void collect_perf( PerfData* data_ptr )
  {
    std::unique_ptr<PerfData> data( data_ptr );
    std::vector<ScriptDiffData> res;
    if ( !data->uoexec_w.exists() )
    {
      DEBUGLOG << "PerformanceMeasure Script has been destroyed\n";
      return;
    }
    double sum_instr( 0 );
    const auto& runlist = Core::scriptScheduler.getRunlist();
    const auto& ranlist = Core::scriptScheduler.getRanlist();
    const auto& holdlist = Core::scriptScheduler.getHoldlist();
    const auto& notimeoutholdlist = Core::scriptScheduler.getNoTimeoutHoldlist();
    auto collect = [&]( Core::UOExecutor* scr ) {
      auto itr = data->data.find( scr->pid() );
      if ( itr == data->data.end() )
        return;
      res.emplace_back( scr, itr->second.instructions );
      sum_instr += res.back().instructions;
    };
    for ( const auto& scr : runlist )
      collect( scr );
    for ( const auto& scr : ranlist )
      collect( scr );
    for ( const auto& scr : holdlist )
      collect( scr.second );
    for ( const auto& scr : notimeoutholdlist )
      collect( scr );
    std::sort( res.begin(), res.end(), std::greater<ScriptDiffData>() );

    std::unique_ptr<ObjArray> arr( new ObjArray );
    for ( size_t i = 0; i < res.size() && i < data->max_scripts; ++i )
    {
      std::unique_ptr<BStruct> elem( new BStruct );
      elem->addMember( "name", new String( res[i].name ) );
      elem->addMember( "instructions", new Double( static_cast<double>( res[i].instructions ) ) );
      elem->addMember( "pid", new BLong( static_cast<int>( res[i].pid ) ) );
      elem->addMember( "percent", new Double( res[i].instructions / sum_instr * 100.0 ) );
      arr->addElement( elem.release() );
    }
    std::unique_ptr<BStruct> result( new BStruct );
    result->addMember( "scripts", arr.release() );
    result->addMember( "total_number_observed", new BLong( static_cast<int>( res.size() ) ) );
    result->addMember( "total_instructions", new Double( sum_instr ) );
    data->uoexec_w.get_weakptr()->ValueStack.back().set( new BObject( result.release() ) );

    data->uoexec_w.get_weakptr()->revive();
  }
};

}  // namespace

BObjectImp* OSExecutorModule::mf_PerformanceMeasure()
{
  int second_delta, max_scripts;
  if ( !getParam( 0, second_delta ) || !getParam( 1, max_scripts ) )
    return new BError( "Invalid parameter type" );

  auto this_uoemod = static_cast<UOExecutorModule*>( exec.findModule( "uo" ) );
  auto this_uoexec = static_cast<Core::UOExecutor*>( &this_uoemod->exec );

  if ( !this_uoexec->suspend() )
  {
    DEBUGLOG << "Script Error in '" << this_uoexec->scriptname() << "' PC=" << this_uoexec->PC
             << ": \n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }

  const auto& runlist = Core::scriptScheduler.getRunlist();
  const auto& ranlist = Core::scriptScheduler.getRanlist();
  const auto& holdlist = Core::scriptScheduler.getHoldlist();
  const auto& notimeoutholdlist = Core::scriptScheduler.getNoTimeoutHoldlist();

  std::unique_ptr<PerfData> perf( new PerfData( this_uoexec->weakptr, max_scripts ) );
  for ( const auto& scr : runlist )
    perf->data.insert( std::make_pair( scr->pid(), ScriptDiffData( scr ) ) );
  for ( const auto& scr : ranlist )
    perf->data.insert( std::make_pair( scr->pid(), ScriptDiffData( scr ) ) );
  for ( const auto& scr : holdlist )
    perf->data.insert( std::make_pair( scr.second->pid(), ScriptDiffData( scr.second ) ) );
  for ( const auto& scr : notimeoutholdlist )
    perf->data.insert( std::make_pair( scr->pid(), ScriptDiffData( scr ) ) );

  new Core::OneShotTaskInst<PerfData*>( nullptr,
                                        Core::polclock() + second_delta * Core::POLCLOCKS_PER_SEC,
                                        PerfData::collect_perf, perf.release() );

  return new BLong( 0 );  // dummy
}

boost::asio::io_context OSExecutorModule::ios;
bool didWork = false;

void eventLoopThreadEntry( void* context )
{
  boost::asio::io_context* ios = static_cast<boost::asio::io_context*>( context );
  while ( !Clib::exit_signalled )
  {
    ios->run_for( std::chrono::seconds( 2 ) );
    if ( ios->stopped() )
      break;
  }
  if ( Clib::exit_signalled )
    ios->stop();
}

void OSExecutorModule::workQueued()
{
  if ( !didWork || ios.stopped() )
  {
    didWork = true;
    ios.restart();
    threadhelp::start_thread( eventLoopThreadEntry, "io_context event loop", (void*)&ios );
  }
}

BObjectImp* OSExecutorModule::mf_ExecuteProcess()
{
  const String* exe_name;
  Bscript::ObjArray* args;
  Bscript::BObjectImp* reserved;
  std::vector<std::string> argsVec;

  Core::UOExecutor* this_uoexec = static_cast<Core::UOExecutor*>( &exec );

  if ( childProcess_ )
  {
    return new BError( "Child process exists" );
  }

  if ( !getStringParam( 0, exe_name ) ||
       !( args = static_cast<Bscript::ObjArray*>(
              exec.getParamImp( 1, Bscript::BObjectImp::OTArray ) ) ) ||
       !getParamImp( 2, reserved ) )
    return new BError( "Invalid parameter type" );

  for ( const auto& c : args->ref_arr )
  {
    if ( !c )
      continue;
    BObjectImp* imp = c.get()->impptr();
    argsVec.push_back( imp->getStringRep() );
  }

  auto procStdout = std::make_shared<std::future<std::string>>(),
       procStderr = std::make_shared<std::future<std::string>>();

  if ( !this_uoexec->suspend() )
  {
    DEBUGLOG << "Script Error in '" << this_uoexec->scriptname() << "' PC=" << this_uoexec->PC
             << ": \n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }

  weak_ptr<Core::UOExecutor> uoexec_w = this_uoexec->weakptr;
  try
  {
    childProcess_ = Clib::make_unique<bp::child>(
        bp::exe = exe_name->value(), bp::args = argsVec, ios, bp::std_out > *procStdout,
        bp::std_err > *procStderr, bp::std_in.close(),
        bp::on_exit = [uoexec_w, procStdout, procStderr]( int exit, const std::error_code& ec_in ) {
          INFO_PRINT << "Process exited!\n";
          auto os_module = static_cast<OSExecutorModule*>( uoexec_w->findModule( "OS" ) );
          os_module->childProcess_.reset();
          Core::PolLock lck;
          INFO_PRINT << "Got lock!\n";
          if ( !uoexec_w.exists() )
          {
            DEBUGLOG << "ExecuteProcess script has been destroyed\n";
            return;
          }

          std::unique_ptr<BStruct> elem( new BStruct );
          elem->addMember( "stdout", new String( procStdout->get() ) );
          elem->addMember( "stderr", new String( procStderr->get() ) );
          elem->addMember( "code", new BLong( exit ) );

          uoexec_w.get_weakptr()->ValueStack.back().set( new BObject( elem.release() ) );
          INFO_PRINT << "Reviving!\n";
          uoexec_w.get_weakptr()->revive();
        } );
  }
  catch ( std::exception& ex )
  {
    return new BError( ex.what() );
  }
  workQueued();

  return new BLong( 0 );  // dummy
}

size_t OSExecutorModule::sizeEstimate() const
{
  size_t size = sizeof( *this );
  for ( const auto& obj : events_ )
  {
    if ( obj != nullptr )
      size += obj->sizeEstimate();
  }
  size += 3 * sizeof( Bscript::BObjectImp** ) + events_.size() * sizeof( Bscript::BObjectImp* );
  return size;
}
}  // namespace Module
}  // namespace Pol
