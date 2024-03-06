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
typedef std::shared_ptr<ResultWrapper> RES_WRAPPER;

class BSQLRow final : public Core::PolObjectImp
{
public:
  BSQLRow( RES_WRAPPER resultset );
  BSQLRow( BSQLResultSet* resultset );
  // BSQLRow(MYSQL_RES *_result, MYSQL_ROW _row);
  BSQLRow( RES_WRAPPER _result, MYSQL_ROW _row, MYSQL_FIELD* _fields );
  ~BSQLRow();

  // virtual BObjectRef get_member( const char* membername );
  // virtual BObjectRef get_member_id( const int id ); //id test
  // virtual Bscript::BObjectImp* call_method( const char* methodname, Executor& ex );
  // virtual Bscript::BObjectImp* call_method_id( const int id, Executor& ex, bool
  // forcebuiltin=false );
  virtual Bscript::BObjectImp* copy() const override;
  virtual std::string getStringRep() const override { return "SQLRow"; }
  virtual size_t sizeEstimate() const override { return sizeof( *this ) + sizeof( MYSQL_FIELD ); }
  virtual const char* typeOf() const override { return "SQLRow"; }
  virtual u8 typeOfInt() const override { return OTSQLRow; }
  virtual bool isTrue() const override { return _row != 0; };
  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;

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
  ~BSQLResultSet();
  int num_fields() const;
  int affected_rows() const;
  const char* field_name( unsigned int index ) const;
  int num_rows() const;
  bool has_result() const;
  virtual Bscript::BObjectImp* copy() const override;
  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override { return sizeof( *this ) + sizeof( MYSQL_FIELD ); }
  virtual const char* typeOf() const override { return "SQLResultSet"; }
  virtual u8 typeOfInt() const override { return OTSQLResultSet; }
  virtual bool isTrue() const override;
  // virtual BObjectRef OperSubscript( const BObject& obj );

  friend class BSQLRow;

private:
  RES_WRAPPER _result;
  MYSQL_FIELD* _fields;
  int _affected_rows;
};
class SQLService;

typedef std::vector<std::string> QueryParam;
typedef std::shared_ptr<QueryParam> QueryParams;

class BSQLConnection final : public Core::PolObjectImp
{
  class ConnectionWrapper;

public:
  BSQLConnection();
  BSQLConnection( std::shared_ptr<ConnectionWrapper> conn );
  ~BSQLConnection();
  bool connect( const char* host, const char* user, const char* passwd, int port = 0 );
  bool query( const std::string query );
  bool query( const std::string query, const QueryParams params );
  bool select_db( const char* db );
  bool close();
  Bscript::BObjectImp* getResultSet() const;

  std::string escape_string( const std::string& text ) const;

  std::string getLastError() const;
  int getLastErrNo() const;
  std::shared_ptr<ConnectionWrapper> getConnection() const;

  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  virtual Bscript::BObjectImp* call_polmethod( const char* methodname, UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* call_polmethod_id( const int id, UOExecutor& ex,
                                                  bool forcebuiltin = false ) override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override { return sizeof( *this ) + _error.capacity(); }
  virtual const char* typeOf() const override { return "SQLConnection"; }
  virtual u8 typeOfInt() const override { return OTSQLConnection; }
  virtual bool isTrue() const override;
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
  typedef std::function<void()> msg;
  typedef Clib::message_queue<msg> msg_queue;
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
