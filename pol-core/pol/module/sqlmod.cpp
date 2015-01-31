/*
History
=======

Notes
=======

*/
#include "sqlmod.h"

#include "../sqlscrobj.h"
#include "../uoexec.h"
#include "../polsem.h"
#include "osmod.h"
#include "../globals/network.h"

#include "../../bscript/bobject.h"
#include "../../bscript/berror.h"
#include "../../bscript/impstr.h"
#include "../../clib/logfacility.h"


namespace Pol {
  namespace Bscript {
    using namespace Module;
    template<>
    TmplExecutorModule<SQLExecutorModule>::FunctionDef
      TmplExecutorModule<SQLExecutorModule>::function_table[] =
    {
      { "MySQL_Connect", &SQLExecutorModule::mf_ConnectToDB },
      { "MySQL_Query", &SQLExecutorModule::mf_Query },
      { "MySQL_Close", &SQLExecutorModule::mf_Close },
      { "MySQL_Num_Fields", &SQLExecutorModule::mf_NumFields },
      { "MySQL_Fetch_Row", &SQLExecutorModule::mf_FetchRow },
      { "MySQL_Affected_Rows", &SQLExecutorModule::mf_AffectedRows },
      { "MySQL_Num_Rows", &SQLExecutorModule::mf_NumRows },
      { "MySQL_Select_Db", &SQLExecutorModule::mf_SelectDb },
      { "MySQL_Field_Name", &SQLExecutorModule::mf_FieldName }
    };
    template<>
    int TmplExecutorModule<SQLExecutorModule>::function_table_size = arsize( function_table );
  }
  namespace Module {
    using namespace Bscript;
#ifdef HAVE_MYSQL

    BObjectImp* SQLExecutorModule::background_connect( Core::UOExecutor& uoexec, const std::string host, const std::string username, const std::string password )
    {
      auto msg = [&uoexec, host, username, password]()
      {
        if ( &uoexec == nullptr )
        {
          INFO_PRINT << "uoexec is null\n";
          return;
        }
        std::unique_ptr<Core::BSQLConnection> sql;
        {
          Core::PolLock lck;
          sql = std::unique_ptr<Core::BSQLConnection>( new Core::BSQLConnection() );
        }
        if ( sql->getLastErrNo() )
        {
          Core::PolLock lck;
          uoexec.ValueStack.back( ).set( new BObject( new BError( "Insufficient memory" ) ));
          uoexec.os_module->revive( );
        }
        else if ( !sql->connect( host.data(), username.data(), password.data() ) )
        {
          Core::PolLock lck;
          uoexec.ValueStack.back( ).set( new BObject( new BError( sql->getLastError( ) ) ) );
          uoexec.os_module->revive( );
        }
        else
        {
          Core::PolLock lck;
          uoexec.ValueStack.back( ).set( new BObject( sql.release( ) ) );
          uoexec.os_module->revive( );
        }
      };
      
      Core::networkManager.sql_service->push( std::move(msg) );
      uoexec.os_module->suspend();
      return new BLong( 0 );
    }

    BObjectImp* SQLExecutorModule::background_select( Core::UOExecutor& uoexec, Core::BSQLConnection *sql, const std::string db )
    {
      auto msg = [&uoexec, sql, db]()
      {
        if ( &uoexec == nullptr )
        {
          INFO_PRINT << "uoexec is null\n";
          return;
        }
        if ( sql == nullptr )
        {
          Core::PolLock lck;
          uoexec.ValueStack.back( ).set( new BObject( new BError( "Invalid parameters" ) ) );
          uoexec.os_module->revive( );
        }
        else if ( !sql->select_db( db.c_str() ) )
        {
          Core::PolLock lck;
          uoexec.ValueStack.back( ).set( new BObject( new BError( sql->getLastError( ) ) ) );
          uoexec.os_module->revive( );
        }
        else
        {
          Core::PolLock lck;
          uoexec.ValueStack.back( ).set( new BObject( new BLong( 1 ) ) );
          uoexec.os_module->revive();
        }
      };
      Core::networkManager.sql_service->push( std::move( msg ) );
      uoexec.os_module->suspend();
      return new BLong( 0 );
    }

    BObjectImp* SQLExecutorModule::background_query( Core::UOExecutor& uoexec, Core::BSQLConnection *sql, const std::string query )
    {
      auto msg = [&uoexec, sql, query]()
      {
        if ( &uoexec == nullptr )
        {
          INFO_PRINT << "uoexec is null\n";
          return;
        }

        if ( sql == nullptr )
        {
          Core::PolLock lck;
          uoexec.ValueStack.back( ).set( new BObject( new BError( "Invalid parameters" ) ) );
          uoexec.os_module->revive( );
        }
        else if ( !sql->query( query.c_str() ) )
        {
          Core::PolLock lck;
          uoexec.ValueStack.back( ).set( new BObject( new BError( sql->getLastError( ) ) ) );
          uoexec.os_module->revive( );
        }
        else
        {
          Core::PolLock lck;
          uoexec.ValueStack.back( ).set( new BObject( sql->getResultSet( ) ) );
          uoexec.os_module->revive( );
        }
      };
      Core::networkManager.sql_service->push( std::move( msg ) );
      uoexec.os_module->suspend();
      return new BLong( 0 );
    }

