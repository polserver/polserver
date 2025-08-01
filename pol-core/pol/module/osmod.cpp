/** @file
 *
 * @par History
 */

#include "osmod.h"

#include "bscript/berror.h"
#include "bscript/bobject.h"
#include "bscript/bstruct.h"
#include "bscript/dict.h"
#include "bscript/impstr.h"
#include "clib/clib.h"
#include "clib/logfacility.h"
#include "clib/network/sckutil.h"
#include "clib/rawtypes.h"
#include "clib/refptr.h"
#include "clib/stlutil.h"
#include "clib/threadhelp.h"
#include "clib/weakptr.h"
#include "globals/settings.h"
#include "plib/systemstate.h"

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

#include <chrono>
#include <module_defs/os.h>

#ifdef _WIN32
#pragma comment( lib, "crypt32.lib" )
#endif
#include <ctime>
#include <curl/curl.h>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <string_view>
#include <unordered_map>

// environment variable for mf_GetEnvironmentVariable
#ifdef _MSC_VER
extern char** _environ;
char** environ_vars = _environ;
#else
extern char** environ;
char** environ_vars = environ;
#endif

namespace
{
class CurlStringList
{
public:
  CurlStringList() : resource_( nullptr ) {}

  ~CurlStringList()
  {
    if ( resource_ )
    {
      curl_slist_free_all( resource_ );
    }
  }

  void add( const std::string& str )
  {
    auto* new_resource = curl_slist_append( resource_, str.c_str() );

    if ( new_resource != resource_ && resource_ != nullptr )
    {
      curl_slist_free_all( resource_ );
    }

    resource_ = new_resource;
  }

  curl_slist* get() const { return resource_; }

private:
  curl_slist* resource_;
};

};  // namespace

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
    : TmplExecutorModule<OSExecutorModule, Core::PolModule>( exec ),
      critical_( false ),
      priority_( 1 ),
      warn_on_runaway_( true ),
      blocked_( false ),
      sleep_until_clock_( 0 ),
      hold_itr_(),
      in_hold_list_( Core::HoldListType::NO_LIST ),
      wait_type( Core::WAIT_TYPE::WAIT_UNKNOWN ),
      pid_( getnewpid( &uoexec() ) ),
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
    return new BLong( uoexec->attach_debugger() );
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
  int nsecs = exec.paramAsLong( 0 );
  SleepFor( nsecs > 0 ? static_cast<u32>( nsecs ) : 1u );
  return new BLong( 0 );
}

