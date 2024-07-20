#include "sqlitemod.h"

#include "../sqlscrobj.h"
#include "bscript/berror.h"
#include "bscript/executor.h"
#include "bscript/impstr.h"
#include "clib/logfacility.h"
#include "sqlmod.h"
#include "sqlmod.inl.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

#include <module_defs/sqlite.h>

using namespace Pol::Bscript;

namespace Pol::Module
{
SQLiteExecutorModule::SQLiteExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<SQLiteExecutorModule, Core::PolModule>( exec )
{
}

BObjectImp* SQLiteExecutorModule::mf_sqlite_open()
{
  const String* filename = getStringParam( 0 );
  if ( !filename )
  {
    return new BError( "Invalid parameters" );
  }

  auto connector = [filename = filename->getStringRep()]( Core::BSQLiteDatabase* conn )
  { return conn->open( filename ); };

  return SQLExecutorModule::background_connect<Core::BSQLiteDatabase>( uoexec().weakptr,
                                                                       connector );
}
}  // namespace Pol::Module
