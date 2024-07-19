#include "sqlitemod.h"
#include "bscript/executor.h"

#include <module_defs/sqlite.h>

using namespace Pol::Bscript;

namespace Pol::Module
{
SQLiteExecutorModule::SQLiteExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<SQLiteExecutorModule, Bscript::ExecutorModule>( exec )
{
}

BObjectImp* SQLiteExecutorModule::mf_Sqlite3_Test()
{
  return new BLong( 123 );
}
}  // namespace Pol::Module