BObjectImp* OSExecutorModule::mf_Sleepms()
{
  int msecs = exec.paramAsLong( 0 );
  SleepForMs( msecs > 0 ? static_cast<u32>( msecs ) : 1u );
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
    auto param = exec.getParamImp( 0 );
    double nsecs = 0;

    if ( auto* long_param = impptrIf<BLong>( param ) )
    {
      nsecs = long_param->value();
      if ( !nsecs )
        return new BLong( 0 );
      if ( nsecs < 1 )
        nsecs = 1;
    }
    else if ( auto* double_param = impptrIf<Double>( param ) )
    {
      nsecs = double_param->value();
      if ( !nsecs )
        return new BLong( 0 );
      if ( nsecs < 0.01 )
        nsecs = 0.01;
    }
    else
    {
      return new BLong( 0 );
    }

    wait_type = Core::WAIT_TYPE::WAIT_EVENT;
    blocked_ = true;
    sleep_until_clock_ =
        Core::polclock() + Clib::clamp_convert<u32>( nsecs * Core::POLCLOCKS_PER_SEC );
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

    return new Core::ScriptExObjImp( &new_uoemod->uoexec() );
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

  if ( getCharacterParam( 0, chr ) && getAttributeParam( 1, attr ) )
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
  Core::UOExecutor& this_uoexec = uoexec();


  if ( this_uoexec.pChild == nullptr )
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
      Core::UOExecutor& new_uoexec = new_uoemod->uoexec();
      //      OSExecutorModule* osemod = uoexec->os_module;
      new_uoexec.pParent = &this_uoexec;
      this_uoexec.pChild = &new_uoexec;

      // we want to forcefully do this instruction over again:
      this_uoexec.PC--;  // run_script(
      this_uoexec.ValueStack.push_back(
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

  if ( this_uoexec.pChild->ValueStack.empty() )
    ret = new BLong( 1 );
  else
    ret = this_uoexec.pChild->ValueStack.back().get()->impptr()->copy();

  this_uoexec.pChild->pParent = nullptr;
  this_uoexec.pChild = nullptr;

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
  int log_verbose;
  const String* color;
  if ( !exec.getParam( 1, log_verbose ) || !exec.getStringParam( 2, color ) )
    return new BError( "Invalid parameter type" );
  std::string strval = imp->getStringRep();
  if ( log_verbose )
  {
    POLLOGLN( "[{}]: {}", exec.scriptname(), strval );
    if ( Plib::systemstate.config.enable_colored_output && color->length() )
    {
      INFO_PRINTLN( "{}syslog [{}]: {}{}", color->value(), exec.scriptname(), strval,
                    Clib::Logging::CONSOLE_RESET_COLOR );
    }
    else
    {
      INFO_PRINTLN( "syslog [{}]: {}", exec.scriptname(), strval );
    }
  }
  else
  {
    if ( Plib::systemstate.config.enable_colored_output && color->length() )
    {
      POLLOGLN( strval );
      INFO_PRINTLN( "{}{}{}", color->value(), strval, Clib::Logging::CONSOLE_RESET_COLOR );
    }
    else
    {
      POLLOG_INFOLN( strval );
    }
  }
  return new BLong( 1 );
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
  const String* host;
  const String* scriptname_str;
  BObjectImp* scriptparam;
  unsigned short port;
  int assume_string_int;
  int keep_connection_int;
  int ignore_line_breaks_int;
  if ( !getStringParam( 0, host ) || !getParam( 1, port ) || !getStringParam( 2, scriptname_str ) ||
       !getParamImp( 3, scriptparam ) || !getParam( 4, assume_string_int ) ||
       !getParam( 5, keep_connection_int ) || !getParam( 6, ignore_line_breaks_int ) )
    return new BError( "Invalid parameter type" );

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
  Core::UOExecutor& this_uoexec = uoexec();
  if ( !this_uoexec.suspend() )
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tThe execution of this script can't be blocked!",
        this_uoexec.scriptname(), this_uoexec.PC );
    return new Bscript::BError( "Script can't be blocked" );
  }

  weak_ptr<Core::UOExecutor> uoexec_w = this_uoexec.weakptr;
  std::string hostname( host->value() );
  bool assume_string = assume_string_int != 0;
  bool keep_connection = keep_connection_int != 0;
  bool ignore_line_breaks = ignore_line_breaks_int != 0;
  auto* paramobjimp_raw = scriptparam->copy();  // prevent delete
  Core::networkManager.auxthreadpool->push(
      [uoexec_w, sd, hostname, port, paramobjimp_raw, assume_string, keep_connection,
       ignore_line_breaks]()
      {
        Clib::Socket s;
        std::unique_ptr<Network::AuxClientThread> client;
        bool success_open = s.open( hostname.c_str(), port );
        {
          Core::PolLock lck;
          std::unique_ptr<BObjectImp> paramobjimp( paramobjimp_raw );
          if ( !uoexec_w.exists() )
          {
            DEBUGLOGLN( "OpenConnection Script has been destroyed" );
            s.close();
            return;
          }
          if ( !success_open )
          {
            uoexec_w.get_weakptr()->ValueStack.back().set(
                new BObject( new BError( "Error connecting to client" ) ) );
            uoexec_w.get_weakptr()->revive();
            return;
          }
          uoexec_w.get_weakptr()->ValueStack.back().set( new BObject( new BLong( 1 ) ) );
          uoexec_w.get_weakptr()->revive();
          client.reset( new Network::AuxClientThread( sd, std::move( s ), paramobjimp.release(),
                                                      assume_string, keep_connection,
                                                      ignore_line_breaks ) );
        }
        if ( client )
          client->run();
      } );

  return new BLong( 0 );
}

