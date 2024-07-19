#include "sqlitemod.h"
#include "bscript/berror.h"
#include "bscript/executor.h"
#include "clib/fileutil.h"
#include "clib/logfacility.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

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
  int nb = -1;

  try
  {
    SQLite::Database db( "transaction.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE );

    db.exec( "DROP TABLE IF EXISTS test" );

    // Begin transaction
    SQLite::Transaction transaction( db );

    db.exec( "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)" );

    nb = db.exec( "INSERT INTO test VALUES (NULL, \"test\")" );

    // Commit transaction
    transaction.commit();
  }
  catch ( std::exception& e )
  {
    return new BError( e.what() );
  }
  
  // Delete file... for now :)
  Clib::RemoveFile( "transaction.db3" );
  
  return new BLong( nb );
}
}  // namespace Pol::Module