	Bscript::BObjectImp* SQLExecutorModule::mf_ConnectToDB()
	{
      const String *host = getStringParam( 0 );
      const String *username = getStringParam( 1 );
      const String *password = getStringParam( 2 );
      if ( !host || !username || !password )
      {
        return new BError( "Invalid parameters" );
      }
      return background_connect( uoexec, host->getStringRep(), username->getStringRep(), password->getStringRep() );
	}
	Bscript::BObjectImp* SQLExecutorModule::mf_SelectDb()
	{
      Core::BSQLConnection *sql = static_cast<Core::BSQLConnection*>( getParamImp( 0, Bscript::BObjectImp::OTSQLConnection ) );
      const String *db = getStringParam( 1 );
      if ( !sql || !db )
      {
        return new BError( "Invalid parameters" );
      }
      return background_select( uoexec, sql, db->getStringRep() );
	}

	Bscript::BObjectImp* SQLExecutorModule::mf_Query()
	{
      Core::BSQLConnection *sql = static_cast<Core::BSQLConnection*>( getParamImp( 0, Bscript::BObjectImp::OTSQLConnection ) );
      const String *query = getStringParam( 1 );
      if ( !sql || !query )
	  {
		return new BError( "Invalid parameters" );
	  }
      return background_query( uoexec, sql, query->getStringRep() );
	}

	Bscript::BObjectImp* SQLExecutorModule::mf_NumFields()
	{
      Core::BSQLResultSet *result = static_cast<Core::BSQLResultSet*>( getParamImp( 0, Bscript::BObjectImp::OTSQLResultSet ));
      if ( !result )
	  {
		return new BError( "Invalid parameters" );
	  }
	  return new BLong( result->num_fields() );
	}

	Bscript::BObjectImp* SQLExecutorModule::mf_FieldName()
	{
      Core::BSQLResultSet *result = static_cast<Core::BSQLResultSet*>( getParamImp( 0, Bscript::BObjectImp::OTSQLResultSet ) );
      int index;
      if (!getParam( 1, index ))
        return new BError( "Invalid parameters" );

      if ( !result || !index )
	  {
		return new BError( "Invalid parameters" );
	  }
	  const char *name = result->field_name( index );
	  if ( name == 0 )
		return new BError( "Column does not exist" );
	  return new String( name );
	}

	Bscript::BObjectImp* SQLExecutorModule::mf_AffectedRows()
	{
      Core::BSQLResultSet *result = static_cast<Core::BSQLResultSet*>( getParamImp( 0, Bscript::BObjectImp::OTSQLResultSet ) );;
      if ( !result )
	  {
		return new BError( "Invalid parameters" );
	  }
	  return new BLong( result->affected_rows() );
	}

	Bscript::BObjectImp* SQLExecutorModule::mf_NumRows()
	{
      Core::BSQLResultSet *result = static_cast<Core::BSQLResultSet*>( getParamImp( 0, Bscript::BObjectImp::OTSQLResultSet ) );;
      if ( !result )
	  {
		return new BError( "Invalid parameters" );
	  }
	  return new BLong( result->num_rows() );
	}

	Bscript::BObjectImp* SQLExecutorModule::mf_Close()
	{
      Core::BSQLConnection *sql = static_cast<Core::BSQLConnection *>( getParamImp( 0, Bscript::BObjectImp::OTSQLConnection ) );
      if ( !sql )
        return new BError( "Invalid parameters" );
	  sql->close();
	  return new BLong( 1 );
	}

	Bscript::BObjectImp* SQLExecutorModule::mf_FetchRow()
	{
      Core::BSQLResultSet *result = static_cast<Core::BSQLResultSet*>( getParamImp( 0, Bscript::BObjectImp::OTSQLResultSet ) );
      if ( !result )
	  {
		return new BError( "Invalid parameters" );
	  }
      return new Core::BSQLRow( result );
	}

#else

#define MF_NO_MYSQL(funcName) BObjectImp* SQLExecutorModule::funcName() { return new BError("POL was not compiled with MySQL support."); }
	MF_NO_MYSQL( mf_ConnectToDB )
	  MF_NO_MYSQL( mf_SelectDb )
	  MF_NO_MYSQL( mf_Query )
	  MF_NO_MYSQL( mf_NumFields )
	  MF_NO_MYSQL( mf_FieldName )
	  MF_NO_MYSQL( mf_AffectedRows )
	  MF_NO_MYSQL( mf_NumRows )
	  MF_NO_MYSQL( mf_Close )
	  MF_NO_MYSQL( mf_FetchRow )
#endif
  }
}