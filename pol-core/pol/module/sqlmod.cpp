/** @file
 *
 * @par History
 */

#include "sqlmod.h"
#include <stddef.h>

#include "bscript/barray.h"
#include "bscript/berror.h"
#include "bscript/blong.h"
#include "bscript/bstring.h"
#include "clib/logfacility.h"
#include "clib/refptr.h"
#include "clib/weakptr.h"

#include "../globals/network.h"
#include "../polsem.h"
#include "../sqlscrobj.h"
#include "../uoexec.h"

#include <module_defs/sql.h>

#include <memory>


namespace Pol::Module
{
using namespace Bscript;

SQLExecutorModule::SQLExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<SQLExecutorModule, Core::PolModule>( exec )
{
}

size_t SQLExecutorModule::sizeEstimate() const
{
  return sizeof( *this );
}

BObjectImp* SQLExecutorModule::background_connect( weak_ptr<Core::UOExecutor> uoexec,
                                                   std::string host, std::string username,
                                                   std::string password, int port )
{
  if ( !uoexec->suspend() )
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tThe execution of this script can't be blocked!",
        uoexec->scriptname(), uoexec->PC );
    return new Bscript::BError( "Script can't be blocked" );
  }
  Core::networkManager.sql_service->push(
      [uoexec = std::move( uoexec ), host = std::move( host ), username = std::move( username ),
       password = std::move( password ), port]()
      {
        std::unique_ptr<Core::BSQLConnection> sql;
        {
          Core::PolLock lck;
          sql = std::make_unique<Core::BSQLConnection>();
        }
        if ( sql->getLastErrNo() )
        {
          Core::PolLock lck;
          if ( !uoexec.exists() )
            INFO_PRINTLN( "Script has been destroyed" );
          else
          {
            uoexec.get_weakptr()->ValueStack.back().set(
                new BObject( new BError( "Insufficient memory" ) ) );
            uoexec.get_weakptr()->revive();
          }
        }
        else if ( !sql->connect( host, username, password, port ) )
        {
          Core::PolLock lck;
          if ( !uoexec.exists() )
            INFO_PRINTLN( "Script has been destroyed" );
          else
          {
            uoexec.get_weakptr()->ValueStack.back().set(
                new BObject( new BError( sql->getLastError() ) ) );
            uoexec.get_weakptr()->revive();
          }
        }
        else
        {
          Core::PolLock lck;
          if ( !uoexec.exists() )
            INFO_PRINTLN( "Script has been destroyed" );
          else
          {
            uoexec.get_weakptr()->ValueStack.back().set( new BObject( sql.release() ) );
            uoexec.get_weakptr()->revive();
          }
        }
      } );

  return new BLong( 0 );
}

Bscript::BObjectImp* SQLExecutorModule::background_select( weak_ptr<Core::UOExecutor> uoexec,
                                                           Core::BSQLConnection* sql,
                                                           std::string db )
{
  // The BSQLConnection shouldn't be destroyed before the lambda runs
  ref_ptr<Core::BSQLConnection> sqlRef( sql );
  if ( !uoexec->suspend() )
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tThe execution of this script can't be blocked!",
        uoexec->scriptname(), uoexec->PC );
    return new Bscript::BError( "Script can't be blocked" );
  }
  Core::networkManager.sql_service->push(
      [uoexec = std::move( uoexec ), sqlRef = std::move( sqlRef ), db = std::move( db )]()
      {
        if ( !sqlRef->select_db( db ) )
        {
          Core::PolLock lck;
          if ( !uoexec.exists() )
            INFO_PRINTLN( "Script has been destroyed" );
          else
          {
            uoexec.get_weakptr()->ValueStack.back().set(
                new BObject( new BError( sqlRef->getLastError() ) ) );
            uoexec.get_weakptr()->revive();
          }
        }
        else
        {
          Core::PolLock lck;
          if ( !uoexec.exists() )
            INFO_PRINTLN( "Script has been destroyed" );
          else
          {
            uoexec.get_weakptr()->ValueStack.back().set( new BObject( new BLong( 1 ) ) );
            uoexec.get_weakptr()->revive();
          }
        }
      } );

  return new BLong( 0 );
}

Bscript::BObjectImp* SQLExecutorModule::background_query( weak_ptr<Core::UOExecutor> uoexec,
                                                          Core::BSQLConnection* sql,
                                                          std::string query,
                                                          const Bscript::ObjArray* params )
{
  // Copy and parse params before they will be deleted by this thread (go out of scope)
  Core::QueryParams sharedParams;
  if ( params != nullptr )
  {
    for ( const auto& ref : params->ref_arr )
    {
      const BObject* obj = ref.get();
      if ( obj != nullptr )
        sharedParams.push_back( obj->impptr()->getStringRep() );
    }
  }

  // The BSQLConnection shouldn't be destroyed before the lambda runs
  ref_ptr<Core::BSQLConnection> sqlRef( sql );
  if ( !uoexec->suspend() )
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tThe execution of this script can't be blocked!",
        uoexec->scriptname(), uoexec->PC );
    return new Bscript::BError( "Script can't be blocked" );
  }

  Core::networkManager.sql_service->push(
      [uoexec = std::move( uoexec ), sqlRef = std::move( sqlRef ), query = std::move( query ),
       sharedParams = std::move( sharedParams )]()
      {
        if ( !sqlRef->query( query, sharedParams ) )
        {
          Core::PolLock lck;
          if ( !uoexec.exists() )
            INFO_PRINTLN( "Script has been destroyed" );
          else
          {
            uoexec.get_weakptr()->ValueStack.back().set(
                new BObject( new BError( sqlRef->getLastError() ) ) );
            uoexec.get_weakptr()->revive();
          }
        }
        else
        {
          Core::PolLock lck;
          if ( !uoexec.exists() )
            INFO_PRINTLN( "Script has been destroyed" );
          else
          {
            uoexec.get_weakptr()->ValueStack.back().set( new BObject( sqlRef->getResultSet() ) );
            uoexec.get_weakptr()->revive();
          }
        }
      } );

  return new BLong( 0 );
}