size_t curlWriteCallback( void* contents, size_t size, size_t nmemb, void* userp )
{
  ( static_cast<std::string*>( userp ) )->append( static_cast<char*>( contents ), size * nmemb );
  return size * nmemb;
}

const int HTTPREQUEST_EXTENDED_RESPONSE = 0x0001;

struct CurlHeaderData
{
  std::optional<std::string> statusText;
  std::map<std::string, std::string> headers = {};
};

size_t curlReadHeaderCallback( char* buffer, size_t size, size_t nitems, void* userdata )
{
  auto headerData = static_cast<CurlHeaderData*>( userdata );
  std::string value( buffer, size * nitems );

  // statusText is set after parsing the first header line
  if ( !headerData->statusText.has_value() )
  {
    std::string statusText = "";
    if ( value.rfind( "HTTP/", 0 ) == 0 )
    {
      // Find space after "HTTP/x" (before status code)
      size_t pos = value.find( " " );
      if ( pos != std::string::npos )
      {
        // Find space after status code (before status text)
        pos = value.find( " ", pos + 1 );
        if ( pos != std::string::npos )
        {
          statusText = Clib::strtrim( value.substr( pos + 1 ) );
        }
      }
    }
    headerData->statusText.emplace( statusText );
  }
  else
  {
    size_t pos = value.find( ":" );
    if ( pos != std::string::npos )
    {
      std::string headerName = Clib::strlowerASCII( value.substr( 0, pos ) );
      std::string headerValue = Clib::strtrim( value.substr( pos + 1 ) );

      // If the header already exists, append it (comma-separated) to previous value
      if ( headerData->headers.count( headerName ) )
      {
        auto& existing = headerData->headers.at( headerName );
        existing.append( ", " );
        existing.append( headerValue );
      }
      else
      {
        headerData->headers.emplace( headerName, headerValue );
      }
    }
  }

  return nitems * size;
}

