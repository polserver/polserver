/** @file
 *
 * @par History
 */


#ifndef SQLSCROBJ_H
#define SQLSCROBJ_H

#ifdef WINDOWS
#include "../clib/pol_global_config_win.h"
#else
#include "pol_global_config.h"
#endif

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
#include "../clib/compilerspecifics.h"
#include "../clib/message_queue.h"
#include "../clib/rawtypes.h"

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

class BSQLRow : public Bscript::BObjectImp
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
  virtual Bscript::BObjectImp* copy() const POL_OVERRIDE;
  virtual std::string getStringRep() const POL_OVERRIDE { return "SQLRow"; }
  virtual size_t sizeEstimate() const POL_OVERRIDE
  {
    return sizeof( *this ) + sizeof( MYSQL_FIELD );
  }
  virtual const char* typeOf() const POL_OVERRIDE { return "SQLRow"; }
  virtual u8 typeOfInt() const POL_OVERRIDE { return OTSQLRow; }
  virtual bool isTrue() const POL_OVERRIDE { return _row != 0; };
  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) POL_OVERRIDE;

private:
  MYSQL_ROW _row;
  RES_WRAPPER _result;
  MYSQL_FIELD* _fields;
};

class BSQLResultSet : public Bscript::BObjectImp
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
  virtual Bscript::BObjectImp* copy() const POL_OVERRIDE;
  virtual std::string getStringRep() const POL_OVERRIDE;
  virtual size_t sizeEstimate() const POL_OVERRIDE
  {
    return sizeof( *this ) + sizeof( MYSQL_FIELD );
  }
  virtual const char* typeOf() const POL_OVERRIDE { return "SQLResultSet"; }
  virtual u8 typeOfInt() const POL_OVERRIDE { return OTSQLResultSet; }
  virtual bool isTrue() const POL_OVERRIDE;
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

class BSQLConnection : public Bscript::BObjectImp
{
  class ConnectionWrapper;

public:
  BSQLConnection();
  BSQLConnection( std::shared_ptr<ConnectionWrapper> conn );
  ~BSQLConnection();
  bool connect( const char* host, const char* user, const char* passwd );
  bool query( const std::string query );
  bool query( const std::string query, const QueryParams params );
  bool select_db( const char* db );
  bool close();
  Bscript::BObjectImp* getResultSet() const;

  std::string getLastError() const;
  int getLastErrNo() const;
  std::shared_ptr<ConnectionWrapper> getConnection() const;

  virtual Bscript::BObjectRef get_member( const char* membername ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member_id( const int id ) POL_OVERRIDE;  // id test
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* copy() const POL_OVERRIDE;
  virtual std::string getStringRep() const POL_OVERRIDE;
  virtual size_t sizeEstimate() const POL_OVERRIDE { return sizeof( *this ) + _error.capacity(); }
  virtual const char* typeOf() const POL_OVERRIDE { return "SQLConnection"; }
  virtual u8 typeOfInt() const POL_OVERRIDE { return OTSQLConnection; }
  virtual bool isTrue() const POL_OVERRIDE;
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
}
}

#endif

#endif