Bscript::BObjectImp* SQLExecutorModule::mf_mysql_connect()
{
  const String* host = getStringParam( 0 );
  const String* username = getStringParam( 1 );
  const String* password = getStringParam( 2 );
  int port;
  if ( !host || !username || !password || !getParam( 3, port ) )
  {
    return new BError( "Invalid parameters" );
  }
  return background_connect( uoexec().weakptr, host->getStringRep(), username->getStringRep(),
                             password->getStringRep(), port );
}
Bscript::BObjectImp* SQLExecutorModule::mf_mysql_select_db()
{
  Core::BSQLConnection* sql =
      static_cast<Core::BSQLConnection*>( getParamImp( 0, Bscript::BObjectImp::OTSQLConnection ) );
  const String* db = getStringParam( 1 );
  if ( !sql || !db )
  {
    return new BError( "Invalid parameters" );
  }
  return background_select( uoexec().weakptr, sql, db->getStringRep() );
}

Bscript::BObjectImp* SQLExecutorModule::mf_mysql_query()
{
  Core::BSQLConnection* sql =
      static_cast<Core::BSQLConnection*>( getParamImp( 0, Bscript::BObjectImp::OTSQLConnection ) );
  const String* query = getStringParam( 1 );
  ObjArray* params;
  bool use_parameters = getObjArrayParam( 2, params );
  if ( !sql || !query )
  {
    return new BError( "Invalid parameters" );
  }

  return background_query( uoexec().weakptr, sql, query->getStringRep(),
                           use_parameters ? params : nullptr );
}

Bscript::BObjectImp* SQLExecutorModule::mf_mysql_num_fields()
{
  Core::BSQLResultSet* result =
      static_cast<Core::BSQLResultSet*>( getParamImp( 0, Bscript::BObjectImp::OTSQLResultSet ) );
  if ( !result )
  {
    return new BError( "Invalid parameters" );
  }
  return new BLong( result->num_fields() );
}

Bscript::BObjectImp* SQLExecutorModule::mf_mysql_field_name()
{
  Core::BSQLResultSet* result =
      static_cast<Core::BSQLResultSet*>( getParamImp( 0, Bscript::BObjectImp::OTSQLResultSet ) );
  int index;
  if ( !getParam( 1, index ) )
    return new BError( "Invalid parameters" );

  if ( !result || !index )
  {
    return new BError( "Invalid parameters" );
  }
  const char* name = result->field_name( index );
  if ( name == nullptr )
    return new BError( "Column does not exist" );
  return new String( name, String::Tainted::YES );
}

Bscript::BObjectImp* SQLExecutorModule::mf_mysql_affected_rows()
{
  Core::BSQLResultSet* result =
      static_cast<Core::BSQLResultSet*>( getParamImp( 0, Bscript::BObjectImp::OTSQLResultSet ) );
  if ( !result )
  {
    return new BError( "Invalid parameters" );
  }
  return new BLong( result->affected_rows() );
}

Bscript::BObjectImp* SQLExecutorModule::mf_mysql_num_rows()
{
  Core::BSQLResultSet* result =
      static_cast<Core::BSQLResultSet*>( getParamImp( 0, Bscript::BObjectImp::OTSQLResultSet ) );
  if ( !result )
  {
    return new BError( "Invalid parameters" );
  }
  return new BLong( result->num_rows() );
}

Bscript::BObjectImp* SQLExecutorModule::mf_mysql_close()
{
  Core::BSQLConnection* sql =
      static_cast<Core::BSQLConnection*>( getParamImp( 0, Bscript::BObjectImp::OTSQLConnection ) );
  if ( !sql )
    return new BError( "Invalid parameters" );
  sql->close();
  return new BLong( 1 );
}

Bscript::BObjectImp* SQLExecutorModule::mf_mysql_fetch_row()
{
  Core::BSQLResultSet* result =
      static_cast<Core::BSQLResultSet*>( getParamImp( 0, Bscript::BObjectImp::OTSQLResultSet ) );
  if ( !result )
  {
    return new BError( "Invalid parameters" );
  }
  if ( !result->has_result() )
  {
    return new BError( "Query returned no result" );
  }
  return new Core::BSQLRow( result );
}

Bscript::BObjectImp* SQLExecutorModule::mf_mysql_escape_string()
{
  Core::BSQLConnection* sql =
      static_cast<Core::BSQLConnection*>( getParamImp( 0, Bscript::BObjectImp::OTSQLConnection ) );
  const String* text;
  if ( !sql || !getStringParam( 1, text ) )
    return new BError( "Invalid parameters" );

  std::string escaped;
  if ( !sql->escape_string( text->value(), &escaped ) )
    return new BError( "failed to escape string" );
  return new String( escaped );
}
}  // namespace Pol::Module
