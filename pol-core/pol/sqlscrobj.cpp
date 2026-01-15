/** @file
 *
 * @par History
 */


#include "sqlscrobj.h"

#include <exception>
#include <regex>
#include <string.h>

#include "../bscript/bobject.h"


#ifdef HAVE_MYSQL

#include "../bscript/berror.h"
#include "../bscript/contiter.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"
#include "../clib/esignal.h"
#include "../clib/logfacility.h"
#include "../clib/threadhelp.h"
#include "globals/network.h"

namespace Pol
{
namespace Core
{
using namespace Bscript;

BSQLRow::BSQLRow( BSQLResultSet* resultset ) : PolObjectImp( OTSQLRow )
{
  _result = resultset->_result;
  _row = mysql_fetch_row( _result->ptr() );
  _fields = mysql_fetch_fields( _result->ptr() );
}
BSQLRow::BSQLRow( RES_WRAPPER result ) : PolObjectImp( OTSQLRow )
{
  _result = result;
  _row = mysql_fetch_row( _result->ptr() );
  _fields = mysql_fetch_fields( _result->ptr() );
}
BSQLRow::BSQLRow( RES_WRAPPER result, MYSQL_ROW row, MYSQL_FIELD* fields )
    : PolObjectImp( OTSQLRow ), _row( row ), _result( result ), _fields( fields )
{
}

class SQLRowIterator final : public Bscript::ContIterator
{
public:
  SQLRowIterator( BSQLRow* node, Bscript::BObject* pIter );
  Bscript::BObject* step() override;

private:
  Bscript::BObject m_RowObj;
  BSQLRow* rowimp;
  Bscript::BObjectRef m_IterVal;
  unsigned int index;
};

SQLRowIterator::SQLRowIterator( BSQLRow* row, Bscript::BObject* pIter )
    : ContIterator(), m_RowObj( row ), rowimp( row ), m_IterVal( pIter ), index( 0 )
{
}

Bscript::BObject* SQLRowIterator::step()
{
  unsigned int num_fields = mysql_num_fields( rowimp->_result->ptr() );
  const auto& fields = rowimp->_fields;
  const auto& row = rowimp->_row;

  if ( index >= num_fields )
    return nullptr;

  m_IterVal->setimp( new String( fields[index].name ) );

  if ( IS_NUM( fields[index].type ) && fields[index].type != MYSQL_TYPE_TIMESTAMP )
  {
    if ( fields[index].type == MYSQL_TYPE_DECIMAL || fields[index].type == MYSQL_TYPE_NEWDECIMAL ||
         fields[index].type == MYSQL_TYPE_FLOAT || fields[index].type == MYSQL_TYPE_DOUBLE )
      return new BObject( new Double( strtod( row[index++], nullptr ) ) );
    return new BObject( new BLong( strtoul( row[index++], nullptr, 0 ) ) );
  }

  return new BObject( new String( row[index++], String::Tainted::YES ) );
}

ContIterator* BSQLRow::createIterator( Bscript::BObject* pIterVal )
{
  if ( !_result )
  {
    return BObjectImp::createIterator( pIterVal );
  }
  return new SQLRowIterator( this, pIterVal );
}

BObjectRef BSQLRow::OperSubscript( const BObject& obj )
{
  const Bscript::BObjectImp& right = obj.impref();
  if ( _result == nullptr )
    return BObjectRef( new BError( "No result" ) );
  unsigned int num_fields = mysql_num_fields( _result->ptr() );
  if ( right.isa( OTLong ) )  // vector
  {
    BLong& lng = (BLong&)right;

    unsigned index = (unsigned)lng.value();
    if ( index > num_fields || index <= 0 )
    {
      return BObjectRef( new BError( "Index out of bounds" ) );
    }
    else if ( _row[index - 1] == nullptr )
    {
      return BObjectRef( UninitObject::create() );
    }
    else if ( IS_NUM( _fields[index - 1].type ) && _fields[index - 1].type != MYSQL_TYPE_TIMESTAMP )
    {
      if ( _fields[index - 1].type == MYSQL_TYPE_DECIMAL ||
           _fields[index - 1].type == MYSQL_TYPE_NEWDECIMAL ||
           _fields[index - 1].type == MYSQL_TYPE_FLOAT ||
           _fields[index - 1].type == MYSQL_TYPE_DOUBLE )
        return BObjectRef( new Double( strtod( _row[index - 1], nullptr ) ) );
      return BObjectRef( new BLong( strtoul( _row[index - 1], nullptr, 0 ) ) );
    }
    return BObjectRef( new String( _row[index - 1], String::Tainted::YES ) );
  }
  else if ( right.isa( OTString ) )
  {
    String& string = (String&)right;
    for ( unsigned int i = 0; i < num_fields; i++ )
    {
      if ( !strncmp( _fields[i].name, string.data(), _fields[i].name_length ) )
      {
        if ( _row[i] == nullptr )
        {
          return BObjectRef( UninitObject::create() );
        }
        else if ( IS_NUM( _fields[i].type ) && _fields[i].type != MYSQL_TYPE_TIMESTAMP )
        {
          if ( _fields[i].type == MYSQL_TYPE_DECIMAL || _fields[i].type == MYSQL_TYPE_NEWDECIMAL ||
               _fields[i].type == MYSQL_TYPE_FLOAT || _fields[i].type == MYSQL_TYPE_DOUBLE )
            return BObjectRef( new Double( strtod( _row[i], nullptr ) ) );
          return BObjectRef( new BLong( strtoul( _row[i], nullptr, 0 ) ) );
        }
        return BObjectRef( new String( _row[i], String::Tainted::YES ) );
      }
    }
    return BObjectRef( new BError( "Column does not exist" ) );
  }
  else
  {
    return BObjectRef( new BError( "SQLRow keys must be integer" ) );
  }
}
BSQLRow::~BSQLRow() = default;
Bscript::BObjectImp* BSQLRow::copy() const
{
  return new BSQLRow( _result, _row, _fields );
}

BSQLResultSet::BSQLResultSet( RES_WRAPPER result )
    : Bscript::BObjectImp( OTSQLResultSet ),
      _result( result ),
      _fields( nullptr ),
      _affected_rows( 0 )
{
  if ( result->ptr() != nullptr )
    _fields = mysql_fetch_fields( result->ptr() );
}
BSQLResultSet::BSQLResultSet( RES_WRAPPER result, MYSQL_FIELD* fields )
    : Bscript::BObjectImp( OTSQLResultSet ),
      _result( result ),
      _fields( fields ),
      _affected_rows( 0 )
{
}
BSQLResultSet::BSQLResultSet( int affected_rows )
    : Bscript::BObjectImp( OTSQLResultSet ),
      _result( nullptr ),
      _fields( nullptr ),
      _affected_rows( affected_rows )
{
}

class SQLResultSetIterator final : public Bscript::ContIterator
{
public:
  SQLResultSetIterator( BSQLResultSet* node, Bscript::BObject* pIter );
  Bscript::BObject* step() override;

private:
  Bscript::BObject m_ResultsObj;
  BSQLResultSet* results;
  Bscript::BObjectRef m_IterVal;
  BLong* m_pIterVal;
};

SQLResultSetIterator::SQLResultSetIterator( BSQLResultSet* results, Bscript::BObject* pIterVal )
    : ContIterator(),
      m_ResultsObj( results ),
      results( results ),
      m_IterVal( pIterVal ),
      m_pIterVal( new BLong( 0 ) )
{
  m_IterVal.get()->setimp( m_pIterVal );
}

Bscript::BObject* SQLResultSetIterator::step()
{
  if ( static_cast<uint64_t>( m_pIterVal->value() ) >= mysql_num_rows( results->_result->ptr() ) )
    return nullptr;

  m_pIterVal->increment();
  return new BObject( new BSQLRow( results ) );
}

ContIterator* BSQLResultSet::createIterator( Bscript::BObject* pIterVal )
{
  if ( !_result )
  {
    return BObjectImp::createIterator( pIterVal );
  }
  return new SQLResultSetIterator( this, pIterVal );
}

const char* BSQLResultSet::field_name( unsigned int index ) const
{
  if ( !_result || _result->ptr() == nullptr )
    return nullptr;
  if ( index <= 0 || index > mysql_num_fields( _result->ptr() ) )
  {
    return nullptr;
  }
  return _fields[index - 1].name;
}
int BSQLResultSet::num_rows() const
{
  if ( !_result )
    return 0;
  return static_cast<int>( mysql_num_rows( _result->ptr() ) );
};

bool BSQLResultSet::has_result() const
{
  return _result && _result->ptr();
}

Bscript::BObjectImp* BSQLResultSet::copy() const
{
  if ( _affected_rows )
    return new BSQLResultSet( _affected_rows );
  else
    return new BSQLResultSet( _result, _fields );
};

int BSQLResultSet::num_fields() const
{
  if ( _result && _result->ptr() != nullptr )
    return mysql_num_fields( _result->ptr() );
  return 0;
}
int BSQLResultSet::affected_rows() const
{
  return _affected_rows;
}
BSQLResultSet::~BSQLResultSet() = default;
bool BSQLResultSet::isTrue() const
{
  return true;
}
std::string BSQLResultSet::getStringRep() const
{
  return "SQLResultSet";
}

bool BSQLConnection::close()
{
  _conn->set( nullptr );
  return true;
}
Bscript::BObjectImp* BSQLConnection::getResultSet() const
{
  if ( _errno )
    return new BError( _error );
  RES_WRAPPER result = std::make_shared<ResultWrapper>( mysql_store_result( _conn->ptr() ) );
  if ( result && result->ptr() != nullptr )  // there are rows
  {
    return new BSQLResultSet( result );
    // retrieve rows, then call mysql_free_result(result)
  }
  else  // mysql_store_result() returned nothing; should it have?
  {
    /*  if (mysql_errno(_conn))
        {
          _error = mysql_error(_conn);
          _errno = mysql_errno(_conn);
          return new BError(_error);
        }
        else */
    if ( mysql_field_count( _conn->ptr() ) == 0 )
    {
      return new BSQLResultSet( static_cast<int>( mysql_affected_rows( _conn->ptr() ) ) );
    }
  }
  return new BError( "Unknown error getting ResultSet" );
}
BSQLConnection::BSQLConnection()
    : PolObjectImp( OTSQLConnection ), _conn( new ConnectionWrapper ), _errno( 0 )
{
  _conn->set( mysql_init( nullptr ) );
  if ( !_conn->ptr() )
  {
    _error = "Insufficient memory";
    _errno = 1;
  }
}

BSQLConnection::BSQLConnection( std::shared_ptr<ConnectionWrapper> conn )
    : PolObjectImp( OTSQLConnection ), _conn( conn ), _errno( 0 )
{
}

BSQLConnection::~BSQLConnection() = default;
std::string BSQLConnection::getStringRep() const
{
  return "SQLConnection";
}
bool BSQLConnection::isTrue() const
{
  if ( !_conn->ptr() )
    return false;  // closed by hand
  if ( !mysql_ping( _conn->ptr() ) )
    return true;
  return false;
}
bool BSQLConnection::connect( const char* host, const char* user, const char* passwd, int port )
{
  if ( !_conn->ptr() )
  {
    _errno = -1;
    _error = "No active MYSQL object instance.";
    return false;
  }
  // port == 0 means default sql port
  if ( !mysql_real_connect( _conn->ptr(), host, user, passwd, nullptr, port, nullptr, 0 ) )
  {
    _errno = mysql_errno( _conn->ptr() );
    _error = mysql_error( _conn->ptr() );
    return false;
  }
  return true;
}
bool BSQLConnection::select_db( const char* db )
{
  if ( !_conn->ptr() )
  {
    _errno = -1;
    _error = "No active MYSQL object instance.";
    return false;
  }
  else if ( mysql_select_db( _conn->ptr(), db ) )
  {
    _errno = mysql_errno( _conn->ptr() );
    _error = mysql_error( _conn->ptr() );
    return false;
  }
  return true;
}

bool BSQLConnection::query( const std::string query )
{
  if ( !_conn->ptr() )
  {
    _errno = -1;
    _error = "No active MYSQL object instance.";
    return false;
  }

  if ( mysql_query( _conn->ptr(), query.c_str() ) )
  {
    _errno = mysql_errno( _conn->ptr() );
    _error = mysql_error( _conn->ptr() );
    return false;
  }

  return true;
}

/*
 * Allows binding parameters to the query
 * Every occurrence of "?" is replaced with a single parameter
 */
bool BSQLConnection::query( const std::string query, QueryParams params )
{
  if ( params == nullptr || !params->size() )
    return this->query( query );

  if ( !_conn->ptr() )
  {
    _errno = -1;
    _error = "No active MYSQL object instance.";
    return false;
  }

  std::string replaced = query;
  std::regex re( "^((?:[^']|'[^']*')*?)(\\?)" );
  for ( auto it = params->begin(); it != params->end(); ++it )
  {
    if ( !std::regex_search( replaced, re ) )
    {
      _errno = -2;
      _error = "Could not replace parameters.";
      return false;
    }

    if ( it->size() > ( std::numeric_limits<size_t>::max() - 5 ) / 2 )
    {
      _errno = -3;
      _error = "Parameter is too long.";
    }

    // Escape the string and add quoting. A bit tricky, but effective.
    size_t escaped_max_size =
        it->size() * 2 + 5;  // max is +1, using +5 to leave space for quoting and "$1"
    std::unique_ptr<char[]> escptr(
        new char[escaped_max_size] );  // will contain the escaped string
    // use +3 to leave space for quoting
    unsigned long esclen = mysql_real_escape_string( _conn->ptr(), escptr.get() + 3, it->c_str(),
                                                     static_cast<unsigned long>( it->size() ) );
    // Now add quoting, equivalent to escptr = "$1'" + escptr + "'"
    esclen += 4;
    escptr[0] = '$';
    escptr[1] = '1';
    escptr[2] = '\'';
    escptr[esclen - 1] = '\'';
    escptr[esclen] = '\0';

    replaced =
        std::regex_replace( replaced, re, escptr.get(), std::regex_constants::format_first_only );
  }

  return this->query( replaced );
}

bool BSQLConnection::escape_string( const std::string& text, std::string* escaped ) const
{
  if ( !_conn->ptr() )
    return false;
  *escaped = std::string( text.size() * 2 + 1, '\0' );
  if ( mysql_real_escape_string( _conn->ptr(), escaped->data(), text.data(),
                                 (unsigned long)text.size() ) == (unsigned long)-1 )
    return false;
  escaped->resize( escaped->find_first_of( '\0' ) );
  return true;
}

std::string BSQLConnection::getLastError() const
{
  return _error;
}
int BSQLConnection::getLastErrNo() const
{
  return _errno;
}
std::shared_ptr<BSQLConnection::ConnectionWrapper> BSQLConnection::getConnection() const
{
  return _conn;
}


BObjectRef BSQLConnection::get_member_id( const int /*id*/ )  // id test
{
  return BObjectRef( UninitObject::create() );
  // switch(id)
  //{

  //  default: return BObjectRef(UninitObject::create());
  //}
}
BObjectRef BSQLConnection::get_member( const char* membername )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  else
    return BObjectRef( UninitObject::create() );
}

Bscript::BObjectImp* BSQLConnection::call_polmethod( const char* methodname, UOExecutor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_polmethod_id( objmethod->id, ex );
  else
    return nullptr;
}

Bscript::BObjectImp* BSQLConnection::call_polmethod_id( const int /*id*/, UOExecutor& /*ex*/,
                                                        bool /*forcebuiltin*/ )
{
  return new BLong( 0 );
}

Bscript::BObjectImp* BSQLConnection::copy() const
{
  return new BSQLConnection( _conn );
}

BSQLConnection::ConnectionWrapper::ConnectionWrapper() : _conn( nullptr ) {}
BSQLConnection::ConnectionWrapper::~ConnectionWrapper()
{
  if ( _conn )
    mysql_close( _conn );
  _conn = nullptr;
}
void BSQLConnection::ConnectionWrapper::set( MYSQL* conn )
{
  if ( _conn )
    mysql_close( _conn );
  _conn = conn;
}
MYSQL* BSQLConnection::ConnectionWrapper::ptr()
{
  return _conn;
};

ResultWrapper::ResultWrapper( MYSQL_RES* res ) : _result( res ) {}
ResultWrapper::ResultWrapper() : _result( nullptr ) {}
ResultWrapper::~ResultWrapper()
{
  if ( _result )
    mysql_free_result( _result );
  _result = nullptr;
}
void ResultWrapper::set( MYSQL_RES* result )
{
  if ( _result )
    mysql_free_result( _result );
  _result = result;
}
MYSQL_RES* ResultWrapper::ptr()
{
  return _result;
}


void sql_service_thread_stub()
{
  try
  {
    networkManager.sql_service->start();
  }
  catch ( const char* msg )
  {
    POLLOGLN( "SQL Thread exits due to exception: {}", msg );
    throw;
  }
  catch ( std::string& str )
  {
    POLLOGLN( "SQL Thread exits due to exception: {}", str );
    throw;
  }
  catch ( std::exception& ex )
  {
    POLLOGLN( "SQL Thread exits due to exception: {}", ex.what() );
    throw;
  }
}

SQLService::SQLService() = default;
SQLService::~SQLService() = default;
void SQLService::stop()
{
  _msgs.cancel();
}
void SQLService::push( msg&& msg_ )
{
  _msgs.push_move( std::move( msg_ ) );
}
void SQLService::start()  // executed inside a extra thread
{
  while ( !Clib::exit_signalled )
  {
    try
    {
      msg task;
      _msgs.pop_wait( &task );
      task();
    }
    catch ( msg_queue::Canceled& )
    {
      break;
    }
    // ignore remaining tasks
  }
}


void start_sql_service()
{
  threadhelp::start_thread( sql_service_thread_stub, "SQLService" );
}
}  // namespace Core
}  // namespace Pol
#endif
