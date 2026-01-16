/** @file
 *
 * @par History
 */


#ifndef SQLSCROBJ_H
#define SQLSCROBJ_H

#include "pol_global_config.h"

#ifdef HAVE_MYSQL

#ifdef _WIN32
#include "../clib/Header_Windows.h"
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

#include <functional>
#include <memory>
#include <string>

#include "../bscript/bobject.h"
#include "../clib/message_queue.h"
#include "../clib/rawtypes.h"
#include "polobject.h"

namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
}  // namespace Pol
namespace Pol
{
namespace Core
{
class BSQLResultSet;

class ResultWrapper
{
public:
  ResultWrapper();
  explicit ResultWrapper( MYSQL_RES* res );
  ~ResultWrapper();
  void set( MYSQL_RES* result );
  MYSQL_RES* ptr();

private:
  MYSQL_RES* _result;
};
using RES_WRAPPER = std::shared_ptr<ResultWrapper>;

class BSQLRow final : public Core::PolObjectImp
{
public:
  BSQLRow( RES_WRAPPER resultset );
  BSQLRow( BSQLResultSet* resultset );
  // BSQLRow(MYSQL_RES *_result, MYSQL_ROW _row);
  BSQLRow( RES_WRAPPER _result, MYSQL_ROW _row, MYSQL_FIELD* _fields );
  ~BSQLRow() override;

  // virtual BObjectRef get_member( const char* membername );
  // virtual BObjectRef get_member_id( const int id ); //id test
  // virtual Bscript::BObjectImp* call_method( const char* methodname, Executor& ex );
  // virtual Bscript::BObjectImp* call_method_id( const int id, Executor& ex, bool
  // forcebuiltin=false );
  Bscript::BObjectImp* copy() const override;
  std::string getStringRep() const override { return "SQLRow"; }
  size_t sizeEstimate() const override { return sizeof( *this ) + sizeof( MYSQL_FIELD ); }
  const char* typeOf() const override { return "SQLRow"; }
  u8 typeOfInt() const override { return OTSQLRow; }
  bool isTrue() const override { return _row != 0; };
  Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;
  Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override;
  friend class SQLRowIterator;

private:
  MYSQL_ROW _row;
  RES_WRAPPER _result;
  MYSQL_FIELD* _fields;
};

class BSQLResultSet final : public Bscript::BObjectImp
{
public:
  BSQLResultSet( RES_WRAPPER result );
  BSQLResultSet( RES_WRAPPER result, MYSQL_FIELD* fields );
  BSQLResultSet( int affected_rows );
  ~BSQLResultSet() override;
  int num_fields() const;
  int affected_rows() const;
  const char* field_name( unsigned int index ) const;
  int num_rows() const;
  bool has_result() const;
  Bscript::BObjectImp* copy() const override;
  std::string getStringRep() const override;
  size_t sizeEstimate() const override { return sizeof( *this ) + sizeof( MYSQL_FIELD ); }
  const char* typeOf() const override { return "SQLResultSet"; }
  u8 typeOfInt() const override { return OTSQLResultSet; }
  bool isTrue() const override;
  Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override;

  friend class BSQLRow;
  friend class SQLResultSetIterator;

private:
  RES_WRAPPER _result;
  MYSQL_FIELD* _fields;
  int _affected_rows;
};
class SQLService;

using QueryParam = std::vector<std::string>;
using QueryParams = std::shared_ptr<QueryParam>;

class BSQLConnection final : public Core::PolObjectImp
{
  class ConnectionWrapper;

public:
  BSQLConnection();
  BSQLConnection( std::shared_ptr<ConnectionWrapper> conn );
  ~BSQLConnection() override;
  bool connect( const char* host, const char* user, const char* passwd, int port = 0 );
  bool query( const std::string query );
  bool query( const std::string query, const QueryParams params );
  bool select_db( const char* db );
  bool close();
  Bscript::BObjectImp* getResultSet() const;

  bool escape_string( const std::string& text, std::string* escaped ) const;

  std::string getLastError() const;
  int getLastErrNo() const;
  std::shared_ptr<ConnectionWrapper> getConnection() const;

  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  Bscript::BObjectImp* call_polmethod( const char* methodname, UOExecutor& ex ) override;
  Bscript::BObjectImp* call_polmethod_id( const int id, UOExecutor& ex,
                                          bool forcebuiltin = false ) override;
  Bscript::BObjectImp* copy() const override;
  std::string getStringRep() const override;
  size_t sizeEstimate() const override { return sizeof( *this ) + _error.capacity(); }
  const char* typeOf() const override { return "SQLConnection"; }
  u8 typeOfInt() const override { return OTSQLConnection; }
  bool isTrue() const override;
  // virtual BObjectRef OperSubscript( const BObject& obj );

private:
  std::shared_ptr<ConnectionWrapper> _conn;
  int _errno;
  std::string _error;

  class ConnectionWrapper
  {
  public:
    ConnectionWrapper();
    ~ConnectionWrapper();
    void set( MYSQL* conn );
    MYSQL* ptr();

  private:
    MYSQL* _conn;
  };
};

class SQLService
{
public:
  using msg = std::function<void()>;
  using msg_queue = Clib::message_queue<msg>;
  SQLService();
  ~SQLService();
  void start();
  void stop();
  void push( msg&& msg_ );

private:
  msg_queue _msgs;
};
void start_sql_service();
}  // namespace Core
}  // namespace Pol

#endif

#endif
