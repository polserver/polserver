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

#include "../clib/clib.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "berror.h"
#include "config.h"
#include "contiter.h"
#include "dict.h"
#include "eprog.h"
#include "escriptv.h"
#include "execmodl.h"
#include "fmodule.h"
#include "impstr.h"
#include "token.h"
#include "tokens.h"
#ifdef MEMORYLEAK
#include "../clib/mlog.h"
#endif

#include <cstdlib>
#include <cstring>
#include <exception>

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
      INFO_PRINT << ex->scriptname() << "\n";
  }
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
      Locals2( new BObjectRefVec ),
      nLines( 0 ),
      current_module_function( nullptr ),
      prog_ok_( false ),
      viewmode_( false ),
      runs_to_completion_( false ),
      debugging_( false ),
      debug_state_( DEBUG_STATE_NONE ),
      breakpoints_(),
      bp_skip_( ~0u ),
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
      ERROR_PRINT << "WARNING: " << scriptname() << ": Unable to find module "
                  << fm->modulename.get() << "\n";
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
            ERROR_PRINT << "Unable to find " << fm->modulename.get() << "::" << func->name.get()
                        << "\n";
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
        POLLOG_ERROR.Format( "Fatal error: Value Stack Empty! ({},PC={})\n" ) << prog_->name << PC;
        seterror( true );
        return -1;
      }
      fparams[i] = ValueStack.back();
      ValueStack.pop_back();
    }
  }
  return 0;
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

  fparams[param].set( new BObject( new Double( static_cast<Double&>( obj->impref() ) ) ) );

  return 0;
}

double Executor::paramAsDouble( unsigned param )
{
  makeDouble( param );
  BObjectImp* objimp = getParam( param )->impptr();

  Double* dbl = (Double*)objimp;
  return dbl ? dbl->value() : 0.0;
}