BObjectImp* OSExecutorModule::mf_HTTPRequest()
{
  const String *url, *method;
  BObjectImp* options;
  int flags;
  if ( !getStringParam( 0, url ) || !getStringParam( 1, method ) || !getParamImp( 2, options ) ||
       !getParam( 3, flags ) )
    return new BError( "Invalid parameter type" );
  Core::UOExecutor& this_uoexec = uoexec();
  weak_ptr<Core::UOExecutor> uoexec_w = this_uoexec.weakptr;

  std::shared_ptr<CURL> curl_sp( curl_easy_init(), curl_easy_cleanup );
  CURL* curl = curl_sp.get();
  if ( !curl )
    return new BError( "curl_easy_init() failed" );
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

  if ( !this_uoexec.suspend() )
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tThe execution of this script can't be blocked!",
        this_uoexec.scriptname(), this_uoexec.PC );
    return new Bscript::BError( "Script can't be blocked" );
  }

  Core::networkManager.auxthreadpool->push(
      [uoexec_w, curl_sp, chunk, flags]()
      {
        CURL* curl = curl_sp.get();
        CURLcode res;
        std::string readBuffer;
        CurlHeaderData headerData;

        if ( flags == HTTPREQUEST_EXTENDED_RESPONSE )
        {
          curl_easy_setopt( curl, CURLOPT_HEADERFUNCTION, curlReadHeaderCallback );
          curl_easy_setopt( curl, CURLOPT_HEADERDATA, &headerData );
        }

        curl_easy_setopt( curl, CURLOPT_WRITEDATA, &readBuffer );

        /* Perform the request, res will get the return code */
        res = curl_easy_perform( curl );
        if ( chunk != nullptr )
          curl_slist_free_all( chunk );
        {
          Core::PolLock lck;

          if ( !uoexec_w.exists() )
          {
            DEBUGLOGLN( "HTTPRequest Script has been destroyed" );
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
            if ( flags == HTTPREQUEST_EXTENDED_RESPONSE )
            {
              auto response = std::make_unique<Bscript::BDictionary>();
              auto headers = std::make_unique<Bscript::BDictionary>();
              long http_code = 0;

              res = curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &http_code );
              if ( res == CURLE_OK )
              {
                response->addMember( new String( "status" ), new BLong( http_code ) );
              }
              else
              {
                response->addMember( new String( "status" ),
                                     new BError( curl_easy_strerror( res ) ) );
              }

              response->addMember( new String( "statusText" ),
                                   new String( headerData.statusText.value_or( "" ) ) );

              response->addMember( new String( "body" ), new String( readBuffer ) );

              for ( auto const& [key, value] : headerData.headers )
              {
                headers->addMember( new String( key ), new String( value ) );
              }

              response->addMember( new String( "headers" ), headers.release() );

              uoexec_w.get_weakptr()->ValueStack.back().set( new BObject( response.release() ) );
            }
            else
            {
              // TODO: no sanitize happens, optional function param iso/utf8 encoding, or
              // parse the header of the http answer?
              uoexec_w.get_weakptr()->ValueStack.back().set(
                  new BObject( new String( readBuffer ) ) );
            }
          }

          uoexec_w.get_weakptr()->revive();
        }

        /* always cleanup */
        // curl_easy_cleanup() is performed when the shared pointer deallocates
      } );

  return new BLong( 0 );
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
          INFO_PRINTLN( "Event queue for {} is full, discarding event.", exec.scriptname() );
          ExecutorModule* em = exec.findModule( "npc" );
          if ( em )
          {
            NPCExecutorModule* npcemod = static_cast<NPCExecutorModule*>( em );
            INFO_PRINTLN( "NPC Serial: {:#x}{}", npcemod->controlled_npc().serial,
                          npcemod->controlled_npc().pos() );
          }

          INFO_PRINTLN( "Event: {}", ob->getStringRep() );
        }
        return false;  // Event-queue is full
      }
    }
  }

  return true;  // Event was successfully sent (perhaps by discarding old events)
}

void OSExecutorModule::SleepFor( u32 nsecs )
{
  if ( !nsecs )
    return;
  blocked_ = true;
  wait_type = Core::WAIT_TYPE::WAIT_SLEEP;
  sleep_until_clock_ = Core::polclock() + nsecs * Core::POLCLOCKS_PER_SEC;
}

