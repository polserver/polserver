/** @file
 *
 * @par History
 * - 2009/12/10 Turley:   Method/Syshook definitions now supports :pkg: format
 */


#include "syshookscript.h"

#include <exception>
#include <stddef.h>
#include <string>

#include "../bscript/berror.h"
#include "../bscript/eprog.h"
#include "../bscript/executor.h"
#include "../clib/clib.h"
#include "../clib/logfacility.h"
#include "scrsched.h"


namespace Pol::Core
{
using namespace Bscript;
ExportScript::ExportScript( const Plib::Package* pkg, std::string scriptname )
{
  if ( scriptname.find( ".ecl" ) == std::string::npos )
    scriptname += ".ecl";
  sd.config( scriptname, pkg, "", true );
}

ExportScript::ExportScript( const ScriptDef& isd ) : sd( isd ) {}

bool ExportScript::Initialize()
{
  BObject ob( run_executor_to_completion( uoexec, sd ) );
  return ob.isTrue();
}

const std::string& ExportScript::scriptname() const
{
  return sd.name();
}

bool ExportScript::FindExportedFunction( const std::string& name, unsigned args,
                                         unsigned& PC ) const
{
  const EScriptProgram* prog = uoexec.prog();
  for ( unsigned i = 0; i < prog->exported_functions.size(); ++i )
  {
    const EPExportedFunction* exportedfunc = &prog->exported_functions[i];
    if ( stricmp( exportedfunc->name.c_str(), name.c_str() ) == 0 )
    {
      if ( args != exportedfunc->nargs )
      {
        INFO_PRINTLN( "Exported function {} in script {} takes {} parameters, expected {}", name,
                      scriptname(), exportedfunc->nargs, args );
        return false;
      }
      PC = exportedfunc->PC;
      return true;
    }
  }
  return false;
}

bool ExportScript::FindExportedFunction( const char* name, unsigned args, unsigned& PC ) const
{
  const EScriptProgram* prog = uoexec.prog();
  for ( unsigned i = 0; i < prog->exported_functions.size(); ++i )
  {
    const EPExportedFunction* exportedfunc = &prog->exported_functions[i];
    if ( stricmp( exportedfunc->name.c_str(), name ) == 0 )
    {
      if ( args != exportedfunc->nargs )
      {
        INFO_PRINTLN( "Exported function {} in script {} takes {} parameters, expected {}", name,
                      scriptname(), exportedfunc->nargs, args );
        return false;
      }
      PC = exportedfunc->PC;
      return true;
    }
  }
  return false;
}

bool ExportScript::call( unsigned PC, BObjectImp* p0, BObjectImp* p1, BObjectImp* p2,
                         BObjectImp* p3 )
{
  try
  {
    // build backup if function is called inside the same script
    BackupStruct backup;
    SaveStack( backup );

    uoexec.initForFnCall( PC );

    uoexec.pushArg( p0 );
    uoexec.pushArg( p1 );
    uoexec.pushArg( p2 );
    uoexec.pushArg( p3 );

    uoexec.exec();

    bool istrue = expect_bool();

    // delete current state and reenable backup
    LoadStack( backup );

    return istrue;
  }
  catch ( std::exception& )  //...
  {
    return false;
  }
}

bool ExportScript::call( unsigned PC, BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 )
{
  try
  {
    // build backup if function is called inside the same script
    BackupStruct backup;
    SaveStack( backup );

    uoexec.initForFnCall( PC );

    uoexec.pushArg( p0 );
    uoexec.pushArg( p1 );
    uoexec.pushArg( p2 );

    uoexec.exec();

    bool istrue = expect_bool();

    // delete current state and reenable backup
    LoadStack( backup );

    return istrue;
  }
  catch ( std::exception& )  //...
  {
    return false;
  }
}

bool ExportScript::call( unsigned PC, BObjectImp* p0, BObjectImp* p1 )
{
  try
  {
    // build backup if function is called inside the same script
    BackupStruct backup;
    SaveStack( backup );

    uoexec.initForFnCall( PC );

    uoexec.pushArg( p0 );
    uoexec.pushArg( p1 );

    uoexec.exec();

    bool istrue = expect_bool();

    // delete current state and reenable backup
    LoadStack( backup );

    return istrue;
  }
  catch ( std::exception& )  //...
  {
    return false;
  }
}

bool ExportScript::call( unsigned PC, BObjectImp* p0 )
{
  try
  {
    // build backup if function is called inside the same script
    BackupStruct backup;
    SaveStack( backup );

    uoexec.initForFnCall( PC );

    uoexec.pushArg( p0 );

    uoexec.exec();

    bool istrue = expect_bool();

    // delete current state and reenable backup
    LoadStack( backup );

    return istrue;
  }
  catch ( std::exception& )  //...
  {
    return false;
  }
}

std::string ExportScript::call_string( unsigned PC, BObjectImp* p0, BObjectImp* p1 )
{
  try
  {
    // build backup if function is called inside the same script
    BackupStruct backup;
    SaveStack( backup );

    uoexec.initForFnCall( PC );

    uoexec.pushArg( p0 );
    uoexec.pushArg( p1 );

    uoexec.exec();

    std::string ret = expect_string();

    // delete current state and reenable backup
    LoadStack( backup );

    return ret;
  }
  catch ( std::exception& )  //...
  {
    return "exception";
  }
}

std::string ExportScript::call_string( unsigned PC, BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 )
{
  try
  {
    // build backup if function is called inside the same script
    BackupStruct backup;
    SaveStack( backup );

    uoexec.initForFnCall( PC );

    uoexec.pushArg( p0 );
    uoexec.pushArg( p1 );
    uoexec.pushArg( p2 );

    uoexec.exec();

    std::string ret = expect_string();

    // delete current state and reenable backup
    LoadStack( backup );

    return ret;
  }
  catch ( std::exception& )  //...
  {
    return "exception";
  }
}

int ExportScript::call_long( unsigned PC, BObjectImp* p0 )
{
  try
  {
    // build backup if function is called inside the same script
    BackupStruct backup;
    SaveStack( backup );

    uoexec.initForFnCall( PC );

    uoexec.pushArg( p0 );

    uoexec.exec();

    int ret = expect_int();

    // delete current state and reenable backup
    LoadStack( backup );

    return ret;
  }
  catch ( std::exception& )  //...
  {
    return 0;
  }
}

int ExportScript::call_long( unsigned PC, BObjectImp* p0, BObjectImp* p1 )
{
  try
  {
    // build backup if function is called inside the same script
    BackupStruct backup;
    SaveStack( backup );

    uoexec.initForFnCall( PC );

    uoexec.pushArg( p0 );
    uoexec.pushArg( p1 );

    uoexec.exec();

    int ret = expect_int();

    // delete current state and reenable backup
    LoadStack( backup );

    return ret;
  }
  catch ( std::exception& )  //...
  {
    return 0;
  }
}

BObjectImp* ExportScript::call( unsigned PC, BObjectImp* p0, std::vector<BObjectRef>& pmore )
{
  try
  {
    // build backup if function is called inside the same script
    BackupStruct backup;
    SaveStack( backup );

    uoexec.initForFnCall( PC );

    uoexec.pushArg( p0 );
    size_t n = pmore.size();
    for ( size_t i = 0; i < n; ++i )
    {  // push BObjectRef so params can be pass-by-ref
      uoexec.pushArg( pmore[i] );
    }

    uoexec.exec();

    BObjectImp* ret = expect_imp();

    // delete current state and reenable backup
    LoadStack( backup );

    return ret;
  }
  catch ( std::exception& )  //...
  {
    return new BError( "Exception during execution" );
  }
}

BObject ExportScript::call( unsigned PC, BObjectImp* p0, BObjectImpRefVec& pmore )
{
  try
  {
    // build backup if function is called inside the same script
    BackupStruct backup;
    SaveStack( backup );

    uoexec.initForFnCall( PC );

    uoexec.pushArg( p0 );
    size_t n = pmore.size();
    for ( size_t i = 0; i < n; ++i )
    {
      uoexec.pushArg( pmore[i].get() );
    }

    uoexec.exec();
    BObjectImp* ret = expect_imp();

    // delete current state and reenable backup
    LoadStack( backup );

    return BObject( ret );
  }
  catch ( std::exception& )  //...
  {
    return BObject( new BError( "Exception during execution" ) );
  }
}

BObject ExportScript::call_object( unsigned PC, BObjectImp* p0, BObjectImp* p1 )
{
  try
  {
    // build backup if function is called inside the same script
    BackupStruct backup;
    SaveStack( backup );

    uoexec.initForFnCall( PC );

    uoexec.pushArg( p0 );
    uoexec.pushArg( p1 );

    uoexec.exec();

    BObjectImp* ret = expect_imp();

    // delete current state and reenable backup
    LoadStack( backup );

    return BObject( ret );
  }
  catch ( std::exception& )  //...
  {
    return BObject( new BError( "Exception during execution" ) );
  }
}

BObject ExportScript::call_object( unsigned PC, BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 )
{
  try
  {
    // build backup if function is called inside the same script
    BackupStruct backup;
    SaveStack( backup );

    uoexec.initForFnCall( PC );

    uoexec.pushArg( p0 );
    uoexec.pushArg( p1 );
    uoexec.pushArg( p2 );

    uoexec.exec();
    BObjectImp* ret = expect_imp();

    // delete current state and reenable backup
    LoadStack( backup );

    return BObject( ret );
  }
  catch ( std::exception& )  //...
  {
    return BObject( new BError( "Exception during execution" ) );
  }
}

void ExportScript::SaveStack( BackupStruct& backup )
{
  if ( uoexec.PC != 0 )
  {
    backup.PC = uoexec.PC;
    backup.ValueStack = std::move( uoexec.ValueStack );
    if ( ( uoexec.Locals2 != nullptr ) && ( !uoexec.Locals2->empty() ) )
    {
      backup.Locals.reset( uoexec.Locals2 );
      uoexec.Locals2 = new BObjectRefVec;
    }
  }
  else
    backup.PC = 0;
}

void ExportScript::LoadStack( BackupStruct& backup )
{
  if ( backup.PC != 0 )
  {
    uoexec.initForFnCall( backup.PC );
    uoexec.ValueStack = std::move( backup.ValueStack );
    if ( backup.Locals.get() != nullptr )
    {
      delete uoexec.Locals2;
      uoexec.Locals2 = backup.Locals.release();
    }
  }
}

size_t ExportScript::estimateSize() const
{
  return sd.estimatedSize() + uoexec.sizeEstimate();
}

bool ExportScript::expect_bool()
{
  if ( uoexec.error() || uoexec.ValueStack.empty() )
    return false;
  bool istrue = uoexec.ValueStack.back().get()->isTrue();
  uoexec.ValueStack.pop_back();
  return istrue;
}

int ExportScript::expect_int()
{
  if ( uoexec.error() || uoexec.ValueStack.empty() )
    return 0;
  int ret;
  BObjectImp* imp = uoexec.ValueStack.back().get()->impptr();
  if ( auto* l = impptrIf<BLong>( imp ) )
    ret = l->value();
  else if ( auto* b = impptrIf<BBoolean>( imp ) )
    ret = b->isTrue() ? 1 : 0;
  else
    ret = 0;
  uoexec.ValueStack.pop_back();
  return ret;
}

std::string ExportScript::expect_string()
{
  if ( uoexec.error() || uoexec.ValueStack.empty() )
    return "error";
  auto ret = uoexec.ValueStack.back().get()->impptr()->getStringRep();
  uoexec.ValueStack.pop_back();
  return ret;
}

BObjectImp* ExportScript::expect_imp()
{
  if ( uoexec.error() )
    return new BError( "Error during execution" );
  if ( uoexec.ValueStack.empty() )
    return new BError( "There was no return value??" );

  auto ret = uoexec.ValueStack.back()->impptr()->copy();
  uoexec.ValueStack.pop_back();
  return ret;
}
}  // namespace Pol::Core
