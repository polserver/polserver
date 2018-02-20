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

#ifdef _MSC_VER
#pragma warning( disable : 4996 )  // disable deprecation warning for stricmp
#endif

namespace Pol
{
namespace Core
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
        INFO_PRINT << "Exported function " << name << " in script " << scriptname() << " takes "
                   << exportedfunc->nargs << " parameters, expected " << args << "\n";
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
        INFO_PRINT << "Exported function " << name << " in script " << scriptname() << " takes "
                   << exportedfunc->nargs << " parameters, expected " << args << "\n";
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

    bool istrue;

    if ( uoexec.error() )
      istrue = false;
    else if ( uoexec.ValueStack.empty() )
      istrue = false;
    else
    {
      istrue = uoexec.ValueStack.back().get()->isTrue();
      uoexec.ValueStack.pop_back();
    }

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

    bool istrue;

    if ( uoexec.error() )
      istrue = false;
    else if ( uoexec.ValueStack.empty() )
      istrue = false;
    else
    {
      istrue = uoexec.ValueStack.back().get()->isTrue();
      uoexec.ValueStack.pop_back();
    }

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

    bool istrue;

    if ( uoexec.error() )
      istrue = false;
    else if ( uoexec.ValueStack.empty() )
      istrue = false;
    else
    {
      istrue = uoexec.ValueStack.back().get()->isTrue();
      uoexec.ValueStack.pop_back();
    }

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

    bool istrue;

    if ( uoexec.error() )
      istrue = false;
    else if ( uoexec.ValueStack.empty() )
      istrue = false;
    else
    {
      istrue = uoexec.ValueStack.back().get()->isTrue();
      uoexec.ValueStack.pop_back();
    }

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

    std::string ret;

    if ( uoexec.error() )
      ret = "error";
    else if ( uoexec.ValueStack.empty() )
      ret = "error";
    else
    {
      ret = uoexec.ValueStack.back().get()->impptr()->getStringRep();
      uoexec.ValueStack.pop_back();
    }

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

    std::string ret;

    if ( uoexec.error() )
      ret = "error";
    else if ( uoexec.ValueStack.empty() )
      ret = "error";
    else
    {
      ret = uoexec.ValueStack.back().get()->impptr()->getStringRep();
      uoexec.ValueStack.pop_back();
    }

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

    int ret;

    if ( uoexec.error() )
      ret = 0;
    else if ( uoexec.ValueStack.empty() )
      ret = 0;
    else
    {
      BObjectImp* imp = uoexec.ValueStack.back().get()->impptr();
      if ( imp->isa( BObjectImp::OTLong ) )
      {
        BLong* pLong = static_cast<BLong*>( imp );
        ret = pLong->value();
      }
      else
      {
        ret = 0;
      }
      uoexec.ValueStack.pop_back();
    }

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

    int ret;

    if ( uoexec.error() )
      ret = 0;
    else if ( uoexec.ValueStack.empty() )
      ret = 0;
    else
    {
      BObjectImp* imp = uoexec.ValueStack.back().get()->impptr();
      if ( imp->isa( BObjectImp::OTLong ) )
      {
        BLong* pLong = static_cast<BLong*>( imp );
        ret = pLong->value();
      }
      else
      {
        ret = 0;
      }
      uoexec.ValueStack.pop_back();
    }

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

    BObjectImp* ret;

    if ( uoexec.error() )
      ret = new BError( "Error during execution" );
    else if ( uoexec.ValueStack.empty() )
      ret = new BError( "There was no return value??" );
    else
    {
      ret = uoexec.ValueStack.back()->impptr()->copy();
      uoexec.ValueStack.pop_back();
    }

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
    BObjectImp* ret;

    if ( uoexec.error() )
      ret = new BError( "Error during execution" );
    else if ( uoexec.ValueStack.empty() )
      ret = new BError( "There was no return value??" );
    else
    {
      ret = uoexec.ValueStack.back()->impptr()->copy();
      uoexec.ValueStack.pop_back();
    }

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

    BObjectImp* ret;

    if ( uoexec.error() )
      ret = new BError( "Error during execution" );
    else if ( uoexec.ValueStack.empty() )
      ret = new BError( "There was no return value??" );
    else
    {
      ret = uoexec.ValueStack.back()->impptr()->copy();
      uoexec.ValueStack.pop_back();
    }

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
    BObjectImp* ret;

    if ( uoexec.error() )
      ret = new BError( "Error during execution" );
    else if ( uoexec.ValueStack.empty() )
      ret = new BError( "There was no return value??" );
    else
    {
      ret = uoexec.ValueStack.back()->impptr()->copy();
      uoexec.ValueStack.pop_back();
    }

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
    while ( !uoexec.ValueStack.empty() )
    {
      backup.ValueStack.push_back( uoexec.ValueStack.back() );
      uoexec.ValueStack.pop_back();
    }
    if ( ( uoexec.Locals2 != NULL ) && ( !uoexec.Locals2->empty() ) )
    {
      backup.Locals.reset( new BObjectRefVec );
      backup.Locals->assign( uoexec.Locals2->begin(), uoexec.Locals2->end() );
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
    while ( !backup.ValueStack.empty() )
    {
      uoexec.ValueStack.push_back( backup.ValueStack.back() );
      backup.ValueStack.pop_back();
    }
    if ( backup.Locals.get() != NULL )
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
}
}