void OSExecutorModule::SleepForMs( u32 msecs )
{
  if ( !msecs )
    return;
  blocked_ = true;
  wait_type = Core::WAIT_TYPE::WAIT_SLEEP;
  sleep_until_clock_ = Core::polclock() + msecs * Core::POLCLOCKS_PER_SEC / 1000;
  if ( !sleep_until_clock_ )
    sleep_until_clock_ = 1;
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
    Core::scriptScheduler.revive_timeout( &uoexec(), hold_itr_ );
  }
  else if ( in_hold_list_ == Core::HoldListType::NOTIMEOUT_LIST )
  {
    in_hold_list_ = Core::HoldListType::NO_LIST;
    Core::scriptScheduler.revive_notimeout( &uoexec() );
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
  Core::scriptScheduler.revive_debugged( &uoexec() );
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
const int SCRIPTOPT_SURVIVE_ATTACHED_DISCONNECT = 6;

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
      Core::UOExecutor& uoex = uoexec();
      oldval = uoex.can_access_offline_mobiles_ ? 1 : 0;
      uoex.can_access_offline_mobiles_ = optval ? true : false;
    }
    break;
    case SCRIPTOPT_AUXSVC_ASSUME_STRING:
    {
      Core::UOExecutor& uoex = uoexec();
      oldval = uoex.auxsvc_assume_string ? 1 : 0;
      uoex.auxsvc_assume_string = optval ? true : false;
    }
    break;
    case SCRIPTOPT_SURVIVE_ATTACHED_DISCONNECT:
    {
      Core::UOExecutor& uoex = uoexec();
      oldval = uoex.survive_attached_disconnect ? 1 : 0;
      uoex.survive_attached_disconnect = optval ? true : false;
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
    else if ( uoemod->attached_item_.get() )
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
      DEBUGLOGLN( "PerformanceMeasure Script has been destroyed" );
      return;
    }
    double sum_instr( 0 );
    const auto& runlist = Core::scriptScheduler.getRunlist();
    const auto& ranlist = Core::scriptScheduler.getRanlist();
    const auto& holdlist = Core::scriptScheduler.getHoldlist();
    const auto& notimeoutholdlist = Core::scriptScheduler.getNoTimeoutHoldlist();
    auto collect = [&]( Core::UOExecutor* scr )
    {
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

  auto& this_uoexec = uoexec();

  if ( !this_uoexec.suspend() )
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tThe execution of this script can't be blocked!",
        this_uoexec.scriptname(), this_uoexec.PC );
    return new Bscript::BError( "Script can't be blocked" );
  }

  const auto& runlist = Core::scriptScheduler.getRunlist();
  const auto& ranlist = Core::scriptScheduler.getRanlist();
  const auto& holdlist = Core::scriptScheduler.getHoldlist();
  const auto& notimeoutholdlist = Core::scriptScheduler.getNoTimeoutHoldlist();

  std::unique_ptr<PerfData> perf( new PerfData( this_uoexec.weakptr, max_scripts ) );
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

BObjectImp* OSExecutorModule::mf_LoadExportedScript()
{
  Core::UOExecutor& this_uoexec = uoexec();
  if ( this_uoexec.pChild == nullptr )
  {
    const String* scriptname_str;
    ObjArray* arr;
    if ( !exec.getStringParam( 0, scriptname_str ) || !getObjArrayParam( 1, arr ) )
      return new BError( "Invalid parameter type" );
    Core::ScriptDef sd;
    if ( !sd.config_nodie( scriptname_str->value(), exec.prog()->pkg, "scripts/" ) )
      return new BError( "Error in script name" );
    if ( !sd.exists() )
      return new BError( "Script " + sd.name() + " does not exist." );
    ref_ptr<Bscript::EScriptProgram> program = find_script2( sd );
    if ( program.get() == nullptr )
    {
      ERROR_PRINTLN( "Error reading script {}", sd.name() );
      return new Bscript::BError( "Unable to read script" );
    }
    Core::UOExecutor* uoexec = Core::create_script_executor();
    uoexec->keep_alive( true );
    Core::add_common_exmods( *uoexec );
    uoexec->addModule( new Module::UOExecutorModule( *uoexec ) );

    uoexec->setProgram( program.get() );
    if ( program->haveProgram )
    {
      for ( int i = (int)( arr->ref_arr.size() ) - 1; i >= 0; --i )
        uoexec->pushArg( arr->ref_arr[i].get()->impptr() );
    }
    if ( this_uoexec.critical() )  // execute directy
    {
      uoexec->exec();
      BObjectImp* ret;
      if ( uoexec->error() )
        ret = new BLong( 0 );
      else if ( uoexec->ValueStack.empty() )
        ret = new BLong( 1 );
      else
        ret = uoexec->ValueStack.back().get()->impptr()->copy();
      uoexec->set_running_to_completion( false );
      uoexec->suspend();
      Core::scriptScheduler.schedule( uoexec );

      auto array = std::make_unique<Bscript::ObjArray>();
      array->addElement( new Core::ExportScriptObjImp( uoexec ) );
      array->addElement( ret );

      return array.release();
    }
    else
    {
      Core::scriptScheduler.schedule( uoexec );

      uoexec->pParent = &this_uoexec;
      this_uoexec.pChild = uoexec;

      this_uoexec.PC--;
      // need to fill the valuestack equal to param count (-the return value)
      this_uoexec.ValueStack.push_back( BObjectRef( new BObject( UninitObject::create() ) ) );
      suspend();

      return UninitObject::create();
    }
  }
  else  // reentry
  {
    BObjectImp* ret;
    if ( this_uoexec.pChild->error() )
      ret = new BLong( 0 );
    else if ( this_uoexec.pChild->ValueStack.empty() )
      ret = new BLong( 1 );
    else
    {
      ret = this_uoexec.pChild->ValueStack.back().get()->impptr()->copy();
      this_uoexec.pChild->ValueStack.pop_back();
    }
    auto array = std::make_unique<Bscript::ObjArray>();
    array->addElement( new Core::ExportScriptObjImp( this_uoexec.pChild ) );
    array->addElement( ret );

    this_uoexec.pChild->pParent = nullptr;
    this_uoexec.pChild = nullptr;

    return array.release();
  }
}

