/*
History
=======


Notes
=======

*/

#ifdef HAVE_MYSQL

#ifndef SQLSCROBJ_H
#define SQLSCROBJ_H


#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

#include "../bscript/contiter.h"
#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <winsock.h>
#	include <mysql.h>
#else
#	include <mysql/mysql.h>
#endif

#include "../clib/message_queue.h"

#include <memory>

namespace Pol {
  namespace Core {
    class BSQLResultSet;

    class ResultWrapper
    {
    public:
      ResultWrapper();
      explicit ResultWrapper( MYSQL_RES* res);
      ~ResultWrapper();
      void set( MYSQL_RES* result );
      MYSQL_RES* ptr() { return _result; }
    private:
      MYSQL_RES* _result;
    };
    typedef std::shared_ptr<ResultWrapper> RES_WRAPPER;

    class BSQLRow : public Bscript::BObjectImp
    {
    public:
      BSQLRow( RES_WRAPPER resultset );
      BSQLRow( BSQLResultSet* resultset );
      //BSQLRow(MYSQL_RES *_result, MYSQL_ROW _row);
      BSQLRow( RES_WRAPPER _result, MYSQL_ROW _row, MYSQL_FIELD *_fields );
      ~BSQLRow();

      //virtual BObjectRef get_member( const char* membername );
      //virtual BObjectRef get_member_id( const int id ); //id test
      //virtual Bscript::BObjectImp* call_method( const char* methodname, Executor& ex );
      //virtual Bscript::BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
      virtual Bscript::BObjectImp* copy() const
      {
        return new BSQLRow( _result, _row, _fields );
      }
      virtual std::string getStringRep() const { return "SQLRow"; }
      virtual size_t sizeEstimate( ) const { return sizeof( *this ) + sizeof(MYSQL_FIELD); }
      virtual const char* typeOf() const { return "SQLRow"; }
      virtual int typeOfInt() const { return OTSQLRow; }
      virtual bool isTrue() const { return _row != 0; };
      virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj );

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
      const char *field_name( unsigned int index ) const;
      int num_rows() const
      {
        if ( !_result ) return 0;
        return static_cast<int>( mysql_num_rows( _result->ptr() ) );
      };
      virtual Bscript::BObjectImp* copy() const
      {
        if ( _affected_rows ) return new BSQLResultSet( _affected_rows );
        else return new BSQLResultSet( _result, _fields );
      };
      virtual std::string getStringRep() const;
      virtual size_t sizeEstimate( ) const { return sizeof( *this ) + sizeof( MYSQL_FIELD); }
      virtual const char* typeOf() const { return "SQLResultSet"; }
      virtual int typeOfInt() const { return OTSQLResultSet; }
      virtual bool isTrue() const;
      // virtual BObjectRef OperSubscript( const BObject& obj );

      friend class BSQLRow;
    private:
      RES_WRAPPER _result;
      MYSQL_FIELD* _fields;
      int _affected_rows;
    };
    class SQLService;
    class BSQLConnection : public Bscript::BObjectImp
    {
      class ConnectionWrapper;
    public:
      BSQLConnection();
      BSQLConnection( std::shared_ptr<ConnectionWrapper> conn );
      BSQLConnection( std::string host, std::string user, std::string password );
      ~BSQLConnection();
      bool connect( const char *host, const char *user, const char *passwd );
      bool query( const char *query );
      bool select_db( const char *db );
      bool close();
      Bscript::BObjectImp *getResultSet() const;

      std::string getLastError() const { return _error; };
      int getLastErrNo() const { return _errno; };
      std::shared_ptr<ConnectionWrapper> getConnection() const { return _conn; };

      virtual Bscript::BObjectRef get_member( const char* membername );
      virtual Bscript::BObjectRef get_member_id( const int id ); //id test
      virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
      virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false );
      virtual Bscript::BObjectImp* copy() const;
      virtual std::string getStringRep() const;
      virtual size_t sizeEstimate() const { return sizeof( *this )+_error.capacity(); }
      virtual const char* typeOf() const { return "SQLConnection"; }
      virtual int typeOfInt() const { return OTSQLConnection; }
      virtual bool isTrue() const;
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
        MYSQL* ptr() { return _conn; };
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
      void push( msg &&msg_ );
    private:
      msg_queue _msgs;
    };
    extern SQLService sql_service;
    void start_sql_service();
  }
}

#endif

#endif
