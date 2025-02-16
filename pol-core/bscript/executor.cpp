/** @file
 *
 * @par History
 * - 2005/09/16 Shinigami: added scripts_thread_script* to support better debugging
 * - 2006/01/27 Shinigami: added missing TOK_BS* to Executor::GetInstrFunc
 * - 2006/06/10 Shinigami: getParamImp/2 - better Error Message added
 * - 2006/10/07 Shinigami: FreeBSD fix - changed __linux__ to __unix__
 * - 2007/07/07 Shinigami: added code to analyze memoryleaks in initForFnCall() (needs defined
 * MEMORYLEAK)
 * - 2009/07/19 MuadDib: Executor::ins_member() Removed, due to no longer used since case
 * optimization code added.
 * - 2009/09/05 Turley: Added struct .? and .- as shortcut for .exists() and .erase()
 */


#include "executor.h"
#include "executor.inl.h"

#include "../clib/clib.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "bclassinstance.h"
#include "berror.h"
#include "config.h"
#include "continueimp.h"
#include "contiter.h"
#include "dict.h"
#include "eprog.h"
#include "escriptv.h"
#include "execmodl.h"
#include "fmodule.h"
#include "impstr.h"
#include "objmethods.h"
#include "str.h"
#include "token.h"
#include "tokens.h"
#include <iterator>
#ifdef MEMORYLEAK
#include "../clib/mlog.h"
#endif

#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index_container.hpp>

#include <cstdlib>
#include <cstring>
#include <exception>
#include <numeric>

#ifdef ESCRIPT_PROFILE
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/time.h>
#endif
#endif