BObjectImp* OSExecutorModule::mf_GetEnvironmentVariable()
{
  const auto& allowed_vars = Plib::systemstate.config.allowed_environmentvariables_access;
  if ( allowed_vars.empty() )
    return new BError( "Environment Variable access disallowed due to pol.cfg setting" );
  const String* env_name;
  if ( !exec.getStringParam( 0, env_name ) )
    return new BError( "Invalid parameter type" );

  bool all_allowed = allowed_vars.size() == 1 && allowed_vars[0] == "*";

  if ( env_name->length() == 0 )
  {
    auto envs = std::make_unique<Bscript::BDictionary>();
    for ( char** current = environ_vars; *current; ++current )
    {
      std::string_view env( *current );
      size_t pos = env.find_first_of( "=" );
      if ( pos == std::string_view::npos )
        continue;
      auto key = env.substr( 0, pos );
      auto key_lowered = Clib::strlowerASCII( std::string{ key } );
      auto val = env.substr( pos + 1 );

      if ( all_allowed || std::find( allowed_vars.begin(), allowed_vars.end(), key_lowered ) !=
                              allowed_vars.end() )
      {
        envs->addMember( new String( key, String::Tainted::YES ),
                         new String( val, String::Tainted::YES ) );
      }
    }
    return envs.release();
  }
  else
  {
    if ( !all_allowed )
    {
      auto name_lowered = Clib::strlowerASCII( env_name->value() );
      if ( std::find( allowed_vars.begin(), allowed_vars.end(), name_lowered ) ==
           allowed_vars.end() )
        return new BError( "Environment Variable access disallowed due to pol.cfg setting" );
    }

    const char* env_val = std::getenv( env_name->data() );
    if ( !env_val )
      return new BError( "Environment variable not found" );
    return new String( env_val, String::Tainted::YES );
  }
}