int Executor::paramAsLong( unsigned param )
{
  BObjectImp* objimp = getParam( param )->impptr();
  if ( objimp->isa( BObjectImp::OTLong ) )
  {
    return ( (BLong*)objimp )->value();
  }
  else if ( objimp->isa( BObjectImp::OTDouble ) )
  {
    return static_cast<int>( ( (Double*)objimp )->value() );
  }
  else
  {
    return 0;
  }
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
      fmt::Writer tmp;
      tmp << "Script Error in '" << scriptname() << "' PC=" << PC << ": \n";
      if ( current_module_function )
        tmp << "\tCall to function " << current_module_function->name.get() << ":\n";
      else
        tmp << "\tCall to an object method.\n";
      tmp << "\tParameter " << param << ": Expected datatype " << BObjectImp::typestr( type )
          << ", got datatype " << BObjectImp::typestr( imp->type() ) << "\n";
      DEBUGLOG << tmp.str();
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
  if ( imp->isa( BObjectImp::OTDouble ) )
  {
    value = static_cast<Double*>( imp )->value();
    return true;
  }
  else if ( imp->isa( BObjectImp::OTLong ) )
  {
    value = static_cast<BLong*>( imp )->value();
    return true;
  }
  else
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << PC << ": \n"
             << "\tCall to function " << current_module_function->name.get() << ":\n"
             << "\tParameter " << param << ": Expected Integer or Real"
             << ", got datatype " << BObjectImp::typestr( imp->type() ) << "\n";

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
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << PC << ": \n"
             << "\tCall to function " << current_module_function->name.get() << ":\n"
             << "\tParameter " << param
             << ": Expected datatype " /*<< pointer_type TODO this is totally useless since its a
                                          pointer address*/
             << ", got datatype " << BObjectImp::typestr( ap->type() ) << "\n";

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
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << PC << ": \n"
             << "\tCall to function " << current_module_function->name.get() << ":\n"
             << "\tParameter " << param
             << ": Expected datatype " /*<< object_type TODO this is totally useless since its a
                                          pointer address*/
             << ", got datatype " << aob->getStringRep() << "\n";

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

bool Executor::getUnicodeStringParam( unsigned param, const String*& pstr )
{
  BObject* obj = getParam( param );
  if ( !obj )
    return false;
  if ( obj->isa( BObjectImp::OTString ) )
  {
    pstr = static_cast<String*>( obj->impptr() );
    return true;
  }
  else if ( obj->isa( BObjectImp::OTArray ) )
  {
    pstr = String::fromUCArray( static_cast<ObjArray*>( obj->impptr() ) );
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
  passert( varnum < Globals2.size() );
  return Globals2[varnum];
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

  Globals2.clear();
  for ( unsigned i = 0; i < prog_->nglobals; ++i )
  {
    Globals2.push_back( BObjectRef() );
    Globals2.back().set( new BObject( UninitObject::create() ) );
  }

  prog_ok_ = true;
  seterror( false );
  ++prog_->invocations;
  return true;
}

BObject* Executor::makeObj( const Token& token )
{
  switch ( token.id )
  {
  case TOK_IDENT:
    return new BObject( new BError( "Please recompile this script!" ) );
  case TOK_LOCALVAR:
    return LocalVar( token.lval ).get();
  case TOK_GLOBALVAR:
    return GlobalVar( token.lval ).get();
  case TOK_STRING:
    return new BObject( new String( token.tokval() ) );
  case TOK_LONG:
    return new BObject( new BLong( token.lval ) );
  case TOK_DOUBLE:
    return new BObject( new Double( token.dval ) );
  case TOK_ERROR:
    return new BObject( new BError( "unknown" ) );
  default:
    passert( 0 );
    break;
  }
  return nullptr;
}

BObjectRef Executor::getObjRef( void )
{
  if ( ValueStack.empty() )
  {
    POLLOG_ERROR.Format( "Fatal error: Value Stack Empty! ({},PC={})\n" ) << prog_->name << PC;
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
    DEBUGLOG << "Error in script '" << prog_->name.get() << "':\n"
             << "\tModule Function " << modfunc->name.get() << " was not found.\n";

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
    ERROR_PRINT << "variable is already initialized..\n";
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

  const String& varname = static_cast<const String&>( right.impref() );

  return left.impref().operDotPlus( varname.data() );
}

BObjectRef Executor::removemember( BObject& left, const BObject& right )
{
  if ( !right.isa( BObjectImp::OTString ) )
  {
    return BObjectRef( left.clone() );
  }

  const String& varname = static_cast<const String&>( right.impref() );

  return left.impref().operDotMinus( varname.data() );
}

BObjectRef Executor::checkmember( BObject& left, const BObject& right )
{
  if ( !right.isa( BObjectImp::OTString ) )
  {
    return BObjectRef( left.clone() );
  }

  const String& varname = static_cast<const String&>( right.impref() );

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
  Locals2->push_back( BObjectRef() );  // the iterator

  // this is almost like popParam, only we don't want a copy.
  BObjectRef objref = getObjRef();
  Locals2->push_back( BObjectRef() );
  Locals2->back().set( objref.get() );

  Locals2->push_back( BObjectRef() );
  Locals2->back().set( new BObject( new BLong( 0 ) ) );

  PC = ins.token.lval;
}


void Executor::ins_stepforeach( const Instruction& ins )
{
  unsigned locsize = static_cast<unsigned int>( Locals2->size() );
  ObjArray* arr = static_cast<ObjArray*>( ( *Locals2 )[locsize - 2]->impptr() );
  if ( !arr->isa( BObjectImp::OTArray ) )
    return;
  BLong* blong = static_cast<BLong*>( ( *Locals2 )[locsize - 1]->impptr() );

  if ( blong->increment() > int( arr->ref_arr.size() ) )
    return;

  BObjectRef& objref = arr->ref_arr[blong->value() - 1];
  BObject* elem = objref.get();
  if ( elem == nullptr )
  {
    elem = new BObject( UninitObject::create() );
    objref.set( elem );
  }
  ( *Locals2 )[locsize - 3].set( elem );
  PC = ins.token.lval;
}

void Executor::ins_initforeach2( const Instruction& ins )
{
  Locals2->push_back( BObjectRef() );  // the iterator


  auto pIterVal = new BObject( UninitObject::create() );

  // this is almost like popParam, only we don't want a copy.
  BObjectRef objref = getObjRef();
  Locals2->push_back( BObjectRef() );
  ContIterator* pIter = objref->impptr()->createIterator( pIterVal );
  Locals2->back().set( new BObject( pIter ) );

  Locals2->push_back( BObjectRef() );
  Locals2->back().set( pIterVal );

  PC = ins.token.lval;
}


void Executor::ins_stepforeach2( const Instruction& ins )
{
  size_t locsize = Locals2->size();
  ContIterator* pIter = static_cast<ContIterator*>( ( *Locals2 )[locsize - 2]->impptr() );

  BObject* next = pIter->step();
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

  if ( itr->isa( BObjectImp::OTLong ) )
  {
    BLong* blong = static_cast<BLong*>( itr );
    blong->increment();
  }
  else if ( itr->isa( BObjectImp::OTDouble ) )
  {
    Double* dbl = static_cast<Double*>( itr );
    dbl->increment();
  }

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
    else
    {
      dataptr += type;
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
    else
    {
      if ( bstring.size() == type && memcmp( bstring.data(), dataptr, type ) == 0 )
      {
        return offset;
      }
      dataptr += type;
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
    else
    {
      dataptr += type;
    }
  }
}

void Executor::ins_casejmp( const Instruction& ins )
{
  BObjectRef& objref = ValueStack.back();
  BObjectImp* objimp = objref->impptr();
  if ( objimp->isa( BObjectImp::OTLong ) )
  {
    PC = ins_casejmp_findlong( ins.token, static_cast<BLong*>( objimp ) );
  }
  else if ( objimp->isa( BObjectImp::OTString ) )
  {
    PC = ins_casejmp_findstring( ins.token, static_cast<String*>( objimp ) );
  }
  else
  {
    PC = ins_casejmp_finddefault( ins.token );
  }
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


// case TOK_LOCALVAR:
void Executor::ins_localvar( const Instruction& ins )
{
  ValueStack.push_back( ( *Locals2 )[ins.token.lval] );
}

// case RSV_GLOBAL:
// case TOK_GLOBALVAR:
void Executor::ins_globalvar( const Instruction& ins )
{
  ValueStack.push_back( Globals2[ins.token.lval] );
}

// case TOK_LONG:
void Executor::ins_long( const Instruction& ins )
{
  ValueStack.push_back( BObjectRef( new BObject( new BLong( ins.token.lval ) ) ) );
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
  BObjectRef& gvar = Globals2[ins.token.lval];

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
  BDictionary* dict = static_cast<BDictionary*>( dictob.impptr() );

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

  left.impref().operInsertInto( left, right.impref() );
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
  unsigned nparams = ins.token.type;
  getParams( nparams );

  if ( ValueStack.back()->isa( BObjectImp::OTFuncRef ) )
  {
    BObjectRef objref = ValueStack.back();
    auto funcr = static_cast<BFunctionRef*>( objref->impptr() );
    Instruction jmp;
    if ( funcr->validCall( ins.token.lval, *this, &jmp ) )
    {
      // params need to be on the stack, without current objectref
      ValueStack.pop_back();
      for ( auto& p : fparams )
        ValueStack.push_back( p );
      // jump to function
      ins_jsr_userfunc( jmp );
      fparams.clear();
      // switch to new block
      ins_makelocal( jmp );
      return;
    }
  }
  BObjectRef& objref = ValueStack.back();
#ifdef ESCRIPT_PROFILE
  std::stringstream strm;
  strm << "MTHID_" << objref->impptr()->typeOf() << " ." << ins.token.lval;
  if ( !fparams.empty() )
    strm << " [" << fparams[0].get()->impptr()->typeOf() << "]";
  std::string name( strm.str() );
  unsigned long profile_start = GetTimeUs();
#endif
  BObjectImp* imp = objref->impptr()->call_method_id( ins.token.lval, *this );
#ifdef ESCRIPT_PROFILE
  profile_escript( name, profile_start );
#endif
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

void Executor::ins_call_method( const Instruction& ins )
{
  unsigned nparams = ins.token.lval;
  getParams( nparams );

  if ( ValueStack.back()->isa( BObjectImp::OTFuncRef ) )
  {
    BObjectRef objref = ValueStack.back();
    auto funcr = static_cast<BFunctionRef*>( objref->impptr() );
    Instruction jmp;
    if ( funcr->validCall( ins.token.tokval(), *this, &jmp ) )
    {
      // params need to be on the stack, without current objectref
      ValueStack.pop_back();
      for ( auto& p : fparams )
        ValueStack.push_back( p );
      // jump to function
      ins_jsr_userfunc( jmp );
      fparams.clear();
      // switch to new block
      ins_makelocal( jmp );
      return;
    }
  }

  BObjectRef& objref = ValueStack.back();
#ifdef ESCRIPT_PROFILE
  std::stringstream strm;
  strm << "MTH_" << objref->impptr()->typeOf() << " ." << ins.token.tokval();
  if ( !fparams.empty() )
    strm << " [" << fparams[0].get()->impptr()->typeOf() << "]";
  std::string name( strm.str() );
  unsigned long profile_start = GetTimeUs();
#endif
  BObjectImp* imp = objref->impptr()->call_method( ins.token.tokval(), *this );
#ifdef ESCRIPT_PROFILE
  profile_escript( name, profile_start );
#endif

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
    INFO_PRINT << ins.token.tokval() << "\n";
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

// CTRL_JSR_USERFUNC:
void Executor::ins_jsr_userfunc( const Instruction& ins )
{
  ReturnContext rc;
  rc.PC = PC;
  rc.ValueStackDepth = static_cast<unsigned int>( ValueStack.size() );
  ControlStack.push_back( rc );

  PC = (unsigned)ins.token.lval;
  if ( ControlStack.size() >= escript_config.max_call_depth )
  {
    fmt::Writer tmp;
    tmp << "Script " << scriptname() << " exceeded maximum call depth\n"
        << "Return path PCs: ";
    while ( !ControlStack.empty() )
    {
      rc = ControlStack.back();
      ControlStack.pop_back();
      tmp << rc.PC << " ";
    }
    POLLOG << tmp.str() << "\n";
    seterror( true );
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
      Globals2.pop_back();
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
    ERROR_PRINT << "Return without GOSUB!\n";

    seterror( true );
    return;
  }
  ReturnContext& rc = ControlStack.back();
  PC = rc.PC;
  // FIXME do something with rc.ValueStackDepth
  ControlStack.pop_back();

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
  ValueStack.push_back( BObjectRef(
      new BObject( new BFunctionRef( ins.token.lval, ins.token.type, scriptname() ) ) ) );
}

void Executor::ins_nop( const Instruction& /*ins*/ ) {}

ExecInstrFunc Executor::GetInstrFunc( const Token& token )
{
  switch ( token.id )
  {
  case INS_INITFOREACH:
    return &Executor::ins_initforeach2;
  case INS_STEPFOREACH:
    return &Executor::ins_stepforeach2;
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
  case TOK_ARRAY:
    return &Executor::ins_array;
  case TOK_DICTIONARY:
    return &Executor::ins_dictionary;
  case TOK_FUNCREF:
    return &Executor::ins_funcref;
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

  default:
    throw std::runtime_error( "Undefined execution token " + Clib::tostring( token.id ) );
  }
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
      INFO_PRINT << PC << ": " << ins.token << "\n";

    if ( debugging_ )
    {
      if ( debug_state_ == DEBUG_STATE_ATTACHING )
      {
        debug_state_ = DEBUG_STATE_ATTACHED;
        sethalt( true );
        return;
      }
      else if ( debug_state_ == DEBUG_STATE_INS_TRACE )
      {
        // let this instruction through.
        debug_state_ = DEBUG_STATE_ATTACHED;
        sethalt( true );
        // but let this instruction execute.
      }
      else if ( debug_state_ == DEBUG_STATE_STEP_INTO )
      {
        debug_state_ = DEBUG_STATE_STEPPING_INTO;
        // let this instruction execute.
      }
      else if ( debug_state_ == DEBUG_STATE_STEPPING_INTO )
      {
        if ( prog_->dbg_ins_statementbegin.size() > PC && prog_->dbg_ins_statementbegin[PC] )
        {
          tmpbreakpoints_.insert( PC );
          // and let breakpoint processing catch it below.
        }
      }
      else if ( debug_state_ == DEBUG_STATE_STEP_OVER )
      {
        unsigned breakPC = PC + 1;
        while ( prog_->dbg_ins_statementbegin.size() > breakPC )
        {
          if ( prog_->dbg_ins_statementbegin[breakPC] )
          {
            tmpbreakpoints_.insert( breakPC );
            debug_state_ = DEBUG_STATE_RUN;
            break;
          }
          else
          {
            ++breakPC;
          }
        }
      }
      else if ( debug_state_ == DEBUG_STATE_RUN )
      {
        // do nothing
      }
      else if ( debug_state_ == DEBUG_STATE_BREAK_INTO )
      {
        debug_state_ = DEBUG_STATE_ATTACHED;
        sethalt( true );
        return;
      }

      // check for breakpoints on this instruction
      if ( ( breakpoints_.count( PC ) || tmpbreakpoints_.count( PC ) ) && bp_skip_ != PC &&
           !halt() )
      {
        tmpbreakpoints_.erase( PC );
        bp_skip_ = PC;
        debug_state_ = DEBUG_STATE_ATTACHED;
        sethalt( true );
        return;
      }
      bp_skip_ = ~0u;
    }

    ++ins.cycles;
    ++prog_->instr_cycles;
    ++escript_instr_cycles;

    ++PC;

    ( this->*( ins.func ) )( ins );
  }
  catch ( std::exception& ex )
  {
    fmt::Writer tmp;
    tmp << "Exception in: " << prog_->name.get() << " PC=" << onPC << ": " << ex.what() << "\n";
    if ( !run_ok_ )
      tmp << "run_ok_ = false\n";
    if ( PC < nLines )
    {
      tmp << " PC < nLines: (" << PC << " < " << nLines << ") \n";
    }
    if ( error_ )
      tmp << "error_ = true\n";
    if ( done )
      tmp << "done = true\n";

    seterror( true );
    POLLOG_ERROR << tmp.str();

    show_context( onPC );
  }
#ifdef __unix__
  catch ( ... )
  {
    seterror( true );
    POLLOG_ERROR << "Exception in " << prog_->name.get() << ", PC=" << onPC << ": unclassified\n";

    show_context( onPC );
  }
#endif
}

std::string Executor::dbg_get_instruction( size_t atPC ) const
{
  fmt::Writer os;
  os << ( ( atPC == PC ) ? ">" : " " ) << atPC
     << ( breakpoints_.count( static_cast<unsigned>( atPC ) ) ? "*" : ":" ) << " "
     << prog_->instr[atPC].token;
  return os.str();
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
    POLLOG.Format( "{}: {}\n" ) << i << dbg_get_instruction( i );
  }
}
void Executor::show_context( fmt::Writer& os, unsigned atPC )
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
    os << dbg_get_instruction( i ) << '\n';
  }
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

  while ( !ValueStack.empty() )
    ValueStack.pop_back();

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
        LEAKLOG << "ValueStack... ";
        data_shown = true;
      }

      LEAKLOG << ValueStack.back()->impptr()->pack();
      LEAKLOG << " [" << ValueStack.back()->impptr()->sizeEstimate() << "] ";
    }
    ValueStack.pop_back();
  }
  if ( Clib::memoryleak_debug )
    if ( data_shown )
      LEAKLOG << " ...deleted\n";
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

void Executor::attach_debugger()
{
  setdebugging( true );
  debug_state_ = DEBUG_STATE_ATTACHING;
}
void Executor::detach_debugger()
{
  setdebugging( false );
  debug_state_ = DEBUG_STATE_NONE;
  sethalt( false );
}
void Executor::dbg_ins_trace()
{
  debug_state_ = DEBUG_STATE_INS_TRACE;
  sethalt( false );
}
void Executor::dbg_step_into()
{
  debug_state_ = DEBUG_STATE_STEP_INTO;
  sethalt( false );
}
void Executor::dbg_step_over()
{
  debug_state_ = DEBUG_STATE_STEP_OVER;
  sethalt( false );
}
void Executor::dbg_run()
{
  debug_state_ = DEBUG_STATE_RUN;
  sethalt( false );
}
void Executor::dbg_break()
{
  debug_state_ = DEBUG_STATE_BREAK_INTO;
}

void Executor::dbg_setbp( unsigned atPC )
{
  breakpoints_.insert( atPC );
}
void Executor::dbg_clrbp( unsigned atPC )
{
  breakpoints_.erase( atPC );
}
void Executor::dbg_clrallbp()
{
  breakpoints_.clear();
}

size_t Executor::sizeEstimate() const
{
  size_t size = sizeof( *this );
  size += 3 * sizeof( BObjectRefVec** ) + upperLocals2.size() * sizeof( BObjectRefVec* );
  for ( const auto& bojectrefvec : upperLocals2 )
  {
    size += 3 * sizeof( BObjectRef* ) + bojectrefvec->capacity() * sizeof( BObjectRef );
    for ( const auto& bojectref : *bojectrefvec )
    {
      if ( bojectref != nullptr )
        size += bojectref->sizeEstimate();
    }
  }
  size += 3 * sizeof( ReturnContext* ) + ControlStack.size() * sizeof( ReturnContext );

  size += 3 * sizeof( BObjectRef* ) + Locals2->size() * sizeof( BObjectRef );
  for ( const auto& bojectref : *Locals2 )
  {
    if ( bojectref != nullptr )
      size += bojectref->sizeEstimate();
  }
  size += 3 * sizeof( BObjectRef* ) + Globals2.size() * sizeof( BObjectRef );
  for ( const auto& bojectref : Globals2 )
  {
    if ( bojectref != nullptr )
      size += bojectref->sizeEstimate();
  }
  size += 3 * sizeof( BObjectRef* ) + ValueStack.size() * sizeof( BObjectRef );
  for ( const auto& bojectref : ValueStack )
  {
    if ( bojectref != nullptr )
      size += bojectref->sizeEstimate();
  }
  size += 3 * sizeof( BObjectRef* ) + fparams.capacity() * sizeof( BObjectRef );
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
  size += 3 * sizeof( ExecutorModule** ) + execmodules.capacity() * sizeof( ExecutorModule* );
  size += 3 * sizeof( ExecutorModule** ) + availmodules.capacity() * sizeof( ExecutorModule* );
  size += 3 * sizeof( unsigned* ) + breakpoints_.size() * sizeof( unsigned );
  size += 3 * sizeof( unsigned* ) + tmpbreakpoints_.size() * sizeof( unsigned );
  size += func_result_ != nullptr ? func_result_->sizeEstimate() : 0;
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
}  // namespace Bscript
}  // namespace Pol