namespace Pol
{
namespace Bscript
{
std::set<Executor*> executor_instances;

#ifdef ESCRIPT_PROFILE
escript_profile_map EscriptProfileMap;
#endif

void display_executor_instances()
{
  for ( const auto& ex : executor_instances )
  {
    // Fix for crashes due to orphaned script instances.
    if ( !ex->empty_scriptname() )
      INFO_PRINTLN( ex->scriptname() );
  }
}

ExecutorDebugEnvironment::ExecutorDebugEnvironment( std::weak_ptr<ExecutorDebugListener> listener,
                                                    bool set_attaching )
    : debug_state( set_attaching ? ExecutorDebugState::ATTACHING : ExecutorDebugState::RUN ),
      breakpoints(),
      break_on_linechange_from{ ~0u, ~0u },
      bp_skip( ~0u ),
      listener( listener )
{
}

size_t ExecutorDebugEnvironment::sizeEstimate() const
{
  size_t size = sizeof( *this ) + Clib::memsize( breakpoints ) + Clib::memsize( tmpbreakpoints );
  return size;
}

bool ExecutorDebugEnvironment::on_instruction( Executor& ex )
{
  switch ( debug_state )
  {
  case ( ExecutorDebugState::ATTACHING ):
  {
    debug_state = ExecutorDebugState::ATTACHED;
    ex.sethalt( true );
    return false;
  }
  case ( ExecutorDebugState::INS_TRACE ):
  {
    // let this instruction through.
    debug_state = ExecutorDebugState::ATTACHED;
    ex.sethalt( true );
    // but let this instruction execute.
    break;
  }
  case ( ExecutorDebugState::STEP_INTO ):
  {
    debug_state = ExecutorDebugState::STEPPING_INTO;
    // let this instruction execute.
    break;
  }
  case ( ExecutorDebugState::STEPPING_INTO ):
  {
    if ( ex.prog()->dbg_ins_statementbegin.size() > ex.PC &&
         ex.prog()->dbg_ins_statementbegin[ex.PC] )
    {
      tmpbreakpoints.insert( ex.PC );
      // and let breakpoint processing catch it below.
    }
    break;
  }
  case ( ExecutorDebugState::STEP_OVER ):
  {
    break_on_linechange_from = { ex.prog()->dbg_linenum[ex.PC], ex.ControlStack.size() };
    debug_state = ExecutorDebugState::STEPPING_OVER;
    break;
  }
  case ( ExecutorDebugState::STEPPING_OVER ):
  {
    if ( ex.ControlStack.size() < break_on_linechange_from.control ||
         ( ex.ControlStack.size() == break_on_linechange_from.control &&
           ex.prog()->dbg_linenum[ex.PC] != break_on_linechange_from.line ) )
    {
      debug_state = ExecutorDebugState::ATTACHED;
      break_on_linechange_from = { ~0u, ~0u };
      ex.sethalt( true );
      return false;
    }
    break;
  }
  case ( ExecutorDebugState::STEP_OUT ):
  {
    if ( ex.ControlStack.size() > 0 )
    {
      tmpbreakpoints.insert( ex.ControlStack.back().PC );
    }
    debug_state = ExecutorDebugState::RUN;
    break;
  }
  case ( ExecutorDebugState::RUN ):
  {
    // do nothing
    break;
  }
  case ( ExecutorDebugState::ATTACHED ):
  {
    return false;
  }
  case ( ExecutorDebugState::BREAK_INTO ):
  {
    debug_state = ExecutorDebugState::ATTACHED;
    ex.sethalt( true );
    return false;
    break;
  }
  }

  // check for breakpoints on this instruction
  if ( ( breakpoints.count( ex.PC ) || tmpbreakpoints.count( ex.PC ) ) && bp_skip != ex.PC &&
       !ex.halt() )
  {
    tmpbreakpoints.erase( ex.PC );
    bp_skip = ex.PC;
    debug_state = ExecutorDebugState::ATTACHED;
    ex.sethalt( true );
    return false;
  }
  bp_skip = ~0u;

  return true;
}

extern int executor_count;
Clib::SpinLock Executor::_executor_lock;
Executor::Executor()
    : done( 0 ),
      error_( false ),
      halt_( false ),
      run_ok_( false ),
      debug_level( NONE ),
      PC( 0 ),
      Globals2( std::make_shared<BObjectRefVec>() ),
      Locals2( new BObjectRefVec ),
      nLines( 0 ),
      current_module_function( nullptr ),
      prog_ok_( false ),
      viewmode_( false ),
      runs_to_completion_( false ),
      dbg_env_( nullptr ),
      func_result_( nullptr )
{
  Clib::SpinLockGuard lock( _executor_lock );
  ++executor_count;
  executor_instances.insert( this );

  if ( !UninitObject::SharedInstance )
  {
    UninitObject::SharedInstance = new UninitObject;
    UninitObject::SharedInstanceOwner.set( UninitObject::SharedInstance );
  }
}

Executor::~Executor()
{
  {
    Clib::SpinLockGuard lock( _executor_lock );
    --executor_count;
    executor_instances.erase( this );
  }
  cleanup();
}
void Executor::cleanup()
{
  if ( dbg_env_ )
  {
    if ( std::shared_ptr<ExecutorDebugListener> listener = dbg_env_->listener.lock() )
      listener->on_destroy();
  }

  delete Locals2;
  Locals2 = nullptr;

  while ( !upperLocals2.empty() )
  {
    delete upperLocals2.back();
    upperLocals2.pop_back();
  }

  execmodules.clear();
  Clib::delete_all( availmodules );
}

bool Executor::AttachFunctionalityModules()
{
  for ( auto& fm : prog_->modules )
  {
    // if no function in the module is actually called, don't go searching for it.
    if ( fm->functions.empty() )
    {
      execmodules.push_back( nullptr );
      continue;
    }

    ExecutorModule* em = findModule( fm->modulename );
    execmodules.push_back( em );
    if ( em == nullptr )
    {
      ERROR_PRINTLN( "WARNING: {}: Unable to find module {}", scriptname(), fm->modulename.get() );
      return false;
    }

    if ( !fm->have_indexes )
    {
      /*
          FIXE: Possible optimization: store these function indexes in the
          EScriptProgram object, since those are cached.  Then, we only
          have to find the module index.
          */
      for ( unsigned fidx = 0; fidx < fm->functions.size(); fidx++ )
      {
        ModuleFunction* func = fm->functions[fidx];
        // FIXME: should check number of params, blah.
        if ( !func->name.get().empty() )
        {
          func->funcidx = em->functionIndex( func->name.get() );
          if ( func->funcidx == -1 )
          {
            ERROR_PRINTLN( "Unable to find {}::{}", fm->modulename.get(), func->name.get() );
            return false;
          }
        }
      }
      fm->have_indexes = true;
    }
  }
  return true;
}

int Executor::getParams( unsigned howMany )
{
  if ( howMany )
  {
    fparams.resize( howMany );
    for ( int i = howMany - 1; i >= 0; --i )
    {
      if ( ValueStack.empty() )
      {
        POLLOG_ERRORLN( "Fatal error: Value Stack Empty! ({},PC={})", prog_->name, PC );
        seterror( true );
        return -1;
      }
      fparams[i] = ValueStack.back();
      ValueStack.pop_back();
    }
  }
  expandParams();
  return 0;
}

void Executor::expandParams()
{
  for ( auto i = static_cast<int>( fparams.size() ) - 1; i >= 0; --i )
  {
    if ( auto* spread = fparams[i]->impptr_if<BSpread>() )
    {
      // defer destruction
      BObjectRef obj( spread );

      // Remove the spread
      fparams.erase( fparams.begin() + i );

      BObjectRef refIter( new BObject( UninitObject::create() ) );

      auto pIter = std::unique_ptr<ContIterator>(
          spread->object->impptr()->createIterator( refIter.get() ) );

      BObject* next = pIter->step();

      int added = 0;
      while ( next != nullptr )
      {
        fparams.insert( fparams.begin() + i + added, BObjectRef( next ) );
        next = pIter->step();
        added++;
      }
      i += added;
    }
  }
}

void Executor::cleanParams()
{
  fparams.clear();
}

int Executor::makeString( unsigned param )
{
  BObject* obj = getParam( param );
  if ( !obj )
    return -1;
  if ( obj->isa( BObjectImp::OTString ) )
    return 0;

  fparams[param].set( new BObject( new String( obj->impref() ) ) );

  return 0;
}

const char* Executor::paramAsString( unsigned param )
{
  makeString( param );
  BObjectImp* objimp = fparams[param]->impptr();

  String* str = (String*)objimp;
  return str ? str->data() : "";
}

int Executor::makeDouble( unsigned param )
{
  BObject* obj = getParam( param );
  if ( !obj )
    return -1;
  if ( obj->isa( BObjectImp::OTDouble ) )
    return 0;
  if ( auto* v = obj->impptr_if<BLong>() )
    fparams[param].set( new BObject( new Double( v->value() ) ) );
  else
    fparams[param].set( new BObject( new Double( 0.0 ) ) );

  return 0;
}

double Executor::paramAsDouble( unsigned param )
{
  makeDouble( param );
  if ( auto* v = getParam( param )->impptr_if<Double>() )
    return v->value();
  return 0.0;
}

int Executor::paramAsLong( unsigned param )
{
  BObjectImp* objimp = getParam( param )->impptr();
  if ( auto* l = impptrIf<BLong>( objimp ) )
    return l->value();
  else if ( auto* d = impptrIf<Double>( objimp ) )
    return static_cast<int>( d->value() );
  return 0;
}
BObject* Executor::getParam( unsigned param )
{
  passert_r( param < fparams.size(), "Script Error in '" + scriptname() +
                                         ": Less Parameter than expected. " +
                                         "You should use *.em-files shipped with this Core and "
                                         "recompile ALL of your Scripts _now_! RTFM" );

  return fparams[param].get();
}

BObjectImp* Executor::getParamImp( unsigned param )
{
  passert_r( param < fparams.size(), "Script Error in '" + scriptname() +
                                         ": Less Parameter than expected. " +
                                         "You should use *.em-files shipped with this Core and "
                                         "recompile ALL of your Scripts _now_! RTFM" );

  return fparams[param].get()->impptr();
}

BObject* Executor::getParamObj( unsigned param )
{
  if ( fparams.size() > param )
  {
    return fparams[param].get();
  }
  else
  {
    return nullptr;
  }
}

BObjectImp* Executor::getParamImp( unsigned param, BObjectImp::BObjectType type )
{
  passert_r( param < fparams.size(), "Script Error in '" + scriptname() +
                                         ": Less Parameter than expected. " +
                                         "You should use *.em-files shipped with this Core and "
                                         "recompile ALL of your Scripts _now_! RTFM" );

  BObjectImp* imp = fparams[param].get()->impptr();

  passert( imp != nullptr );

  if ( imp->isa( type ) )
  {
    return imp;
  }
  else
  {
    if ( !IS_DEBUGLOG_DISABLED )
    {
      std::string tmp = fmt::format( "Script Error in '{}' PC={}:\n", scriptname(), PC );
      if ( current_module_function )
        fmt::format_to( std::back_inserter( tmp ), "\tCall to function {}:\n",
                        current_module_function->name.get() );
      else
        tmp += "\tCall to an object method.\n";
      fmt::format_to( std::back_inserter( tmp ),
                      "\tParameter {}: Expected datatype {}, got datatype {}", param,
                      BObjectImp::typestr( type ), BObjectImp::typestr( imp->type() ) );
      DEBUGLOGLN( tmp );
    }
    return nullptr;
  }
}

BObjectImp* Executor::getParamImp2( unsigned param, BObjectImp::BObjectType type )
{
  passert_r( param < fparams.size(), "Script Error in '" + scriptname() +
                                         ": Less Parameter than expected. " +
                                         "You should use *.em-files shipped with this Core and "
                                         "recompile ALL of your Scripts _now_! RTFM" );

  BObjectImp* imp = fparams[param].get()->impptr();

  passert( imp != nullptr );

  if ( imp->isa( type ) )
  {
    return imp;
  }
  else
  {
    std::string report = "Invalid parameter type.  Expected param " + Clib::tostring( param ) +
                         " as " + BObjectImp::typestr( type ) + ", got " +
                         BObjectImp::typestr( imp->type() );
    func_result_ = new BError( report );
    return nullptr;
  }
}


const String* Executor::getStringParam( unsigned param )
{
  return Clib::explicit_cast<String*, BObjectImp*>( getParamImp( param, BObjectImp::OTString ) );
}

const BLong* Executor::getLongParam( unsigned param )
{
  return Clib::explicit_cast<BLong*, BObjectImp*>( getParamImp( param, BObjectImp::OTLong ) );
}

bool Executor::getStringParam( unsigned param, const String*& pstr )
{
  pstr = getStringParam( param );
  return ( pstr != nullptr );
}

bool Executor::getParam( unsigned param, int& value )
{
  BLong* plong =
      Clib::explicit_cast<BLong*, BObjectImp*>( getParamImp( param, BObjectImp::OTLong ) );
  if ( plong == nullptr )
    return false;

  value = plong->value();
  return true;
}

void Executor::setFunctionResult( BObjectImp* imp )
{
  func_result_ = imp;
}

void Executor::printStack( const std::string& message = "" )
{
  if ( debug_level < INSTRUCTIONS )
    return;

  if ( !message.empty() )
  {
    INFO_PRINTLN( message );
  }

  size_t i = 0;
  for ( auto riter = fparams.rbegin(); riter != fparams.rend(); ++riter )
  {
    auto* ptr = riter->get()->impptr();
    INFO_PRINTLN( "fparam[{} @ {}] {}", static_cast<void*>( ptr ), i, ptr->getStringRep() );
    i++;
  }

  i = 0;
  for ( auto riter = ValueStack.rbegin(); riter != ValueStack.rend(); ++riter )
  {
    auto* ptr = riter->get()->impptr();
    INFO_PRINTLN( "vstack[{} @ {}] {}", static_cast<void*>( ptr ), i, ptr->getStringRep() );
    i++;
  }

  INFO_PRINTLN( "---" );
}

bool Executor::getParam( unsigned param, int& value, int maxval )
{
  BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
  if ( imp )
  {
    BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    value = plong->value();
    if ( value >= 0 && value <= maxval )
    {
      return true;
    }
    else
    {
      std::string report = "Parameter " + Clib::tostring( param ) + " value " +
                           Clib::tostring( value ) + " out of expected range of [0.." +
                           Clib::tostring( maxval ) + "]";
      func_result_ = new BError( report );
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool Executor::getParam( unsigned param, int& value, int minval, int maxval )
{
  BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
  if ( imp )
  {
    BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    value = plong->value();
    if ( value >= minval && value <= maxval )
    {
      return true;
    }
    else
    {
      std::string report = "Parameter " + Clib::tostring( param ) + " value " +
                           Clib::tostring( value ) + " out of expected range of [" +
                           Clib::tostring( minval ) + ".." + Clib::tostring( maxval ) + "]";
      func_result_ = new BError( report );
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool Executor::getRealParam( unsigned param, double& value )
{
  BObjectImp* imp = getParamImp( param );
  if ( auto* d = impptrIf<Double>( imp ) )
  {
    value = d->value();
    return true;
  }
  else if ( auto* l = impptrIf<BLong>( imp ) )
  {
    value = l->value();
    return true;
  }
  else
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tCall to function {}:\n"
        "\tParameter {}: Expected Integer or Real, got datatype {}",
        scriptname(), PC, current_module_function->name.get(), param,
        BObjectImp::typestr( imp->type() ) );

    return false;
  }
}

bool Executor::getObjArrayParam( unsigned param, ObjArray*& pobjarr )
{
  pobjarr =
      Clib::explicit_cast<ObjArray*, BObjectImp*>( getParamImp( param, BObjectImp::OTArray ) );
  return ( pobjarr != nullptr );
}

void* Executor::getApplicPtrParam( unsigned param, const BApplicObjType* pointer_type )
{
  auto ap = static_cast<BApplicPtr*>( getParamImp( param, BObjectImp::OTApplicPtr ) );
  if ( ap == nullptr )
    return nullptr;

  if ( ap->pointer_type() == pointer_type )
  {
    return ap->ptr();
  }
  else
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tCall to function {}:\n"
        "\tParameter {}: Expected datatype, got datatype {}",
        scriptname(), PC, current_module_function->name.get(), param,
        BObjectImp::typestr( ap->type() ) );

    return nullptr;
  }
}

BApplicObjBase* Executor::getApplicObjParam( unsigned param, const BApplicObjType* object_type )
{
  auto aob = static_cast<BApplicObjBase*>( getParamImp( param, BObjectImp::OTApplicObj ) );
  if ( aob == nullptr )
    return nullptr;

  if ( aob->object_type() == object_type )
  {
    return aob;
  }
  else
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tCall to function {}:\n"
        "\tParameter {}: Expected datatype, got datatype {}",
        scriptname(), PC, current_module_function->name.get(), param, aob->getStringRep() );

    return nullptr;
  }
}

bool Executor::getParam( unsigned param, unsigned short& value, unsigned short maxval )
{
  BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
  if ( imp )
  {
    BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    int longvalue = plong->value();
    if ( longvalue >= 0 && longvalue <= maxval )
    {
      value = static_cast<unsigned short>( longvalue );
      return true;
    }
    else
    {
      std::string report = "Parameter " + Clib::tostring( param ) + " value " +
                           Clib::tostring( longvalue ) + " out of expected range of [0.." +
                           Clib::tostring( maxval ) + "]";
      func_result_ = new BError( report );
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool Executor::getParam( unsigned param, unsigned short& value, unsigned short minval,
                         unsigned short maxval )
{
  BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
  if ( imp )
  {
    BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    int longvalue = plong->value();
    if ( longvalue >= minval && longvalue <= maxval )
    {
      value = static_cast<unsigned short>( longvalue );
      return true;
    }
    else
    {
      std::string report = "Parameter " + Clib::tostring( param ) + " value " +
                           Clib::tostring( longvalue ) + " out of expected range of [" +
                           Clib::tostring( minval ) + ".." + Clib::tostring( maxval ) + "]";
      func_result_ = new BError( report );
      return false;
    }
  }
  else
  {
    return false;
  }
}
bool Executor::getParam( unsigned param, unsigned short& value )
{
  BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
  if ( imp )
  {
    BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    int longvalue = plong->value();
    if ( longvalue >= 0 && longvalue <= USHRT_MAX )
    {
      value = static_cast<unsigned short>( longvalue );
      return true;
    }
    else
    {
      std::string report = "Parameter " + Clib::tostring( param ) + " value " +
                           Clib::tostring( longvalue ) + " out of expected range of [0.." +
                           Clib::tostring( USHRT_MAX ) + "]";
      func_result_ = new BError( report );
      return false;
    }
  }
  else
  {
    return false;
  }
}
bool Executor::getParam( unsigned param, unsigned& value )
{
  BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
  if ( imp )
  {
    BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    int longvalue = plong->value();
    if ( longvalue >= 0 )  // && longvalue <= (int)INT_MAX )
    {
      value = static_cast<unsigned>( longvalue );
      return true;
    }
    else
    {
      std::string report = "Parameter " + Clib::tostring( param ) + " value " +
                           Clib::tostring( longvalue ) + " out of expected range of [0.." +
                           Clib::tostring( INT_MAX ) + "]";
      func_result_ = new BError( report );
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool Executor::getParam( unsigned param, short& value )
{
  BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
  if ( imp )
  {
    BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    int longvalue = plong->value();
    if ( longvalue >= (int)SHRT_MIN && longvalue <= (int)SHRT_MAX )
    {
      value = static_cast<short>( longvalue );
      return true;
    }
    else
    {
      std::string report = "Parameter " + Clib::tostring( param ) + " value " +
                           Clib::tostring( longvalue ) + " out of expected range of [" +
                           Clib::tostring( SHRT_MIN ) + ".." + Clib::tostring( SHRT_MAX ) + "]";
      func_result_ = new BError( report );
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool Executor::getParam( unsigned param, short& value, short maxval )
{
  BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
  if ( imp )
  {
    BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    int longvalue = plong->value();
    if ( longvalue >= (int)SHRT_MIN && longvalue <= maxval )
    {
      value = static_cast<short>( longvalue );
      return true;
    }
    else
    {
      std::string report = "Parameter " + Clib::tostring( param ) + " value " +
                           Clib::tostring( longvalue ) + " out of expected range of [" +
                           Clib::tostring( SHRT_MIN ) + ".." + Clib::tostring( maxval ) + "]";
      func_result_ = new BError( report );
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool Executor::getParam( unsigned param, short& value, short minval, short maxval )
{
  BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
  if ( imp )
  {
    BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    int longvalue = plong->value();
    if ( longvalue >= minval && longvalue <= maxval )
    {
      value = static_cast<short>( longvalue );
      return true;
    }
    else
    {
      std::string report = "Parameter " + Clib::tostring( param ) + " value " +
                           Clib::tostring( longvalue ) + " out of expected range of [" +
                           Clib::tostring( minval ) + ".." + Clib::tostring( maxval ) + "]";
      func_result_ = new BError( report );
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool Executor::getParam( unsigned param, signed char& value )
{
  BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
  if ( !imp )
    return false;

  BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

  int longvalue = plong->value();
  if ( longvalue >= std::numeric_limits<s8>::min() && longvalue <= std::numeric_limits<s8>::max() )
  {
    value = static_cast<signed char>( longvalue );
    return true;
  }
  else
  {
    std::string report = "Parameter " + Clib::tostring( param ) + " value " +
                         Clib::tostring( longvalue ) + " out of expected range of [" +
                         Clib::tostring( std::numeric_limits<s8>::max() ) + ".." +
                         Clib::tostring( std::numeric_limits<s8>::max() ) + "]";
    func_result_ = new BError( report );
    return false;
  }
}

bool Executor::getParam( unsigned param, bool& value )
{
  BObjectImp* imp = getParamImp( param );
  if ( auto* b = impptrIf<BBoolean>( imp ) )
  {
    value = b->value();
    return true;
  }
  else if ( auto* l = impptrIf<BLong>( imp ) )
  {
    value = l->isTrue();
    return true;
  }
  else
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tCall to function {}:\n"
        "\tParameter {}: Expected Boolean or Integer, got datatype {}",
        scriptname(), PC, current_module_function->name.get(), param,
        BObjectImp::typestr( imp->type() ) );

    return false;
  }
}

bool Executor::getUnicodeStringParam( unsigned param, const String*& pstr )
{
  BObject* obj = getParam( param );
  if ( !obj )
    return false;
  if ( auto* s = obj->impptr_if<String>() )
  {
    pstr = s;
    return true;
  }
  else if ( auto* a = obj->impptr_if<ObjArray>() )
  {
    String* str = String::fromUCArray( a );
    fparams[param].set( new BObject( str ) );  // store raw pointer
    pstr = str;
    return true;
  }
  std::string report = "Invalid parameter type.  Expected param " + Clib::tostring( param ) +
                       " as " + BObjectImp::typestr( BObjectImp::OTString ) + " or " +
                       BObjectImp::typestr( BObjectImp::OTArray ) + ", got " +
                       BObjectImp::typestr( obj->impptr()->type() );
  func_result_ = new BError( report );
  return false;
}

BObjectRef& Executor::LocalVar( unsigned int varnum )
{
  passert( Locals2 );
  passert( varnum < Locals2->size() );

  return ( *Locals2 )[varnum];
}

BObjectRef& Executor::GlobalVar( unsigned int varnum )
{
  passert( varnum < Globals2->size() );
  return ( *Globals2 )[varnum];
}

int Executor::getToken( Token& token, unsigned position )
{
  if ( position >= nLines )
    return -1;
  token = prog_->instr[position].token;
  return 0;
}


bool Executor::setProgram( EScriptProgram* i_prog )
{
  prog_.set( i_prog );
  prog_ok_ = false;
  seterror( true );
  if ( !viewmode_ )
  {
    if ( !AttachFunctionalityModules() )
      return false;
  }

  nLines = static_cast<unsigned int>( prog_->instr.size() );

  Globals2->clear();
  for ( unsigned i = 0; i < prog_->nglobals; ++i )
  {
    Globals2->push_back( BObjectRef() );
    Globals2->back().set( new BObject( UninitObject::create() ) );
  }

  prog_ok_ = true;
  seterror( false );
  ++prog_->invocations;
  return true;
}

BObjectRef Executor::getObjRef( void )
{
  if ( ValueStack.empty() )
  {
    POLLOG_ERRORLN( "Fatal error: Value Stack Empty! ({},PC={})", prog_->name, PC );
    seterror( true );
    return BObjectRef( UninitObject::create() );
  }

  BObjectRef ref = ValueStack.back();
  ValueStack.pop_back();
  return ref;
}


void Executor::execFunc( const Token& token )
{
  FunctionalityModule* fm = prog_->modules[token.module];
  ModuleFunction* modfunc = fm->functions[token.lval];
  current_module_function = modfunc;
  if ( modfunc->funcidx == -1 )
  {
    DEBUGLOGLN(
        "Error in script '{}':\n"
        "\tModule Function {} was not found.",
        prog_->name.get(), modfunc->name.get() );

    throw std::runtime_error( "No implementation for function found." );
  }

  ExecutorModule* em = execmodules[token.module];

  func_result_ = nullptr;
#ifdef ESCRIPT_PROFILE
  std::stringstream strm;
  strm << em->functionName( modfunc->funcidx );
  if ( !fparams.empty() )
    strm << " [" << fparams[0].get()->impptr()->typeOf() << "]";
  std::string name( strm.str() );
  unsigned long profile_start = GetTimeUs();
#endif
  BObjectImp* resimp = em->execFunc( modfunc->funcidx );
#ifdef ESCRIPT_PROFILE
  profile_escript( name, profile_start );
#endif

  if ( func_result_ )
  {
    if ( resimp )
    {
      BObject obj( resimp );
    }
    ValueStack.push_back( BObjectRef( new BObject( func_result_ ) ) );
    func_result_ = nullptr;
  }
  else if ( resimp )
  {
    ValueStack.push_back( BObjectRef( new BObject( resimp ) ) );
  }
  else
  {
    ValueStack.push_back( BObjectRef( new BObject( UninitObject::create() ) ) );
  }

  current_module_function = nullptr;
  return;
}

// RSV_LOCAL
void Executor::ins_makeLocal( const Instruction& /*ins*/ )
{
  passert( Locals2 != nullptr );

  Locals2->push_back( BObjectRef() );
  Locals2->back().set( new BObject( UninitObject::create() ) );

  ValueStack.push_back( BObjectRef( Locals2->back().get() ) );
}

// RSV_DECLARE_ARRAY
void Executor::ins_declareArray( const Instruction& /*ins*/ )
{
  BObjectRef objref = getObjRef();

  if ( !objref->isa( BObjectImp::OTUninit ) )
  {
    // FIXME: weak error message
    ERROR_PRINTLN( "variable is already initialized.." );
    seterror( true );
    return;
  }
  auto arr = new ObjArray;

  objref->setimp( arr );

  ValueStack.push_back( BObjectRef( objref ) );
}

void Executor::popParam( const Token& /*token*/ )
{
  BObjectRef objref = getObjRef();

  Locals2->push_back( BObjectRef() );
  Locals2->back().set( new BObject( objref->impptr()->copy() ) );
}

void Executor::popParamByRef( const Token& /*token*/ )
{
  BObjectRef objref = getObjRef();

  Locals2->push_back( BObjectRef( objref ) );
}

void Executor::getArg( const Token& /*token*/ )
{
  if ( ValueStack.empty() )
  {
    Locals2->push_back( BObjectRef() );
    Locals2->back().set( new BObject( UninitObject::create() ) );
  }
  else
  {
    BObjectRef objref = getObjRef();
    Locals2->push_back( BObjectRef() );
    Locals2->back().set( new BObject( objref->impptr()->copy() ) );
  }
}


BObjectRef Executor::addmember( BObject& left, const BObject& right )
{
  if ( !right.isa( BObjectImp::OTString ) )
  {
    return BObjectRef( left.clone() );
  }

  const String& varname = right.impref<const String>();

  return left.impref().operDotPlus( varname.data() );
}

BObjectRef Executor::removemember( BObject& left, const BObject& right )
{
  if ( !right.isa( BObjectImp::OTString ) )
  {
    return BObjectRef( left.clone() );
  }

  const String& varname = right.impref<const String>();

  return left.impref().operDotMinus( varname.data() );
}

BObjectRef Executor::checkmember( BObject& left, const BObject& right )
{
  if ( !right.isa( BObjectImp::OTString ) )
  {
    return BObjectRef( left.clone() );
  }

  const String& varname = right.impref<const String>();

  return left.impref().operDotQMark( varname.data() );
}


ContIterator::ContIterator() : BObjectImp( BObjectImp::OTUnknown ) {}
BObject* ContIterator::step()
{
  return nullptr;
}
BObjectImp* ContIterator::copy( void ) const
{
  return nullptr;
}
size_t ContIterator::sizeEstimate() const
{
  return sizeof( ContIterator );
}
std::string ContIterator::getStringRep() const
{
  return "<iterator>";
}

class ArrayIterator final : public ContIterator
{
public:
  ArrayIterator( ObjArray* pArr, BObject* pIterVal );
  virtual BObject* step() override;

private:
  size_t m_Index;
  BObject m_Array;
  ObjArray* m_pArray;
  BObjectRef m_IterVal;
  BLong* m_pIterVal;
};
ArrayIterator::ArrayIterator( ObjArray* pArr, BObject* pIterVal )
    : ContIterator(),
      m_Index( 0 ),
      m_Array( pArr ),
      m_pArray( pArr ),
      m_IterVal( pIterVal ),
      m_pIterVal( new BLong( 0 ) )
{
  m_IterVal.get()->setimp( m_pIterVal );
}
BObject* ArrayIterator::step()
{
  m_pIterVal->increment();
  if ( ++m_Index > m_pArray->ref_arr.size() )
    return nullptr;

  BObjectRef& objref = m_pArray->ref_arr[m_Index - 1];
  BObject* elem = objref.get();
  if ( elem == nullptr )
  {
    elem = new BObject( UninitObject::create() );
    objref.set( elem );
  }
  return elem;
}

ContIterator* BObjectImp::createIterator( BObject* /*pIterVal*/ )
{
  return new ContIterator();
}
ContIterator* ObjArray::createIterator( BObject* pIterVal )
{
  auto pItr = new ArrayIterator( this, pIterVal );
  return pItr;
}

/* Coming into initforeach, the expr to be iterated through is on the value stack.
   Initforeach must create three local variables:
   0. the iterator
   1. the expression
   2. the counter
   and remove the expression from the value stack.
   It then jumps to the STEPFOREACH instruction.
   */
void Executor::ins_initforeach( const Instruction& ins )
{
  Locals2->push_back( BObjectRef( UninitObject::create() ) );  // the iterator

  auto pIterVal = new BObject( UninitObject::create() );

  // this is almost like popParam, only we don't want a copy.
  BObjectRef objref = getObjRef();
  Locals2->push_back( BObjectRef() );
  ContIterator* pIter = objref->impptr()->createIterator( pIterVal );
  Locals2->back().set( new BObject( pIter ) );

  Locals2->push_back( BObjectRef() );
  Locals2->back().set( pIterVal );

  // Jump to to the corresponding `stepforeach` instruction, advancing the iterator.
  PC = ins.token.lval;
}

void Executor::ins_stepforeach( const Instruction& ins )
{
  size_t locsize = Locals2->size();
  ContIterator* pIter = ( *Locals2 )[locsize - 2]->impptr<ContIterator>();

  BObject* next = pIter->step();
  // If iterator has a value, set it on the locals stack and jump to the
  // corresponding instruction after `initforeach`.
  if ( next != nullptr )
  {
    ( *Locals2 )[locsize - 3].set( next );
    PC = ins.token.lval;
  }
}

/*
    Coming into the INITFOR, there will be two values on the value stack:
    START VALUE
    END VALUE

    If START VALUE > END VALUE, we skip the whole for loop.
    (the INITFOR's lval has the instr to jump to)
    */

void Executor::ins_initfor( const Instruction& ins )
{
  BObjectRef endref = getObjRef();
  BObjectRef startref = getObjRef();
  if ( *startref.get() > *endref.get() )
  {
    PC = ins.token.lval;
    return;
  }

  Locals2->push_back( BObjectRef( startref->clone() ) );  // the iterator
  Locals2->push_back( BObjectRef( endref->clone() ) );
}

void Executor::ins_nextfor( const Instruction& ins )
{
  size_t locsize = Locals2->size();
  BObjectImp* itr = ( *Locals2 )[locsize - 2]->impptr();
  BObjectImp* end = ( *Locals2 )[locsize - 1]->impptr();

  if ( auto* l = impptrIf<BLong>( itr ) )
    l->increment();
  else if ( auto* d = impptrIf<Double>( itr ) )
    d->increment();

  if ( *end >= *itr )
  {
    PC = ins.token.lval;
  }
}


int Executor::ins_casejmp_findlong( const Token& token, BLong* blong )
{
  const unsigned char* dataptr = token.dataptr;
  for ( ;; )
  {
    unsigned short offset;
    std::memcpy( &offset, dataptr, sizeof( unsigned short ) );
    dataptr += 2;
    unsigned char type = *dataptr;
    dataptr += 1;
    if ( type == CASE_TYPE_LONG )
    {
      int v = blong->value();
      if ( std::memcmp( &v, dataptr, sizeof( int ) ) == 0 )
        return offset;
      dataptr += 4;
    }
    else if ( type == CASE_TYPE_DEFAULT )
    {
      return offset;
    }
    else if ( type == CASE_TYPE_UNINIT )
    {
      /* nothing */
    }
    else if ( type == CASE_TYPE_BOOL )
    {
      dataptr += 1;
    }
    else if ( type == CASE_TYPE_STRING )
    {
      unsigned char len = *dataptr;
      dataptr += 1 + len;
    }
  }
}

int Executor::ins_casejmp_findbool( const Token& token, BBoolean* bbool )
{
  const unsigned char* dataptr = token.dataptr;
  for ( ;; )
  {
    unsigned short offset;
    std::memcpy( &offset, dataptr, sizeof( unsigned short ) );
    dataptr += 2;
    unsigned char type = *dataptr;
    dataptr += 1;
    if ( type == CASE_TYPE_LONG )
    {
      dataptr += 4;
    }
    else if ( type == CASE_TYPE_DEFAULT )
    {
      return offset;
    }
    else if ( type == CASE_TYPE_UNINIT )
    {
      /* nothing */
    }
    else if ( type == CASE_TYPE_BOOL )
    {
      bool value = static_cast<bool>( *dataptr );
      dataptr += 1;
      if ( value == bbool->value() )
      {
        return offset;
      }
    }
    else if ( type == CASE_TYPE_STRING )
    {
      unsigned char len = *dataptr;
      dataptr += 1 + len;
    }
  }
}

int Executor::ins_casejmp_finduninit( const Token& token )
{
  const unsigned char* dataptr = token.dataptr;
  for ( ;; )
  {
    unsigned short offset;
    std::memcpy( &offset, dataptr, sizeof( unsigned short ) );
    dataptr += 2;
    unsigned char type = *dataptr;
    dataptr += 1;
    if ( type == CASE_TYPE_LONG )
    {
      dataptr += 4;
    }
    else if ( type == CASE_TYPE_DEFAULT )
    {
      return offset;
    }
    else if ( type == CASE_TYPE_UNINIT )
    {
      return offset;
    }
    else if ( type == CASE_TYPE_BOOL )
    {
      dataptr += 1;
    }
    else if ( type == CASE_TYPE_STRING )
    {
      unsigned char len = *dataptr;
      dataptr += 1 + len;
    }
  }
}

int Executor::ins_casejmp_findstring( const Token& token, String* bstringimp )
{
  const std::string& bstring = bstringimp->value();
  const unsigned char* dataptr = token.dataptr;
  for ( ;; )
  {
    unsigned short offset;
    std::memcpy( &offset, dataptr, sizeof( unsigned short ) );
    dataptr += 2;
    unsigned char type = *dataptr;
    dataptr += 1;
    if ( type == CASE_TYPE_LONG )
    {
      dataptr += 4;
    }
    else if ( type == CASE_TYPE_DEFAULT )
    {
      return offset;
    }
    else if ( type == CASE_TYPE_BOOL )
    {
      dataptr += 1;
    }
    else if ( type == CASE_TYPE_UNINIT )
    {
      /* nothing */
    }
    else if ( type == CASE_TYPE_STRING )
    {
      unsigned char len = *dataptr;
      dataptr += 1;
      if ( bstring.size() == len && memcmp( bstring.data(), dataptr, len ) == 0 )
      {
        return offset;
      }
      dataptr += len;
    }
  }
}

int Executor::ins_casejmp_finddefault( const Token& token )
{
  const unsigned char* dataptr = token.dataptr;
  for ( ;; )
  {
    unsigned short offset;
    std::memcpy( &offset, dataptr, sizeof( unsigned short ) );
    dataptr += 2;
    unsigned char type = *dataptr;
    dataptr += 1;
    if ( type == CASE_TYPE_LONG )
    {
      dataptr += 4;
    }
    else if ( type == CASE_TYPE_DEFAULT )
    {
      return offset;
    }
    else if ( type == CASE_TYPE_UNINIT )
    {
      /* nothing */
    }
    else if ( type == CASE_TYPE_BOOL )
    {
      dataptr += 1;
    }
    else if ( type == CASE_TYPE_STRING )
    {
      unsigned char len = *dataptr;
      dataptr += 1 + len;
    }
  }
}

void Executor::ins_casejmp( const Instruction& ins )
{
  BObjectRef& objref = ValueStack.back();
  BObjectImp* objimp = objref->impptr();
  if ( auto* l = impptrIf<BLong>( objimp ) )
    PC = ins_casejmp_findlong( ins.token, l );
  else if ( auto* s = impptrIf<String>( objimp ) )
    PC = ins_casejmp_findstring( ins.token, s );
  else if ( auto* b = impptrIf<BBoolean>( objimp ) )
    PC = ins_casejmp_findbool( ins.token, b );
  else if ( impptrIf<UninitObject>( objimp ) )
    PC = ins_casejmp_finduninit( ins.token );
  else
    PC = ins_casejmp_finddefault( ins.token );
  ValueStack.pop_back();
}


void Executor::ins_jmpiftrue( const Instruction& ins )
{
  BObjectRef& objref = ValueStack.back();

  if ( objref->impptr()->isTrue() )
    PC = (unsigned)ins.token.lval;

  ValueStack.pop_back();
}

void Executor::ins_jmpiffalse( const Instruction& ins )
{
  BObjectRef& objref = ValueStack.back();

  if ( !objref->impptr()->isTrue() )
    PC = (unsigned)ins.token.lval;

  ValueStack.pop_back();
}

void Executor::ins_interpolate_string( const Instruction& ins )
{
  auto count = ins.token.lval;
  if ( count == 0 )
  {
    ValueStack.push_back( BObjectRef( new BObject( new String( "" ) ) ) );
  }
  else
  {
    size_t length = 0;

    std::vector<std::string> contents;
    contents.reserve( count );

    while ( count-- )
    {
      BObjectRef rightref = ValueStack.back();
      ValueStack.pop_back();
      auto str = rightref->impptr()->getStringRep();
      length += str.length();
      contents.push_back( std::move( str ) );
    }

    std::string joined;
    joined.reserve( length );

    while ( !contents.empty() )
    {
      joined += contents.back();
      contents.pop_back();
    }

    ValueStack.push_back( BObjectRef( new BObject( new String( joined ) ) ) );
  }
}

void Executor::ins_format_expression( const Instruction& )
{
  BObjectRef formatref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& exprref = ValueStack.back();
  BObject& expr = *exprref;

  auto format = formatref->impptr()->getFormattedStringRep();
  auto formatted = Bscript::get_formatted( expr.impptr(), format );

  exprref.set( new BObject( new String( formatted ) ) );
}

void Executor::ins_skipiftrue_else_consume( const Instruction& ins )
{
  // This is for short-circuit evaluation of the elvis operator [expr_a] ?: [expr_b]
  //
  // Program instructions look like this:
  //   [expr_a instructions] INS_SKIPIFTRUE_ELSE_CONSUME [expr_b instructions]
  //
  // The result value of expr_a is on the top of the value stack when this instruction executes.
  //
  // If [expr_a] evaluated to true, leave its result and skip over the expr_b instructions
  // otherwise, consume the false value and continue so that expr_b can replace it.
  //
  BObjectRef& objref = ValueStack.back();

  if ( objref->impptr()->isTrue() )
  {
    PC = PC + (unsigned)( ins.token.lval );
  }
  else
  {
    ValueStack.pop_back();
  }
}


// case TOK_LOCALVAR:
void Executor::ins_localvar( const Instruction& ins )
{
  ValueStack.push_back( ( *Locals2 )[ins.token.lval] );
}

// case RSV_GLOBAL:
// case TOK_GLOBALVAR:
void Executor::ins_globalvar( const Instruction& ins )
{
  ValueStack.push_back( ( *Globals2 )[ins.token.lval] );
}

// case TOK_LONG:
void Executor::ins_long( const Instruction& ins )
{
  ValueStack.push_back( BObjectRef( new BObject( new BLong( ins.token.lval ) ) ) );
}

// case TOK_BOOL:
void Executor::ins_bool( const Instruction& ins )
{
  ValueStack.push_back( BObjectRef( new BObject( new BBoolean( ins.token.lval ) ) ) );
}

// case TOK_CONSUMER:
void Executor::ins_consume( const Instruction& /*ins*/ )
{
  ValueStack.pop_back();
}

void Executor::ins_set_member( const Instruction& ins )
{
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  BObjectImp& rightimpref = right.impref();
  left.impref().set_member( ins.token.tokval(), &rightimpref,
                            !( right.count() == 1 && rightimpref.count() == 1 ) );
}

void Executor::ins_set_member_id( const Instruction& ins )
{
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  BObjectImp& rightimpref = right.impref();
  left.impref().set_member_id( ins.token.lval, &rightimpref,
                               !( right.count() == 1 && rightimpref.count() == 1 ) );
}

void Executor::ins_set_member_consume( const Instruction& ins )
{
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  BObjectImp& rightimpref = right.impref();
  left.impref().set_member( ins.token.tokval(), &rightimpref,
                            !( right.count() == 1 && rightimpref.count() == 1 ) );
  ValueStack.pop_back();
}

void Executor::ins_set_member_id_consume( const Instruction& ins )
{
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  BObjectImp& rightimpref = right.impref();

  left.impref().set_member_id( ins.token.lval, &rightimpref,
                               !( right.count() == 1 && rightimpref.count() == 1 ) );
  ValueStack.pop_back();
}

void Executor::ins_set_member_id_consume_plusequal( const Instruction& ins )
{
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  BObjectImp& leftimpref = left.impref();

  BObjectRef tmp = leftimpref.get_member_id( ins.token.lval );

  if ( !tmp->isa( BObjectImp::OTUninit ) &&
       !tmp->isa( BObjectImp::OTError ) )  // do nothing if curval is uninit or error
  {
    tmp->impref().operPlusEqual( *tmp, right.impref() );
    leftimpref.set_member_id( ins.token.lval, &tmp->impref(), false );
  }
  ValueStack.pop_back();
}

void Executor::ins_set_member_id_consume_minusequal( const Instruction& ins )
{
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  BObjectImp& leftimpref = left.impref();

  BObjectRef tmp = leftimpref.get_member_id( ins.token.lval );

  if ( !tmp->isa( BObjectImp::OTUninit ) &&
       !tmp->isa( BObjectImp::OTError ) )  // do nothing if curval is uninit or error
  {
    tmp->impref().operMinusEqual( *tmp, right.impref() );
    leftimpref.set_member_id( ins.token.lval, &tmp->impref(), false );
  }
  ValueStack.pop_back();
}

void Executor::ins_set_member_id_consume_timesequal( const Instruction& ins )
{
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  BObjectImp& leftimpref = left.impref();

  BObjectRef tmp = leftimpref.get_member_id( ins.token.lval );

  if ( !tmp->isa( BObjectImp::OTUninit ) &&
       !tmp->isa( BObjectImp::OTError ) )  // do nothing if curval is uninit or error
  {
    tmp->impref().operTimesEqual( *tmp, right.impref() );
    leftimpref.set_member_id( ins.token.lval, &tmp->impref(), false );
  }
  ValueStack.pop_back();
}

void Executor::ins_set_member_id_consume_divideequal( const Instruction& ins )
{
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  BObjectImp& leftimpref = left.impref();

  BObjectRef tmp = leftimpref.get_member_id( ins.token.lval );

  if ( !tmp->isa( BObjectImp::OTUninit ) &&
       !tmp->isa( BObjectImp::OTError ) )  // do nothing if curval is uninit or error
  {
    tmp->impref().operDivideEqual( *tmp, right.impref() );
    leftimpref.set_member_id( ins.token.lval, &tmp->impref(), false );
  }
  ValueStack.pop_back();
}

void Executor::ins_set_member_id_consume_modulusequal( const Instruction& ins )
{
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  BObjectImp& leftimpref = left.impref();

  BObjectRef tmp = leftimpref.get_member_id( ins.token.lval );

  if ( !tmp->isa( BObjectImp::OTUninit ) &&
       !tmp->isa( BObjectImp::OTError ) )  // do nothing if curval is uninit or error
  {
    tmp->impref().operModulusEqual( *tmp, right.impref() );
    leftimpref.set_member_id( ins.token.lval, &tmp->impref(), false );
  }
  ValueStack.pop_back();
}

void Executor::ins_get_member( const Instruction& ins )
{
  BObjectRef& leftref = ValueStack.back();

  BObject& left = *leftref;

#ifdef ESCRIPT_PROFILE
  std::stringstream strm;
  strm << "MBR_" << leftref->impptr()->typeOf() << " ." << ins.token.tokval();
  if ( !fparams.empty() )
    strm << " [" << fparams[0].get()->impptr()->typeOf() << "]";
  std::string name( strm.str() );
  unsigned long profile_start = GetTimeUs();
#endif
  leftref = left->get_member( ins.token.tokval() );
#ifdef ESCRIPT_PROFILE
  profile_escript( name, profile_start );
#endif
}

void Executor::ins_get_member_id( const Instruction& ins )
{
  BObjectRef& leftref = ValueStack.back();

  BObject& left = *leftref;

#ifdef ESCRIPT_PROFILE
  std::stringstream strm;
  strm << "MBR_" << leftref->impptr()->typeOf() << " ." << ins.token.lval;
  if ( !fparams.empty() )
    strm << " [" << fparams[0].get()->impptr()->typeOf() << "]";
  std::string name( strm.str() );
  unsigned long profile_start = GetTimeUs();
#endif
  leftref = left->get_member_id( ins.token.lval );
#ifdef ESCRIPT_PROFILE
  profile_escript( name, profile_start );
#endif
}

void Executor::ins_assign_localvar( const Instruction& ins )
{
  BObjectRef& lvar = ( *Locals2 )[ins.token.lval];

  BObjectRef& rightref = ValueStack.back();

  BObject& right = *rightref;

  BObjectImp& rightimpref = right.impref();

  if ( right.count() == 1 && rightimpref.count() == 1 )
  {
    lvar->setimp( &rightimpref );
  }
  else
  {
    lvar->setimp( rightimpref.copy() );
  }
  ValueStack.pop_back();
}
void Executor::ins_assign_globalvar( const Instruction& ins )
{
  BObjectRef& gvar = ( *Globals2 )[ins.token.lval];

  BObjectRef& rightref = ValueStack.back();

  BObject& right = *rightref;

  BObjectImp& rightimpref = right.impref();

  if ( right.count() == 1 && rightimpref.count() == 1 )
  {
    gvar->setimp( &rightimpref );
  }
  else
  {
    gvar->setimp( rightimpref.copy() );
  }
  ValueStack.pop_back();
}

// case INS_ASSIGN_CONSUME:
void Executor::ins_assign_consume( const Instruction& /*ins*/ )
{
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  BObjectImp& rightimpref = right.impref();

  if ( right.count() == 1 && rightimpref.count() == 1 )
  {
    left.setimp( &rightimpref );
  }
  else
  {
    left.setimp( rightimpref.copy() );
  }
  ValueStack.pop_back();
}

void Executor::ins_assign( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  BObjectImp& rightimpref = right.impref();

  if ( right.count() == 1 && rightimpref.count() == 1 )
  {
    left.setimp( &rightimpref );
  }
  else
  {
    left.setimp( rightimpref.copy() );
  }
}

void Executor::ins_array_assign( const Instruction& /*ins*/ )
{
  /*
      on the value stack:
      x[i] := y;
      (top)
      y
      i
      x
      upon exit:
      (x[i])
      */
  BObjectRef y_ref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef i_ref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& x_ref = ValueStack.back();

  BObject& y = *y_ref;
  BObject& i = *i_ref;
  BObject& x = *x_ref;

  BObjectImp* result;
  result = x->array_assign( i.impptr(), y.impptr(), ( y.count() != 1 ) );

  x_ref.set( new BObject( result ) );
}
void Executor::ins_array_assign_consume( const Instruction& /*ins*/ )
{
  /*
      on the value stack:
      x[i] := y;
      (top)
      y
      i
      x
      upon exit:
      (x[i])
      */
  BObjectRef y_ref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef i_ref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& x_ref = ValueStack.back();

  BObject& y = *y_ref;
  BObject& i = *i_ref;
  BObject& x = *x_ref;

  BObjectImp* result;
  result = x->array_assign( i.impptr(), y.impptr(), ( y.count() != 1 ) );

  BObject obj( result );
  ValueStack.pop_back();
}

// TOK_ADD:
void Executor::ins_add( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref.set( new BObject( right.impref().selfPlusObjImp( left.impref() ) ) );
}

// TOK_SUBTRACT
void Executor::ins_subtract( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref.set( new BObject( right.impref().selfMinusObjImp( left.impref() ) ) );
}

// TOK_MULT:
void Executor::ins_mult( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref.set( new BObject( right.impref().selfTimesObjImp( left.impref() ) ) );
}
// TOK_DIV:
void Executor::ins_div( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref.set( new BObject( right.impref().selfDividedByObjImp( left.impref() ) ) );
}
// TOK_MODULUS:
void Executor::ins_modulus( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref.set( new BObject( right.impref().selfModulusObjImp( left.impref() ) ) );
}

// TOK_IS:
void Executor::ins_is( const Instruction& /*ins*/ )
{
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref.set( new BObject( right.impref().selfIsObjImp( left.impref() ) ) );
}

// TOK_BSRIGHT:
void Executor::ins_bitshift_right( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref.set( new BObject( right.impref().selfBitShiftRightObjImp( left.impref() ) ) );
}
// TOK_BSLEFT:
void Executor::ins_bitshift_left( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref.set( new BObject( right.impref().selfBitShiftLeftObjImp( left.impref() ) ) );
}
// TOK_BITAND:
void Executor::ins_bitwise_and( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref.set( new BObject( right.impref().selfBitAndObjImp( left.impref() ) ) );
}
// TOK_BITXOR:
void Executor::ins_bitwise_xor( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref.set( new BObject( right.impref().selfBitXorObjImp( left.impref() ) ) );
}
// TOK_BITOR:
void Executor::ins_bitwise_or( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref.set( new BObject( right.impref().selfBitOrObjImp( left.impref() ) ) );
}

void Executor::ins_logical_and( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  int _true = ( left.isTrue() && right.isTrue() );
  leftref.set( new BObject( new BLong( _true ) ) );
}
void Executor::ins_logical_or( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  int _true = ( left.isTrue() || right.isTrue() );
  leftref.set( new BObject( new BLong( _true ) ) );
}

void Executor::ins_notequal( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  int _true = ( left != right );
  leftref.set( new BObject( new BLong( _true ) ) );
}

void Executor::ins_equal( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  int _true = ( left == right );
  leftref.set( new BObject( new BLong( _true ) ) );
}

void Executor::ins_lessthan( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  int _true = ( left < right );
  leftref.set( new BObject( new BLong( _true ) ) );
}

void Executor::ins_lessequal( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;
  int _true = ( left <= right );
  leftref.set( new BObject( new BLong( _true ) ) );
}
void Executor::ins_greaterthan( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  int _true = ( left > right );
  leftref.set( new BObject( new BLong( _true ) ) );
}
void Executor::ins_greaterequal( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  int _true = ( left >= right );
  leftref.set( new BObject( new BLong( _true ) ) );
}

// case TOK_ARRAY_SUBSCRIPT:
void Executor::ins_arraysubscript( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  leftref = ( *leftref )->OperSubscript( *rightref );
}

void Executor::ins_multisubscript( const Instruction& ins )
{
  // the subscripts are on the value stack in right-to-left order, followed by the array itself
  std::stack<BObjectRef> indices;
  for ( int i = 0; i < ins.token.lval; ++i )
  {
    indices.push( ValueStack.back() );
    ValueStack.pop_back();
  }

  BObjectRef& leftref = ValueStack.back();
  leftref = ( *leftref )->OperMultiSubscript( indices );
}

void Executor::ins_unpack_sequence( const Instruction& ins )
{
  bool rest = ins.token.lval >> 14;
  auto count = Clib::clamp_convert<u8>( ins.token.lval & 0x7F );

  BObjectRef refIter( new BObject( UninitObject::create() ) );

  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();

  // Reserve to keep the insert_at iterator valid
  ValueStack.reserve( ValueStack.size() + count );
  auto insert_at = ValueStack.begin() + ValueStack.size();
  auto pIter = std::unique_ptr<ContIterator>( rightref->impptr()->createIterator( refIter.get() ) );

  if ( rest )
  {
    auto rest_index = Clib::clamp_convert<u8>( ( ins.token.lval & 0x3FFF ) >> 7 );

    // Add all elements up to the rest index
    for ( u8 i = 0; i < rest_index; ++i )
    {
      if ( auto res = pIter->step() )
        ValueStack.emplace( insert_at, res );
      else
        ValueStack.emplace( insert_at, new BError( "Index out of bounds" ) );
    }

    // Add the rest array, and fill it with the remaining iterator elements
    auto rest_array = ValueStack.emplace( insert_at, new ObjArray )->get()->impptr<ObjArray>();

    while ( auto res = pIter->step() )
      rest_array->addElement( res->impptr() );

    // Take the remaining elements from the rest array to fill the rest of the bindings.
    auto left = count - rest_index - 1;

    for ( u8 i = 0; i < left; ++i )
    {
      if ( rest_array->ref_arr.empty() )
        ValueStack.emplace( insert_at + i, new BError( "Index out of bounds" ) );
      else
      {
        ValueStack.insert( insert_at + i, rest_array->ref_arr.back() );

        rest_array->ref_arr.pop_back();
      }
    }
  }
  else
  {
    for ( u8 i = 0; i < count; ++i )
    {
      if ( auto res = pIter->step() )
        ValueStack.emplace( insert_at, res );
      else
        ValueStack.emplace( insert_at, new BError( "Index out of bounds" ) );
    }
  }
}

void Executor::ins_unpack_indices( const Instruction& ins )
{
  bool rest = ins.token.lval >> 14;
  auto binding_count = Clib::clamp_convert<u8>( ins.token.lval & 0x7F );
  // If there is a rest binding, there will be one less index than the binding
  // count, as the rest binding has no corresponding element index access.
  auto index_count = rest ? Clib::clamp_convert<u8>( binding_count - 1 ) : binding_count;

  // Ensure there is a ValueStack entry for each index, + 1 for the unpacking object ('rightref').
  passert_r( ValueStack.size() >= index_count + 1, "Not enough values to unpack" );

  if ( rest )
  {
    // Use a multi_index because we need to (1) iterate over the indexes in
    // order of the bindings in the script, and (2) keep track of which indexes
    // have been used.
    using namespace boost::multi_index;
    using OrderedSet =
        multi_index_container<BObject,
                              indexed_by<sequenced<>,  // Maintains insertion order
                                         ordered_unique<identity<BObject>>  // Ensures uniqueness
                                         >>;

    OrderedSet indexes;

    for ( u8 i = 0; i < index_count; ++i )
    {
      indexes.insert( indexes.begin(), BObject( *ValueStack.back().get() ) );
      ValueStack.pop_back();
    }

    BObjectRef rightref = ValueStack.back();
    ValueStack.pop_back();

    // Reserve to keep the insert_at iterator valid
    ValueStack.reserve( ValueStack.size() + binding_count );
    auto insert_at = ValueStack.end();

    for ( const auto& index : indexes )
    {
      ValueStack.insert( insert_at, rightref->impptr()->OperSubscript( index ) );
    }

    // Rest object is always last element (validated by semantic analyzer), so
    // no need to calculate `rest_index`.
    std::unique_ptr<BObjectImp> rest_obj;

    if ( rightref->isa( BObjectImp::OTStruct ) )
      rest_obj = std::make_unique<BStruct>();
    else if ( rightref->isa( BObjectImp::OTDictionary ) )
      rest_obj = std::make_unique<BDictionary>();
    else
    {
      ValueStack.emplace( insert_at, new BError( "Invalid type for rest binding" ) );
      return;
    }

    BObjectRef refIter( new BObject( UninitObject::create() ) );
    auto pIter =
        std::unique_ptr<ContIterator>( rightref->impptr()->createIterator( refIter.get() ) );

    auto& unique_index = indexes.get<1>();

    while ( auto res = pIter->step() )
    {
      auto itr = unique_index.find( *refIter.get() );

      if ( itr == unique_index.end() )
        rest_obj->array_assign( refIter->impptr(), res->impptr(), true );
    }
    ValueStack.emplace( insert_at, rest_obj.release() );
  }
  else
  {
    // If not using a rest binding, only keep track of binding order.
    std::list<BObject> indexes;

    for ( u8 i = 0; i < index_count; ++i )
    {
      indexes.insert( indexes.begin(), BObject( *ValueStack.back().get() ) );
      ValueStack.pop_back();
    }

    BObjectRef rightref = ValueStack.back();
    ValueStack.pop_back();

    // Reserve to keep the insert_at iterator valid
    ValueStack.reserve( ValueStack.size() + binding_count );
    auto insert_at = ValueStack.end();

    for ( const auto& index : indexes )
    {
      ValueStack.insert( insert_at, rightref->impptr()->OperSubscript( index ) );
    }
  }
}

void Executor::ins_take_local( const Instruction& )
{
  passert( Locals2 != nullptr );
  passert( !ValueStack.empty() );

  // There is no entry in the locals vector, so create a new one.
  Locals2->push_back( BObjectRef( UninitObject::create() ) );
  BObjectRef& lvar = ( *Locals2 ).back();

  BObjectRef& rightref = ValueStack.back();

  BObject& right = *rightref;

  BObjectImp& rightimpref = right.impref();

  if ( right.count() == 1 && rightimpref.count() == 1 )
  {
    lvar->setimp( &rightimpref );
  }
  else
  {
    lvar->setimp( rightimpref.copy() );
  }
  ValueStack.pop_back();
}

void Executor::ins_take_global( const Instruction& ins )
{
  passert( !ValueStack.empty() );

  // Globals already have an entry in the globals vector, so just index into it.
  BObjectRef& gvar = ( *Globals2 )[ins.token.lval];

  BObjectRef& rightref = ValueStack.back();

  BObject& right = *rightref;

  BObjectImp& rightimpref = right.impref();

  if ( right.count() == 1 && rightimpref.count() == 1 )
  {
    gvar->setimp( &rightimpref );
  }
  else
  {
    gvar->setimp( rightimpref.copy() );
  }
  ValueStack.pop_back();
}

void Executor::ins_multisubscript_assign( const Instruction& ins )
{
  BObjectRef target_ref = ValueStack.back();
  ValueStack.pop_back();
  // the subscripts are on the value stack in right-to-left order, followed by the array itself
  std::stack<BObjectRef> indices;
  for ( int i = 0; i < ins.token.lval; ++i )
  {
    indices.push( ValueStack.back() );
    ValueStack.pop_back();
  }

  BObjectRef& leftref = ValueStack.back();
  leftref = ( *leftref )->OperMultiSubscriptAssign( indices, target_ref->impptr() );
}

void Executor::ins_addmember( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref = addmember( left, right );
}

void Executor::ins_removemember( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref = removemember( left, right );
}

void Executor::ins_checkmember( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref = checkmember( left, right );
}

void Executor::ins_addmember2( const Instruction& ins )
{
  BObjectRef obref = ValueStack.back();

  BObject& ob = *obref;

  ob.impref().operDotPlus( ins.token.tokval() );
}

void Executor::ins_addmember_assign( const Instruction& ins )
{
  BObjectRef valref = ValueStack.back();
  BObject& valob = *valref;
  BObjectImp* valimp = valref->impptr();

  ValueStack.pop_back();

  BObjectRef obref = ValueStack.back();
  BObject& ob = *obref;

  BObjectRef memref = ob.impref().operDotPlus( ins.token.tokval() );
  BObject& mem = *memref;

  if ( valob.count() == 1 && valimp->count() == 1 )
  {
    mem.setimp( valimp );
  }
  else
  {
    mem.setimp( valimp->copy() );
  }
  // the struct is at the top of the stack
}

void Executor::ins_dictionary_addmember( const Instruction& /*ins*/ )
{
  /*
      ENTRANCE: value stack:
      dictionary
      key
      value
      EXIT: value stack:
      dictionary
      FUNCTION:
      adds the (key, value) pair to the dictionary
      */

  BObjectRef valref = ValueStack.back();
  ValueStack.pop_back();
  BObject& valob = *valref;
  BObjectImp* valimp = valob.impptr();

  BObjectRef keyref = ValueStack.back();
  ValueStack.pop_back();
  BObject& keyob = *keyref;
  BObjectImp* keyimp = keyob.impptr();

  BObjectRef dictref = ValueStack.back();
  BObject& dictob = *dictref;
  BDictionary* dict = dictob.impptr<BDictionary>();

  if ( keyob.count() != 1 || keyimp->count() != 1 )
  {
    keyimp = keyimp->copy();
  }
  if ( valob.count() != 1 || valimp->count() != 1 )
  {
    valimp = valimp->copy();
  }

  dict->addMember( keyimp, valimp );
  // the dictionary remains at the top of the stack.
}

void Executor::ins_in( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  leftref.set( new BObject( new BLong( right.impref().contains( left.impref() ) ) ) );
}

void Executor::ins_insert_into( const Instruction& /*ins*/ )
{
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  if ( auto* spread = right.impptr_if<BSpread>() )
  {
    BObjectRef refIter( new BObject( UninitObject::create() ) );

    auto pIter =
        std::unique_ptr<ContIterator>( spread->object->impptr()->createIterator( refIter.get() ) );

    BObject* next = pIter->step();
    while ( next != nullptr )
    {
      left.impref().operInsertInto( left, next->impref() );
      next = pIter->step();
    }
  }
  else
  {
    left.impref().operInsertInto( left, right.impref() );
  }
}

void Executor::ins_plusequal( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  left.impref().operPlusEqual( left, right.impref() );
}

void Executor::ins_minusequal( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  left.impref().operMinusEqual( left, right.impref() );
}

void Executor::ins_timesequal( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  left.impref().operTimesEqual( left, right.impref() );
}

void Executor::ins_divideequal( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  left.impref().operDivideEqual( left, right.impref() );
}

void Executor::ins_modulusequal( const Instruction& /*ins*/ )
{
  /*
      These each take two operands, and replace them with one.
      We'll leave the second one on the value stack, and
      just replace its object with the result
      */
  BObjectRef rightref = ValueStack.back();
  ValueStack.pop_back();
  BObjectRef& leftref = ValueStack.back();

  BObject& right = *rightref;
  BObject& left = *leftref;

  left.impref().operModulusEqual( left, right.impref() );
}

// case RSV_GOTO:
void Executor::ins_goto( const Instruction& ins )
{
  PC = (unsigned)ins.token.lval;
}

// TOK_FUNC:
void Executor::ins_func( const Instruction& ins )
{
  unsigned nparams = prog_->modules[ins.token.module]->functions[ins.token.lval]->nargs;
  getParams( nparams );
  execFunc( ins.token );
  cleanParams();
  return;
}

void Executor::ins_call_method_id( const Instruction& ins )
{
  BContinuation* continuation = nullptr;
  unsigned nparams = ins.token.type;

  do
  {
    getParams( nparams );
    if ( auto* funcr = ValueStack.back()->impptr_if<BFunctionRef>() )
    {
      Instruction jmp;
      bool add_new_classinst = ins.token.lval == MTH_NEW;

      if ( add_new_classinst )
      {
        if ( funcr->constructor() )
        {
          fparams.insert( fparams.begin(),
                          BObjectRef( new BConstObject( new BClassInstanceRef(
                              new BClassInstance( prog_, funcr->class_index(), Globals2 ) ) ) ) );
        }
      }

      if ( funcr->validCall( continuation ? MTH_CALL : ins.token.lval, *this, &jmp ) )
      {
        BObjectRef funcobj( ValueStack.back() );  // valuestack gets modified, protect BFunctionRef
        call_function_reference( funcr, continuation, jmp );
        return;
      }
    }
    // If there _was_ a continuation to be handled (from previous loop
    // iteration), there must have been a FuncRef on the stack. Otherwise,
    // `continuation` may leak.
    passert_always( continuation == nullptr );

    size_t stacksize = ValueStack.size();  // ValueStack can grow
#ifdef ESCRIPT_PROFILE
    std::stringstream strm;
    strm << "MTHID_" << ValueStack.back()->impptr()->typeOf() << " ." << ins.token.lval;
    if ( !fparams.empty() )
      strm << " [" << fparams[0].get()->impptr()->typeOf() << "]";
    std::string name( strm.str() );
    unsigned long profile_start = GetTimeUs();
#endif
    BObjectImp* imp = ValueStack.back()->impptr()->call_method_id( ins.token.lval, *this );

    if ( auto* cont = impptrIf<BContinuation>( imp ) )
    {
      continuation = cont;
      // Set nparams, so the next loop iteration's `getParams` will know how many arguments to
      // move.
      nparams = static_cast<unsigned int>( continuation->args.size() );

      // Add function reference to stack
      ValueStack.push_back( BObjectRef( continuation->func() ) );

      // Move all arguments to the value stack
      ValueStack.insert( ValueStack.end(), std::make_move_iterator( continuation->args.begin() ),
                         std::make_move_iterator( continuation->args.end() ) );

      continuation->args.clear();

      cleanParams();

      printStack( fmt::format(
          "call_method_id continuation arguments added to ValueStack, prior to getParams({}) and "
          "funcref.call()",
          nparams ) );

      // Next on the stack is a `FuncRef` that we need to call. We will continue the loop and handle
      // it.

      // Prior to handling the `FuncRef` in the next loop, it will move from ValueStack to fparam.
      // Then, having a `continuation` set while processing the `FuncRef`, will create the proper
      // jumps.

      continue;
    }

#ifdef ESCRIPT_PROFILE
    profile_escript( name, profile_start );
#endif
    BObjectRef& objref = ValueStack[stacksize - 1];
    if ( func_result_ )
    {
      if ( imp )
      {
        BObject obj( imp );
      }

      objref.set( new BObject( func_result_ ) );
      func_result_ = nullptr;
    }
    else if ( imp )
    {
      objref.set( new BObject( imp ) );
    }
    else
    {
      objref.set( new BObject( UninitObject::create() ) );
    }

    cleanParams();
    return;
  }
  // This condition should only ever evaluate to `true` once. In the second loop
  // iteration, handling the FuncRef will return out of this method.
  while ( continuation != nullptr );
}

void Executor::ins_call_method( const Instruction& ins )
{
  unsigned nparams = ins.token.lval;
  auto method_name = ins.token.tokval();

  getParams( nparams );
  BObjectImp* callee = ValueStack.back()->impptr();

  if ( auto* classinstref = ValueStack.back()->impptr_if<BClassInstanceRef>() )
  {
    BFunctionRef* funcr = nullptr;
    auto classinst = classinstref->instance();

    // Prefer members over class methods by checking contents first.
    auto member_itr = classinst->contents().find( method_name );

    if ( member_itr != classinst->contents().end() )
    {
      // If the member exists and is NOT a function reference, we will still try
      // to "call" it. This is _intentional_, and will result in a runtime
      // BError. This is similar to `var foo := 3; print(foo.bar());`, resulting
      // in a "Method 'bar' not found" error.
      callee = member_itr->second.get()->impptr();

      funcr = member_itr->second.get()->impptr_if<BFunctionRef>();
    }
    else
    {
      // Have we already looked up this method?
      ClassMethodKey key{ prog_, classinst->index(), method_name };
      auto cache_itr = class_methods.find( key );
      if ( cache_itr != class_methods.end() )
      {
        // Switch the callee to the function reference: if the
        // funcr->validCall fails, we will go into the funcref
        // ins_call_method, giving the error about invalid parameter counts.
        funcr = cache_itr->second->impptr_if<BFunctionRef>();
        callee = funcr;
        method_name = getObjMethod( MTH_CALL_METHOD )->code;
      }
      else
      {
        // Does the class define this method?
        funcr = classinst->makeMethod( method_name );

        if ( funcr != nullptr )
        {
          // Cache the method for future lookups
          class_methods[key] = BObjectRef( funcr );

          // Switch the callee to the function reference.
          callee = funcr;
          method_name = getObjMethod( MTH_CALL_METHOD )->code;
        }
      }
    }

    if ( funcr != nullptr )
    {
      Instruction jmp;
      int id;

      // Add `this` to the front of the argument list only for class methods,
      // skipping eg. an instance member function reference set via
      // `this.foo := @(){};`.
      if ( funcr->class_method() )
      {
        id = MTH_CALL_METHOD;
        fparams.insert( fparams.begin(), ValueStack.back() );
      }
      else
      {
        id = MTH_CALL;
      }

      if ( funcr->validCall( id, *this, &jmp ) )
      {
        BObjectRef funcobj( funcr );  // valuestack gets modified, protect BFunctionRef
        call_function_reference( funcr, nullptr, jmp );
        return;
      }
    }
  }
  else if ( auto* funcr = ValueStack.back()->impptr_if<BFunctionRef>() )
  {
    Instruction jmp;
    if ( funcr->validCall( method_name, *this, &jmp ) )
    {
      BObjectRef funcobj( ValueStack.back() );  // valuestack gets modified, protect BFunctionRef
      call_function_reference( funcr, nullptr, jmp );
      return;
    }
  }

  size_t stacksize = ValueStack.size();  // ValueStack can grow
#ifdef ESCRIPT_PROFILE
  std::stringstream strm;
  strm << "MTH_" << callee->typeOf() << " ." << method_name;
  if ( !fparams.empty() )
    strm << " [" << fparams[0].get()->impptr()->typeOf() << "]";
  std::string name( strm.str() );
  unsigned long profile_start = GetTimeUs();
#endif
#ifdef BOBJECTIMP_DEBUG
  BObjectImp* imp;

  if ( strcmp( method_name, "impptr" ) == 0 )
    imp = new String( fmt::format( "{}", static_cast<void*>( callee ) ) );
  else
    imp = callee->call_method( method_name, *this );
#else
  BObjectImp* imp = callee->call_method( method_name, *this );
#endif
#ifdef ESCRIPT_PROFILE
  profile_escript( name, profile_start );
#endif

  BObjectRef& objref = ValueStack[stacksize - 1];
  if ( func_result_ )
  {
    if ( imp )
    {
      BObject obj( imp );
    }

    objref.set( new BObject( func_result_ ) );
    func_result_ = nullptr;
  }
  else if ( imp )
  {
    objref.set( new BObject( imp ) );
  }
  else
  {
    objref.set( new BObject( UninitObject::create() ) );
  }

  cleanParams();
  return;
}

// CTRL_STATEMENTBEGIN:
void Executor::ins_statementbegin( const Instruction& ins )
{
  if ( debug_level >= SOURCELINES && ins.token.tokval() )
    INFO_PRINTLN( ins.token.tokval() );
}

// case CTRL_PROGEND:
void Executor::ins_progend( const Instruction& /*ins*/ )
{
  done = 1;
  run_ok_ = false;
  PC = 0;
}


// case CTRL_MAKELOCAL:
void Executor::ins_makelocal( const Instruction& /*ins*/ )
{
  if ( Locals2 )
    upperLocals2.push_back( Locals2 );
  Locals2 = new BObjectRefVec;
}

void Executor::ins_check_mro( const Instruction& ins )
{
  auto classinst_offset = ins.token.lval;

  if ( classinst_offset > static_cast<int>( ValueStack.size() ) || ValueStack.empty() )
  {
    POLLOG_ERRORLN( "Fatal error: Check MRO offset error! offset={}, ValueStack.size={} ({},PC={})",
                    classinst_offset, ValueStack.size(), prog_->name, PC );
    seterror( true );
    return;
  }

  const auto& classinst_ref = ValueStack.at( ValueStack.size() - classinst_offset - 1 );

  if ( nLines < PC + 1 )
  {
    POLLOG_ERRORLN( "Fatal error: Check MRO instruction out of bounds! nLines={} ({},PC={})",
                    nLines, prog_->name, PC );
    seterror( true );
    return;
  }

  const Instruction& jsr_ins = prog_->instr.at( PC + 1 );
  if ( jsr_ins.func != &Executor::ins_jsr_userfunc )
  {
    POLLOG_ERRORLN( "Fatal error: Check MRO instruction not followed by JSR_USERFUNC! ({},PC={})",
                    prog_->name, PC );
    seterror( true );
    return;
  }

  auto ctor_addr = jsr_ins.token.lval;

  auto classinstref = classinst_ref->impptr_if<BClassInstanceRef>();

  if ( classinstref != nullptr && classinstref->instance()->constructors_called.find( ctor_addr ) ==
                                      classinstref->instance()->constructors_called.end() )
  {
    classinstref->instance()->constructors_called.insert( ctor_addr );
  }
  else
  {
    // Constructor has been called, or `this` is not a class instance: clear
    // arguments and skip jump instructions (makelocal, jsr_userfunc)
    ValueStack.resize( ValueStack.size() - ins.token.lval );
    PC += 2;
  }
}

// CTRL_JSR_USERFUNC:
void Executor::ins_jsr_userfunc( const Instruction& ins )
{
  jump( ins.token.lval, nullptr, nullptr );
}

void Executor::jump( int target_PC, BContinuation* continuation, BFunctionRef* funcref )
{
  ReturnContext rc;
  rc.PC = PC;
  rc.ValueStackDepth = static_cast<unsigned int>( ValueStack.size() );
  if ( continuation )
  {
    rc.Continuation.set( new BObject( continuation ) );
  }

  // Only store our global context if the function is external to the current program.
  if ( funcref != nullptr && funcref->prog() != prog_ )
  {
    // Store external context for the return path.
    rc.ExternalContext = ReturnContext::External( prog_, std::move( execmodules ), Globals2 );

    // Set the prog and globals to the external function's, updating nLines and
    // execmodules.
    prog_ = funcref->prog();

    Globals2 = funcref->globals;

    nLines = static_cast<unsigned int>( prog_->instr.size() );

    // Re-attach modules, as the external user function's module function call
    // instructions refer to modules by index.
    execmodules.clear();

    if ( !viewmode_ )
    {
      if ( !AttachFunctionalityModules() )
      {
        POLLOGLN( "Could not attach modules for external function call jump" );
        seterror( true );
      }
    }
  }

  ControlStack.push_back( rc );

  PC = target_PC;
  if ( ControlStack.size() >= escript_config.max_call_depth )
  {
    std::string tmp = fmt::format(
        "Script {} exceeded maximum call depth\n"
        "Return path PCs: ",
        scriptname() );
    while ( !ControlStack.empty() )
    {
      rc = ControlStack.back();
      ControlStack.pop_back();
      fmt::format_to( std::back_inserter( tmp ), "{} ", rc.PC );
    }
    POLLOGLN( tmp );
    seterror( true );
  }
}


BObjectImp* Executor::get_stacktrace( bool as_array )
{
  bool has_symbols = prog_->read_dbg_file( true ) == 0;

  auto with_dbginfo =
      [&]( const std::function<void( unsigned int /*pc*/, const std::string& /*file*/,
                                     unsigned int /*line*/, const std::string& /*functionName*/ )>&
               handler )
  {
    walkCallStack(
        [&]( unsigned int pc )
        {
          auto filename = prog()->dbg_filenames[prog()->dbg_filenum[pc]];
          auto line = prog()->dbg_linenum[pc];
          auto dbgFunction =
              std::find_if( prog()->dbg_functions.begin(), prog()->dbg_functions.end(),
                            [&]( auto& i ) { return i.firstPC <= pc && pc <= i.lastPC; } );

          std::string functionName =
              dbgFunction != prog()->dbg_functions.end() ? dbgFunction->name : "<program>";

          handler( pc, filename, line, functionName );
        } );
  };

  if ( as_array )
  {
    std::unique_ptr<ObjArray> result( new ObjArray );

    if ( has_symbols )
    {
      with_dbginfo(
          [&]( unsigned int pc, const std::string& filename, unsigned int line,
               const std::string& functionName )
          {
            std::unique_ptr<BStruct> entry( new BStruct );
            entry->addMember( "file", new String( filename ) );
            entry->addMember( "line", new BLong( line ) );
            entry->addMember( "name", new String( functionName ) );
            entry->addMember( "pc", new BLong( pc ) );
            result->addElement( entry.release() );
          } );
    }
    else
    {
      walkCallStack(
          [&]( unsigned int pc )
          {
            std::unique_ptr<BStruct> entry( new BStruct );
            entry->addMember( "file", new String( scriptname() ) );
            entry->addMember( "pc", new BLong( pc ) );
            result->addElement( entry.release() );
          } );
    }

    return result.release();
  }
  else  // as string
  {
    std::string result;

    if ( has_symbols )
    {
      with_dbginfo(
          [&]( unsigned int /*pc*/, const std::string& filename, unsigned int line,
               const std::string& functionName )
          {
            result.append( fmt::format( "{}at {} ({}:{})", result.empty() ? "" : "\n", functionName,
                                        filename, line ) );
          } );
    }
    else
    {
      walkCallStack(
          [&]( unsigned int pc ) {
            result.append(
                fmt::format( "{}at {}+{}", result.empty() ? "" : "\n", scriptname(), pc ) );
          } );
    }

    return new String( std::move( result ) );
  }
}

void Executor::ins_pop_param( const Instruction& ins )
{
  popParam( ins.token );
}

void Executor::ins_pop_param_byref( const Instruction& ins )
{
  popParamByRef( ins.token );
}

void Executor::ins_get_arg( const Instruction& ins )
{
  getArg( ins.token );
}

// CTRL_LEAVE_BLOCK:
void Executor::ins_leave_block( const Instruction& ins )
{
  if ( Locals2 )
  {
    for ( int i = 0; i < ins.token.lval; i++ )
      Locals2->pop_back();
  }
  else  // at global level.  ick.
  {
    for ( int i = 0; i < ins.token.lval; i++ )
      Globals2->pop_back();
  }
}

void Executor::ins_gosub( const Instruction& ins )
{
  ReturnContext rc;
  rc.PC = PC;
  rc.ValueStackDepth = static_cast<unsigned int>( ValueStack.size() );
  ControlStack.push_back( rc );
  if ( Locals2 )
    upperLocals2.push_back( Locals2 );
  Locals2 = new BObjectRefVec;

  PC = (unsigned)ins.token.lval;
}

// case RSV_RETURN
void Executor::ins_return( const Instruction& /*ins*/ )
{
  if ( ControlStack.empty() )
  {
    ERROR_PRINTLN( "Return without GOSUB! (PC={}, {})", PC, scriptname() );

    seterror( true );
    return;
  }
  BObjectRef continuation;
  ReturnContext& rc = ControlStack.back();
  PC = rc.PC;

  if ( rc.Continuation.get() != nullptr )
  {
    continuation = rc.Continuation;
  }

  if ( Locals2 )
  {
    delete Locals2;
    Locals2 = nullptr;
  }
  if ( !upperLocals2.empty() )
  {
    Locals2 = upperLocals2.back();
    upperLocals2.pop_back();
  }

  if ( rc.ExternalContext.has_value() )
  {
    prog_ = std::move( rc.ExternalContext->Program );
    nLines = static_cast<unsigned int>( prog_->instr.size() );
    execmodules = std::move( rc.ExternalContext->Modules );
    Globals2 = std::move( rc.ExternalContext->Globals );
  }

  // FIXME do something with rc.ValueStackDepth
  ControlStack.pop_back();

  if ( continuation != nullptr )
  {
    auto result = ValueStack.back();
    ValueStack.pop_back();

    // Do not move the `result` object, as the continuation callback may return
    // the result's BObjectImp*. If we move `result`, the BObjectImp* will be
    // deleted when the callback ends.
    auto* imp = continuation->impptr<BContinuation>()->continueWith( *this, result );

    // If the the continuation callback returned a continuation, handle the jump.
    if ( auto* cont = impptrIf<BContinuation>( imp ) )
    {
      // Do not delete imp, as the ReturnContext created in `ins_jsr_userfunc`
      // takes ownership.

      // Add function reference to stack
      ValueStack.push_back( BObjectRef( new BObject( cont->func() ) ) );

      // Move all arguments to the fparams stack
      fparams.insert( fparams.end(), std::make_move_iterator( cont->args.begin() ),
                      std::make_move_iterator( cont->args.end() ) );

      cont->args.clear();

      printStack(
          "continuation callback returned a continuation; continuation args added to fparams" );

      BObjectRef objref = ValueStack.back();
      auto funcr = objref->impptr<BFunctionRef>();
      Instruction jmp;
      if ( funcr->validCall( MTH_CALL, *this, &jmp ) )
      {
        call_function_reference( funcr, cont, jmp );
      }
      else
      {
        // Delete `imp` since a ReturnContext was not created.
        BObject bobj( imp );
      }
    }
    else
    {
      // Remove the original `this` receiver from the stack, eg. remove `array{}` from
      // `array{}.filter(...)`
      ValueStack.pop_back();

      // Add the result to the stack.
      ValueStack.push_back( BObjectRef( new BObject( imp ) ) );
    }
    printStack( fmt::format( "Continuation end of ins_return, jumping to PC={}", PC ) );
  }
}

void Executor::ins_exit( const Instruction& /*ins*/ )
{
  done = 1;
  run_ok_ = false;
}

void Executor::ins_double( const Instruction& ins )
{
  ValueStack.push_back( BObjectRef( new BObject( new Double( ins.token.dval ) ) ) );
}

void Executor::ins_classinst( const Instruction& ins )
{
  ValueStack.push_back( BObjectRef( new BConstObject(
      new BClassInstanceRef( new BClassInstance( prog_, ins.token.lval, Globals2 ) ) ) ) );
}

void Executor::ins_string( const Instruction& ins )
{
  ValueStack.push_back( BObjectRef( new BObject( new String( ins.token.tokval() ) ) ) );
}
void Executor::ins_error( const Instruction& /*ins*/ )
{
  ValueStack.push_back( BObjectRef( new BObject( new BError() ) ) );
}
void Executor::ins_struct( const Instruction& /*ins*/ )
{
  ValueStack.push_back( BObjectRef( new BObject( new BStruct ) ) );
}
void Executor::ins_spread( const Instruction& /*ins*/ )
{
  auto spread = new BSpread( ValueStack.back() );
  ValueStack.pop_back();
  ValueStack.push_back( BObjectRef( new BObject( spread ) ) );
}
void Executor::ins_array( const Instruction& /*ins*/ )
{
  ValueStack.push_back( BObjectRef( new BObject( new ObjArray ) ) );
}
void Executor::ins_dictionary( const Instruction& /*ins*/ )
{
  ValueStack.push_back( BObjectRef( new BObject( new BDictionary ) ) );
}
void Executor::ins_uninit( const Instruction& /*ins*/ )
{
  ValueStack.push_back( BObjectRef( new BObject( UninitObject::create() ) ) );
}
void Executor::ins_ident( const Instruction& /*ins*/ )
{
  ValueStack.push_back( BObjectRef( new BObject( new BError( "Please recompile this script" ) ) ) );
}

// case TOK_UNMINUS:
void Executor::ins_unminus( const Instruction& /*ins*/ )
{
  BObjectRef ref = getObjRef();
  BObjectImp* newobj;
  newobj = ref->impref().inverse();

  ValueStack.push_back( BObjectRef( new BObject( newobj ) ) );
}

// case TOK_UNPLUSPLUS:
void Executor::ins_unplusplus( const Instruction& /*ins*/ )
{
  BObjectRef ref = ValueStack.back();
  ref->impref().selfPlusPlus();
}

// case TOK_UNMINUSMINUS:
void Executor::ins_unminusminus( const Instruction& /*ins*/ )
{
  BObjectRef ref = ValueStack.back();
  ref->impref().selfMinusMinus();
}

// case TOK_UNPLUSPLUS_POST:
void Executor::ins_unplusplus_post( const Instruction& /*ins*/ )
{
  BObjectRef ref = ValueStack.back();
  BObjectImp* imp = ref->impptr();
  BObject* n = ref->clone();
  imp->selfPlusPlus();
  ValueStack.back().set( n );
}

// case TOK_UNMINUSMINUS_POST:
void Executor::ins_unminusminus_post( const Instruction& /*ins*/ )
{
  BObjectRef ref = ValueStack.back();
  BObjectImp* imp = ref->impptr();
  BObject* n = ref->clone();
  imp->selfMinusMinus();
  ValueStack.back().set( n );
}

// case INS_SET_MEMBER_ID_UNPLUSPLUS:
void Executor::ins_set_member_id_unplusplus( const Instruction& ins )
{
  BObjectRef ref = ValueStack.back();
  BObjectRef tmp = ref->impref().get_member_id( ins.token.lval );
  if ( !tmp->isa( BObjectImp::OTUninit ) &&
       !tmp->isa( BObjectImp::OTError ) )  // do nothing if curval is uninit or error
  {
    tmp->impref().selfPlusPlus();
    ref->impref().set_member_id( ins.token.lval, tmp->impptr(), false );
  }
  ValueStack.back().set( tmp.get() );
}

// case INS_SET_MEMBER_ID_UNPLUSPLUS_POST:
void Executor::ins_set_member_id_unplusplus_post( const Instruction& ins )
{
  BObjectRef ref = ValueStack.back();
  BObjectRef tmp = ref->impref().get_member_id( ins.token.lval );
  BObject* res = tmp->clone();
  if ( !tmp->isa( BObjectImp::OTUninit ) &&
       !tmp->isa( BObjectImp::OTError ) )  // do nothing if curval is uninit or error
  {
    tmp->impref().selfPlusPlus();
    ref->impref().set_member_id( ins.token.lval, tmp->impptr(), false );
  }
  ValueStack.back().set( res );
}

// case INS_SET_MEMBER_ID_UNMINUSMINUS:
void Executor::ins_set_member_id_unminusminus( const Instruction& ins )
{
  BObjectRef ref = ValueStack.back();
  BObjectRef tmp = ref->impref().get_member_id( ins.token.lval );
  if ( !tmp->isa( BObjectImp::OTUninit ) &&
       !tmp->isa( BObjectImp::OTError ) )  // do nothing if curval is uninit or error
  {
    tmp->impref().selfMinusMinus();
    ref->impref().set_member_id( ins.token.lval, tmp->impptr(), false );
  }
  ValueStack.back().set( tmp.get() );
}

// case INS_SET_MEMBER_ID_UNMINUSMINUS_POST:
void Executor::ins_set_member_id_unminusminus_post( const Instruction& ins )
{
  BObjectRef ref = ValueStack.back();
  BObjectRef tmp = ref->impref().get_member_id( ins.token.lval );
  BObject* res = tmp->clone();
  if ( !tmp->isa( BObjectImp::OTUninit ) &&
       !tmp->isa( BObjectImp::OTError ) )  // do nothing if curval is uninit or error
  {
    tmp->impref().selfMinusMinus();
    ref->impref().set_member_id( ins.token.lval, tmp->impptr(), false );
  }
  ValueStack.back().set( res );
}

// case TOK_LOG_NOT:
void Executor::ins_logical_not( const Instruction& /*ins*/ )
{
  BObjectRef ref = getObjRef();
  ValueStack.push_back( BObjectRef( new BObject( new BLong( (int)!ref->impptr()->isTrue() ) ) ) );
}

// case TOK_BITWISE_NOT:
void Executor::ins_bitwise_not( const Instruction& /*ins*/ )
{
  BObjectRef ref = getObjRef();
  ValueStack.push_back( BObjectRef( new BObject( ref->impptr()->bitnot() ) ) );
}

// case TOK_FUNCREF:
void Executor::ins_funcref( const Instruction& ins )
{
  if ( ins.token.lval >= static_cast<int>( prog_->function_references.size() ) )
  {
    POLLOG_ERRORLN( "Function reference index out of bounds: {} >= {}", ins.token.lval,
                    prog_->function_references.size() );
    seterror( true );
    return;
  }

  auto funcref_index = static_cast<unsigned>( ins.token.lval );

  ValueStack.push_back(
      BObjectRef( new BFunctionRef( prog_, funcref_index, Globals2, {} /* captures */ ) ) );
}

void Executor::ins_functor( const Instruction& ins )
{
  auto funcref_index = static_cast<int>( ins.token.type );

  const auto& ep_funcref = prog_->function_references[funcref_index];

  int capture_count = ep_funcref.capture_count;

  auto captures = ValueStackCont();
  while ( capture_count > 0 )
  {
    captures.push_back( ValueStack.back() );
    ValueStack.pop_back();
    capture_count--;
  }

  auto func = new BFunctionRef( prog_, funcref_index, Globals2, std::move( captures ) );

  ValueStack.push_back( BObjectRef( func ) );

  PC += ins.token.lval;
}

void Executor::ins_nop( const Instruction& /*ins*/ ) {}

ExecInstrFunc Executor::GetInstrFunc( const Token& token )
{
  switch ( token.id )
  {
  case INS_INITFOREACH:
    return &Executor::ins_initforeach;
  case INS_STEPFOREACH:
    return &Executor::ins_stepforeach;
  case INS_INITFOR:
    return &Executor::ins_initfor;
  case INS_NEXTFOR:
    return &Executor::ins_nextfor;
  case INS_CASEJMP:
    return &Executor::ins_casejmp;
  case RSV_JMPIFTRUE:
    return &Executor::ins_jmpiftrue;
  case RSV_JMPIFFALSE:
    return &Executor::ins_jmpiffalse;
  case RSV_LOCAL:
    return &Executor::ins_makeLocal;
  case RSV_GLOBAL:
  case TOK_GLOBALVAR:
    return &Executor::ins_globalvar;
  case TOK_LOCALVAR:
    return &Executor::ins_localvar;
  case TOK_LONG:
    return &Executor::ins_long;
  case TOK_DOUBLE:
    return &Executor::ins_double;
  case TOK_STRING:
    return &Executor::ins_string;
  case TOK_ERROR:
    return &Executor::ins_error;
  case TOK_STRUCT:
    return &Executor::ins_struct;
  case TOK_SPREAD:
    return &Executor::ins_spread;
  case TOK_CLASSINST:
    return &Executor::ins_classinst;
  case TOK_ARRAY:
    return &Executor::ins_array;
  case TOK_DICTIONARY:
    return &Executor::ins_dictionary;
  case TOK_FUNCREF:
    return &Executor::ins_funcref;
  case TOK_FUNCTOR:
    return &Executor::ins_functor;
  case INS_UNINIT:
    return &Executor::ins_uninit;
  case TOK_IDENT:
    return &Executor::ins_ident;
  case INS_ASSIGN_GLOBALVAR:
    return &Executor::ins_assign_globalvar;
  case INS_ASSIGN_LOCALVAR:
    return &Executor::ins_assign_localvar;
  case INS_ASSIGN_CONSUME:
    return &Executor::ins_assign_consume;
  case TOK_CONSUMER:
    return &Executor::ins_consume;
  case TOK_ASSIGN:
    return &Executor::ins_assign;
  case INS_SUBSCRIPT_ASSIGN:
    return &Executor::ins_array_assign;
  case INS_SUBSCRIPT_ASSIGN_CONSUME:
    return &Executor::ins_array_assign_consume;
  case INS_MULTISUBSCRIPT:
    return &Executor::ins_multisubscript;
  case INS_MULTISUBSCRIPT_ASSIGN:
    return &Executor::ins_multisubscript_assign;
  case INS_GET_MEMBER:
    return &Executor::ins_get_member;
  case INS_SET_MEMBER:
    return &Executor::ins_set_member;
  case INS_SET_MEMBER_CONSUME:
    return &Executor::ins_set_member_consume;

  case INS_UNPACK_SEQUENCE:
    return &Executor::ins_unpack_sequence;
  case INS_UNPACK_INDICES:
    return &Executor::ins_unpack_indices;
  case INS_TAKE_GLOBAL:
    return &Executor::ins_take_global;
  case INS_TAKE_LOCAL:
    return &Executor::ins_take_local;

  case INS_GET_MEMBER_ID:
    return &Executor::ins_get_member_id;  // test id
  case INS_SET_MEMBER_ID:
    return &Executor::ins_set_member_id;  // test id
  case INS_SET_MEMBER_ID_CONSUME:
    return &Executor::ins_set_member_id_consume;  // test id

  case INS_SET_MEMBER_ID_CONSUME_PLUSEQUAL:
    return &Executor::ins_set_member_id_consume_plusequal;  // test id
  case INS_SET_MEMBER_ID_CONSUME_MINUSEQUAL:
    return &Executor::ins_set_member_id_consume_minusequal;  // test id
  case INS_SET_MEMBER_ID_CONSUME_TIMESEQUAL:
    return &Executor::ins_set_member_id_consume_timesequal;  // test id
  case INS_SET_MEMBER_ID_CONSUME_DIVIDEEQUAL:
    return &Executor::ins_set_member_id_consume_divideequal;  // test id
  case INS_SET_MEMBER_ID_CONSUME_MODULUSEQUAL:
    return &Executor::ins_set_member_id_consume_modulusequal;  // test id

  case TOK_ADD:
    return &Executor::ins_add;
  case TOK_SUBTRACT:
    return &Executor::ins_subtract;
  case TOK_DIV:
    return &Executor::ins_div;
  case TOK_MULT:
    return &Executor::ins_mult;
  case TOK_MODULUS:
    return &Executor::ins_modulus;

  case TOK_INSERTINTO:
    return &Executor::ins_insert_into;

  case TOK_PLUSEQUAL:
    return &Executor::ins_plusequal;
  case TOK_MINUSEQUAL:
    return &Executor::ins_minusequal;
  case TOK_TIMESEQUAL:
    return &Executor::ins_timesequal;
  case TOK_DIVIDEEQUAL:
    return &Executor::ins_divideequal;
  case TOK_MODULUSEQUAL:
    return &Executor::ins_modulusequal;

  case TOK_LESSTHAN:
    return &Executor::ins_lessthan;
  case TOK_LESSEQ:
    return &Executor::ins_lessequal;
  case RSV_GOTO:
    return &Executor::ins_goto;
  case TOK_ARRAY_SUBSCRIPT:
    return &Executor::ins_arraysubscript;
  case TOK_EQUAL:
    return &Executor::ins_equal;
  case TOK_FUNC:
    return &Executor::ins_func;
  case INS_CALL_METHOD:
    return &Executor::ins_call_method;
  case INS_CALL_METHOD_ID:
    return &Executor::ins_call_method_id;
  case CTRL_STATEMENTBEGIN:
    return &Executor::ins_statementbegin;
  case CTRL_MAKELOCAL:
    return &Executor::ins_makelocal;
  case INS_CHECK_MRO:
    return &Executor::ins_check_mro;
  case CTRL_JSR_USERFUNC:
    return &Executor::ins_jsr_userfunc;
  case INS_POP_PARAM:
    return &Executor::ins_pop_param;
  case INS_POP_PARAM_BYREF:
    return &Executor::ins_pop_param_byref;
  case INS_GET_ARG:
    return &Executor::ins_get_arg;
  case CTRL_LEAVE_BLOCK:
    return &Executor::ins_leave_block;
  case RSV_GOSUB:
    return &Executor::ins_gosub;
  case RSV_RETURN:
    return &Executor::ins_return;
  case RSV_EXIT:
    return &Executor::ins_exit;
  case INS_DECLARE_ARRAY:
    return &Executor::ins_declareArray;
  case TOK_UNMINUS:
    return &Executor::ins_unminus;
  case TOK_UNPLUS:
    return &Executor::ins_nop;
  case TOK_LOG_NOT:
    return &Executor::ins_logical_not;
  case TOK_BITWISE_NOT:
    return &Executor::ins_bitwise_not;
  case TOK_BSRIGHT:
    return &Executor::ins_bitshift_right;
  case TOK_BSLEFT:
    return &Executor::ins_bitshift_left;
  case TOK_BITAND:
    return &Executor::ins_bitwise_and;
  case TOK_BITXOR:
    return &Executor::ins_bitwise_xor;
  case TOK_BITOR:
    return &Executor::ins_bitwise_or;

  case TOK_NEQ:
    return &Executor::ins_notequal;
  case TOK_GRTHAN:
    return &Executor::ins_greaterthan;
  case TOK_GREQ:
    return &Executor::ins_greaterequal;
  case TOK_AND:
    return &Executor::ins_logical_and;
  case TOK_OR:
    return &Executor::ins_logical_or;

  case TOK_ADDMEMBER:
    return &Executor::ins_addmember;
  case TOK_DELMEMBER:
    return &Executor::ins_removemember;
  case TOK_CHKMEMBER:
    return &Executor::ins_checkmember;
  case INS_DICTIONARY_ADDMEMBER:
    return &Executor::ins_dictionary_addmember;
  case TOK_IN:
    return &Executor::ins_in;
  case TOK_IS:
    return &Executor::ins_is;
  case INS_ADDMEMBER2:
    return &Executor::ins_addmember2;
  case INS_ADDMEMBER_ASSIGN:
    return &Executor::ins_addmember_assign;
  case CTRL_PROGEND:
    return &Executor::ins_progend;
  case TOK_UNPLUSPLUS:
    return &Executor::ins_unplusplus;
  case TOK_UNMINUSMINUS:
    return &Executor::ins_unminusminus;
  case TOK_UNPLUSPLUS_POST:
    return &Executor::ins_unplusplus_post;
  case TOK_UNMINUSMINUS_POST:
    return &Executor::ins_unminusminus_post;
  case INS_SET_MEMBER_ID_UNPLUSPLUS:
    return &Executor::ins_set_member_id_unplusplus;  // test id
  case INS_SET_MEMBER_ID_UNMINUSMINUS:
    return &Executor::ins_set_member_id_unminusminus;  // test id
  case INS_SET_MEMBER_ID_UNPLUSPLUS_POST:
    return &Executor::ins_set_member_id_unplusplus_post;  // test id
  case INS_SET_MEMBER_ID_UNMINUSMINUS_POST:
    return &Executor::ins_set_member_id_unminusminus_post;  // test id
  case INS_SKIPIFTRUE_ELSE_CONSUME:
    return &Executor::ins_skipiftrue_else_consume;
  case TOK_INTERPOLATE_STRING:
    return &Executor::ins_interpolate_string;
  case TOK_FORMAT_EXPRESSION:
    return &Executor::ins_format_expression;
  case TOK_BOOL:
    return &Executor::ins_bool;
  default:
    throw std::runtime_error( "Undefined execution token " + Clib::tostring( token.id ) );
  }
}

void Executor::sethalt( bool halt )
{
  halt_ = halt;

  if ( halt && dbg_env_ )
    if ( std::shared_ptr<ExecutorDebugListener> listener = dbg_env_->listener.lock() )
      listener->on_halt();

  calcrunnable();
}

void Executor::execInstr()
{
  unsigned onPC = PC;
  try
  {  // this is really more of a class invariant.
    passert( run_ok_ );
    passert( PC < nLines );
    passert( !error_ );
    passert( !done );

#ifdef NDEBUG
    const Instruction& ins = prog_->instr[PC];
#else
    const Instruction& ins = prog_->instr.at( PC );
#endif
    if ( debug_level >= INSTRUCTIONS )
      INFO_PRINTLN( "{}: {}", PC, ins.token );

    // If `on_instruction` returns false, do not execute this instruction.
    if ( dbg_env_ && !dbg_env_->on_instruction( *this ) )
    {
      return;
    }

    ++ins.cycles;
    ++prog_->instr_cycles;
    ++escript_instr_cycles;

    ++PC;

    ( this->*( ins.func ) )( ins );
  }
  catch ( std::exception& ex )
  {
    std::string tmp =
        fmt::format( "Exception in: {} PC={}: {}\n", prog_->name.get(), onPC, ex.what() );
    if ( !run_ok_ )
      tmp += "run_ok_ = false\n";
    if ( PC < nLines )
      fmt::format_to( std::back_inserter( tmp ), " PC < nLines: ({} < {})\n", PC, nLines );
    if ( error_ )
      tmp += "error_ = true\n";
    if ( done )
      tmp += "done = true\n";

    seterror( true );
    POLLOG_ERROR( tmp );

    show_context( onPC );
  }
#ifdef __unix__
  catch ( ... )
  {
    seterror( true );
    POLLOG_ERRORLN( "Exception in {}, PC={}: unclassified", prog_->name.get(), onPC );

    show_context( onPC );
  }
#endif
}

std::string Executor::dbg_get_instruction( size_t atPC ) const
{
  std::string out;
  dbg_get_instruction( atPC, out );
  return out;
}

void Executor::dbg_get_instruction( size_t atPC, std::string& os ) const
{
  bool has_breakpoint =
      dbg_env_ ? dbg_env_->breakpoints.count( static_cast<unsigned>( atPC ) ) : false;
  fmt::format_to( std::back_inserter( os ), "{}{}{} {}", ( atPC == PC ) ? ">" : " ", atPC,
                  has_breakpoint ? "*" : ":", prog_->instr[atPC].token );
}

void Executor::show_context( unsigned atPC )
{
  unsigned start, end;
  if ( atPC >= 5 )
    start = atPC - 5;
  else
    start = 0;

  end = atPC + 5;

  if ( end >= nLines )
    end = nLines - 1;

  for ( unsigned i = start; i <= end; ++i )
  {
    POLLOGLN( "{}: {}", i, dbg_get_instruction( i ) );
  }
}
void Executor::show_context( std::string& os, unsigned atPC )
{
  unsigned start, end;
  if ( atPC >= 5 )
    start = atPC - 5;
  else
    start = 0;

  end = atPC + 5;

  if ( end >= nLines )
    end = nLines - 1;

  for ( unsigned i = start; i <= end; ++i )
  {
    dbg_get_instruction( i, os );
    os += '\n';
  }
}

void Executor::call_function_reference( BFunctionRef* funcr, BContinuation* continuation,
                                        const Instruction& jmp )
{
  // params need to be on the stack, without current objectref
  ValueStack.pop_back();

  // Push captured parameters onto the stack prior to function parameters.
  for ( auto& p : funcr->captures )
    ValueStack.push_back( p );

  auto nparams = static_cast<int>( fparams.size() );

  // Handle variadic functions special. Construct an array{} corresponding to
  // the rest parameter (the last parameter for the function). The logic for the
  // condition:
  // - if true, the last argument in the call may not be an array{}, so we need
  //   to construct one (which is why the condition is `>=` and not `>`).
  // - if false, then the address we're jumping to will be a "default argument
  //   address" and _not_ the user function directly, which will create the
  //   array{}. (NB: The address/PC comes from BFunctionRef::validCall)
  if ( funcr->variadic() && nparams >= funcr->numParams() )
  {
    auto num_nonrest_args = funcr->numParams() - 1;

    auto rest_arg = std::make_unique<ObjArray>();

    for ( int i = 0; i < static_cast<int>( fparams.size() ); ++i )
    {
      auto& p = fparams[i];

      if ( i < num_nonrest_args )
      {
        ValueStack.push_back( p );
      }
      else
      {
        rest_arg->ref_arr.push_back( p );
      }
    }
    ValueStack.push_back( BObjectRef( rest_arg.release() ) );
  }
  // The array{} will be created via the regular default-parameter handling by
  // jumping to the address/PC which pushes an empty array{} on the ValueStack
  // prior to jumping to the user function.
  else
  {
    for ( auto& p : fparams )
      ValueStack.push_back( p );
  }

  // jump to function
  jump( jmp.token.lval, continuation, funcr );
  fparams.clear();
  // switch to new block
  ins_makelocal( jmp );
}

bool Executor::exec()
{
  passert( prog_ok_ );
  passert( !error_ );

  Clib::scripts_thread_script = scriptname();

  set_running_to_completion( true );
  while ( runnable() )
  {
    Clib::scripts_thread_scriptPC = PC;
    execInstr();
  }

  return !error_;
}

void Executor::reinitExec()
{
  PC = 0;
  done = 0;
  seterror( false );

  ValueStack.clear();
  delete Locals2;
  Locals2 = new BObjectRefVec;

  if ( !prog_ok_ )
  {
    seterror( true );
  }
}

void Executor::initForFnCall( unsigned in_PC )
{
#ifdef MEMORYLEAK
  bool data_shown = false;
#endif

  PC = in_PC;
  done = 0;
  seterror( false );

#ifdef MEMORYLEAK
  while ( !ValueStack.empty() )
  {
    if ( Clib::memoryleak_debug )
    {
      if ( !data_shown )
      {
        LEAKLOG( "ValueStack... " );
        data_shown = true;
      }

      LEAKLOG( "{} [{}]", ValueStack.back()->impptr()->pack(),
               ValueStack.back()->impptr()->sizeEstimate() );
    }
    ValueStack.pop_back();
  }
  if ( Clib::memoryleak_debug )
    if ( data_shown )
      LEAKLOGLN( " ...deleted" );
#endif

  ValueStack.clear();
  Locals2->clear();
}

void Executor::pushArg( BObjectImp* arg )
{
  passert_always( arg );
  ValueStack.push_back( BObjectRef( arg ) );
}

void Executor::pushArg( const BObjectRef& arg )
{
  ValueStack.push_back( arg );
}

void Executor::addModule( ExecutorModule* module )
{
  availmodules.push_back( module );
}


ExecutorModule* Executor::findModule( const std::string& name )
{
  unsigned idx;
  for ( idx = 0; idx < availmodules.size(); idx++ )
  {
    ExecutorModule* module = availmodules[idx];
    if ( stricmp( module->moduleName.get().c_str(), name.c_str() ) == 0 )
      return module;
  }
  return nullptr;
}

bool Executor::attach_debugger( std::weak_ptr<ExecutorDebugListener> listener, bool set_attaching )
{
  // FIXME: a script can be in debugging state but have no debugger attached,
  // eg. a script that called `os::Debugger()`. This needs to check if a
  // debugger is attached. This works for `os::Debugger()` but not for poldbg cmd_attach.
  if ( dbg_env_ )
  {
    if ( !listener.expired() )
    {
      auto& dbg_env_listener = dbg_env_->listener;
      if ( !dbg_env_listener.expired() )
      {
        return false;
      }
      dbg_env_listener = listener;
    }
    if ( set_attaching )
      dbg_env_->debug_state = ExecutorDebugState::ATTACHING;
  }
  else
  {
    dbg_env_ = std::make_unique<ExecutorDebugEnvironment>( listener, set_attaching );
  }

  return true;
}

void Executor::detach_debugger()
{
  dbg_env_.reset();
  sethalt( false );
}

void Executor::print_to_debugger( const std::string& message )
{
  if ( dbg_env_ )
  {
    if ( std::shared_ptr<ExecutorDebugListener> listener = dbg_env_->listener.lock() )
      listener->on_print( message );
  }
}

void Executor::dbg_ins_trace()
{
  if ( dbg_env_ )
  {
    dbg_env_->debug_state = ExecutorDebugState::INS_TRACE;
  }
  sethalt( false );
}
void Executor::dbg_step_into()
{
  if ( dbg_env_ )
  {
    dbg_env_->debug_state = ExecutorDebugState::STEP_INTO;
  }
  sethalt( false );
}
void Executor::dbg_step_over()
{
  if ( dbg_env_ )
  {
    dbg_env_->debug_state = ExecutorDebugState::STEP_OVER;
  }
  sethalt( false );
}
void Executor::dbg_step_out()
{
  if ( dbg_env_ )
  {
    dbg_env_->debug_state = ExecutorDebugState::STEP_OUT;
  }
  sethalt( false );
}
void Executor::dbg_run()
{
  if ( dbg_env_ )
  {
    dbg_env_->debug_state = ExecutorDebugState::RUN;
  }
  sethalt( false );
}
void Executor::dbg_break()
{
  if ( dbg_env_ )
  {
    dbg_env_->debug_state = ExecutorDebugState::BREAK_INTO;
  }
}

void Executor::dbg_setbp( unsigned atPC )
{
  if ( dbg_env_ )
  {
    dbg_env_->breakpoints.insert( atPC );
  }
}
void Executor::dbg_clrbp( unsigned atPC )
{
  if ( dbg_env_ )
  {
    dbg_env_->breakpoints.erase( atPC );
  }
}

void Executor::dbg_clrbps( const std::set<unsigned>& PCs )
{
  if ( dbg_env_ )
  {
    std::set<unsigned> result;
    auto& breakpoints = dbg_env_->breakpoints;
    std::set_difference( breakpoints.begin(), breakpoints.end(), PCs.begin(), PCs.end(),
                         std::inserter( result, result.end() ) );
    breakpoints = result;
  }
}

void Executor::dbg_clrallbp()
{
  if ( dbg_env_ )
  {
    dbg_env_->breakpoints.clear();
  }
}

size_t Executor::sizeEstimate() const
{
  size_t size = sizeof( *this );
  size += Clib::memsize( upperLocals2 );
  for ( const auto& bobjectrefvec : upperLocals2 )
  {
    size += Clib::memsize( *bobjectrefvec );
    for ( const auto& bojectref : *bobjectrefvec )
    {
      if ( bojectref != nullptr )
        size += bojectref->sizeEstimate();
    }
  }
  size += Clib::memsize( ControlStack );

  size += Clib::memsize( *Locals2 );
  for ( const auto& bojectref : *Locals2 )
  {
    if ( bojectref != nullptr )
      size += bojectref->sizeEstimate();
  }
  size += Clib::memsize( *Globals2 );
  for ( const auto& bojectref : *Globals2 )
  {
    if ( bojectref != nullptr )
      size += bojectref->sizeEstimate();
  }
  size += Clib::memsize( ValueStack );
  for ( const auto& bojectref : ValueStack )
  {
    if ( bojectref != nullptr )
      size += bojectref->sizeEstimate();
  }
  size += Clib::memsize( fparams );
  for ( const auto& bojectref : fparams )
  {
    if ( bojectref != nullptr )
      size += bojectref->sizeEstimate();
  }
  for ( const auto& module : availmodules )
  {
    if ( module != nullptr )
      size += module->sizeEstimate();
  }
  size += Clib::memsize( execmodules ) + Clib::memsize( availmodules );
  size += dbg_env_ != nullptr ? dbg_env_->sizeEstimate() : 0;
  size += func_result_ != nullptr ? func_result_->sizeEstimate() : 0;
  size += Clib::memsize( class_methods );
  return size;
}

bool Executor::builtinMethodForced( const char*& methodname )
{
  if ( methodname[0] == '_' )
  {
    ++methodname;
    return true;
  }
  return false;
}

#ifdef ESCRIPT_PROFILE
void Executor::profile_escript( std::string name, unsigned long profile_start )
{
  unsigned long profile_end = GetTimeUs() - profile_start;
  escript_profile_map::iterator itr = EscriptProfileMap.find( name );
  if ( itr != EscriptProfileMap.end() )
  {
    itr->second.count++;
    itr->second.sum += profile_end;
    if ( itr->second.max < profile_end )
      itr->second.max = profile_end;
    else if ( itr->second.min > profile_end )
      itr->second.min = profile_end;
  }
  else
  {
    profile_instr profInstr;
    profInstr.count = 1;
    profInstr.max = profile_end;
    profInstr.min = profile_end;
    profInstr.sum = profile_end;
    EscriptProfileMap[name] = profInstr;
  }
}
#ifdef _WIN32

unsigned long Executor::GetTimeUs()
{
  static bool bInitialized = false;
  static LARGE_INTEGER lFreq, lStart;
  static LARGE_INTEGER lDivisor;
  if ( !bInitialized )
  {
    bInitialized = true;
    QueryPerformanceFrequency( &lFreq );
    QueryPerformanceCounter( &lStart );
    lDivisor.QuadPart = lFreq.QuadPart / 1000000;
  }

  LARGE_INTEGER lEnd;
  QueryPerformanceCounter( &lEnd );
  double duration = double( lEnd.QuadPart - lStart.QuadPart ) / lFreq.QuadPart;
  duration *= 1000000;
  LONGLONG llDuration = static_cast<LONGLONG>( duration );
  return llDuration & 0xffffffff;
}
#else
unsigned long Executor::GetTimeUs()
{
  static bool bInitialized = false;
  static timeval t1;
  if ( !bInitialized )
  {
    bInitialized = true;
    gettimeofday( &t1, nullptr );
  }

  timeval t2;
  gettimeofday( &t2, nullptr );

  double elapsedTime;
  elapsedTime = ( t2.tv_sec - t1.tv_sec ) * 1000000.0;
  elapsedTime += ( t2.tv_usec - t1.tv_usec );

  long long llDuration = static_cast<long long>( elapsedTime );
  return llDuration & 0xffffffff;
}
#endif
#endif
BContinuation* Executor::withContinuation( BContinuation* continuation, BObjectRefVec args )
{
  auto* func = continuation->func();

  // Add function arguments to value stack. Add arguments if there are not enough.  Remove if
  // there are too many
  while ( func->numParams() > static_cast<int>( args.size() ) )
  {
    args.push_back( BObjectRef( new BObject( UninitObject::create() ) ) );
  }

  // Resize args only for non-varadic functions
  if ( !func->variadic() )
    args.resize( func->numParams() );

  continuation->args = std::move( args );

  return continuation;
}

bool Executor::ClassMethodKey::operator<( const ClassMethodKey& other ) const
{
  // Compare the program pointers
  if ( prog < other.prog )
    return true;
  if ( prog > other.prog )
    return false;

  // Compare the indices
  if ( index < other.index )
    return true;
  if ( index > other.index )
    return false;

  // Perform a case-insensitive comparison for method_name using stricmp
  return stricmp( method_name.c_str(), other.method_name.c_str() ) < 0;
}
}  // namespace Bscript
}  // namespace Pol