BObjectImp* OSExecutorModule::mf_SendEmail()
{
  if ( Core::settingsManager.email_cfg.url.empty() )
    return new BError( "Email settings are not configured" );


  const String *from, *subject, *body, *contentType;
  BObjectImp *recipient, *bcc;

  if ( !( getStringParam( 0, from ) && getParamImp( 1, recipient ) &&
          getStringParam( 2, subject ) && getStringParam( 3, body ) && getParamImp( 4, bcc ) &&
          getStringParam( 5, contentType ) ) )
  {
    return new BError( "Invalid parameter type" );
  }

  std::shared_ptr<CURL> curl_sp( curl_easy_init(), curl_easy_cleanup );
  CURL* curl = curl_sp.get();
  if ( !curl )
  {
    return new BError( "curl_easy_init() failed" );
  }

  auto headers_slist = std::make_shared<CurlStringList>();
  auto recipients_slist = std::make_shared<CurlStringList>();
  std::string to_header_value;

  auto extract_email_address = []( const String* email_string )
  {
    const auto& input = email_string->value();
    auto start = input.find( '<' );
    auto end = input.find( '>' );

    if ( start != std::string::npos && end != std::string::npos && end > start + 1 )
    {
      return input.substr( start + 1, end - start - 1 );
    }

    // No angle brackets: assume the whole input is the email address
    return input;
  };

  auto handle_recipients = [&]( BObjectImp* string_or_array, bool is_bcc ) -> BObjectImp*
  {
    // The `to` or `bcc` can be falsey to specify no recipient for this field.
    if ( !string_or_array->isTrue() )
      return nullptr;

    if ( auto* recipient_string = impptrIf<String>( string_or_array ) )
    {
      if ( !is_bcc )
      {
        if ( !to_header_value.empty() )
          to_header_value += ", ";

        to_header_value += recipient_string->data();
      }

      recipients_slist->add( extract_email_address( recipient_string ) );
    }
    else if ( auto* recipient_array = impptrIf<ObjArray>( string_or_array ) )
    {
      for ( const auto& elem : recipient_array->ref_arr )
      {
        if ( auto* this_recipient = elem->impptr_if<String>() )
        {
          // Bcc is not added to the header, and only included in CURLOPT_MAIL_RCPT
          if ( !is_bcc )
          {
            if ( !to_header_value.empty() )
              to_header_value += ", ";

            to_header_value += this_recipient->data();
          }

          recipients_slist->add( extract_email_address( this_recipient ) );
        }
        else
        {
          return new BError( "Invalid recipient type in array" );
        }
      }
    }
    else
    {
      return new BError( "Invalid recipient type" );
    }

    return nullptr;
  };

  if ( auto res = curl_easy_setopt( curl, CURLOPT_URL, Core::settingsManager.email_cfg.url.data() );
       res != CURLE_OK )
  {
    return new BError( "curl_easy_setopt(CURLOPT_URL) failed: " +
                       std::string( curl_easy_strerror( res ) ) );
  }

  if ( !Core::settingsManager.email_cfg.username.empty() )
  {
    if ( auto res = curl_easy_setopt( curl, CURLOPT_USERNAME,
                                      Core::settingsManager.email_cfg.username.c_str() );
         res != CURLE_OK )
    {
      return new BError( "curl_easy_setopt(CURLOPT_USERNAME) failed: " +
                         std::string( curl_easy_strerror( res ) ) );
    }
  }

  if ( !Core::settingsManager.email_cfg.password.empty() )
  {
    if ( auto res = curl_easy_setopt( curl, CURLOPT_PASSWORD,
                                      Core::settingsManager.email_cfg.password.c_str() );
         res != CURLE_OK )
    {
      return new BError( "curl_easy_setopt(CURLOPT_PASSWORD) failed: " +
                         std::string( curl_easy_strerror( res ) ) );
    }
  }

  if ( Core::settingsManager.email_cfg.use_tls )
  {
    if ( auto res = curl_easy_setopt( curl, CURLOPT_USE_SSL, CURLUSESSL_ALL ); res != CURLE_OK )
    {
      return new BError( "curl_easy_setopt(CURLOPT_USE_SSL) failed: " +
                         std::string( curl_easy_strerror( res ) ) );
    }
  }

  if ( !Core::settingsManager.email_cfg.ca_file.empty() )
  {
    if ( auto res = curl_easy_setopt( curl, CURLOPT_CAINFO,
                                      Core::settingsManager.email_cfg.ca_file.c_str() );
         res != CURLE_OK )
    {
      return new BError( "curl_easy_setopt(CURLOPT_CAINFO) failed: " +
                         std::string( curl_easy_strerror( res ) ) );
    }
  }

  if ( auto res =
           curl_easy_setopt( curl, CURLOPT_MAIL_FROM, extract_email_address( from ).c_str() );
       res != CURLE_OK )
  {
    return new BError( "curl_easy_setopt(CURLOPT_MAIL_FROM) failed: " +
                       std::string( curl_easy_strerror( res ) ) );
  }

  if ( auto* recipient_to_error = handle_recipients( recipient, false ) )  // Handle To recipients
  {
    return recipient_to_error;
  }

  if ( auto* recipient_bcc_error = handle_recipients( bcc, true ) )  // Handle Bcc recipients
  {
    return recipient_bcc_error;
  }

  if ( auto res = curl_easy_setopt( curl, CURLOPT_MAIL_RCPT, recipients_slist->get() );
       res != CURLE_OK )
  {
    return new BError( "curl_easy_setopt(CURLOPT_MAIL_RCPT) failed: " +
                       std::string( curl_easy_strerror( res ) ) );
  }

  std::string date_header_value = fmt::format(
      "{:%a, %d %b %Y %H:%M:%S %z}",
      std::chrono::time_point_cast<std::chrono::seconds>( std::chrono::system_clock::now() ) );

  std::vector<std::string> email_headers{ "Date: " + date_header_value,  //
                                          "To: " + to_header_value,      //
                                          "From: " + from->value(),      //
                                          "Subject: " + subject->value() };

  for ( const auto& header : email_headers )
    headers_slist->add( header );

  if ( auto res = curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers_slist->get() );
       res != CURLE_OK )
  {
    return new BError( "curl_easy_setopt(CURLOPT_HTTPHEADER) failed: " +
                       std::string( curl_easy_strerror( res ) ) );
  }

  std::shared_ptr<curl_mime> mime( curl_mime_init( curl ), curl_mime_free );

  curl_mimepart* part = curl_mime_addpart( mime.get() );

  if ( !part )
  {
    return new BError( "curl_mime_addpart() failed" );
  }

  // `curl_mime_data` copies the string into its internal structure, so using this local is
  // okay. Ref: https://github.com/curl/curl/blob/curl-8_2_1/lib/mime.c#L1380-L1390
  curl_mime_data( part, body->data(), CURL_ZERO_TERMINATED );

  // `curl_mime_type` copies the string into its internal structure, so using this local is
  // okay. Ref: https://github.com/curl/curl/blob/curl-8_2_1/lib/mime.c#L1461
  curl_mime_type( part, contentType->data() );

  if ( auto res = curl_easy_setopt( curl, CURLOPT_MIMEPOST, mime.get() ); res != CURLE_OK )
  {
    return new BError( "curl_easy_setopt(CURLOPT_MIMEPOST) failed: " +
                       std::string( curl_easy_strerror( res ) ) );
  }

  Core::UOExecutor& this_uoexec = uoexec();

  if ( !this_uoexec.suspend() )
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tThe execution of this script can't be blocked!",
        this_uoexec.scriptname(), this_uoexec.PC );
    return new Bscript::BError( "Script can't be blocked" );
  }

  weak_ptr<Core::UOExecutor> uoexec_w = this_uoexec.weakptr;

  Core::networkManager.auxthreadpool->push(
      [uoexec_w, curl_sp, recipients_slist, headers_slist, mime]()
      {
        CURL* curl = curl_sp.get();

        auto res = curl_easy_perform( curl );

        {
          Core::PolLock lck;

          if ( !uoexec_w.exists() )
          {
            DEBUGLOGLN( "SendEmail Script has been destroyed" );
            return;
          }
          if ( res != CURLE_OK )
            uoexec_w.get_weakptr()->ValueStack.back().set(
                new BObject( new BError( curl_easy_strerror( res ) ) ) );
          else
            uoexec_w.get_weakptr()->ValueStack.back().set( new BObject( new BLong( 1 ) ) );

          uoexec_w.get_weakptr()->revive();
        }
      }

      /* always cleanup */
      // curl_easy_cleanup(), curl_slist_free_all(), curl_mime_free() are
      // performed when the shared pointer deallocates
  );

  return new BLong( 0 );
}


size_t OSExecutorModule::sizeEstimate() const
{
  size_t size = sizeof( *this );
  for ( const auto& obj : events_ )
  {
    if ( obj != nullptr )
      size += obj->sizeEstimate();
  }
  size += Clib::memsize( events_ );
  return size;
}
}  // namespace Module
}  // namespace Pol
