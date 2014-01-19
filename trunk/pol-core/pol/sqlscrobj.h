/*
History
=======


Notes
=======

*/

#ifdef HAVE_MYSQL

#ifndef SQLSCROBJ_H
#define SQLSCROBJ_H

#include "../bscript/bobject.h"
#include "../bscript/contiter.h"
#ifdef _WIN32
#	include <windows.h>
#	include <winsock.h>
#	include <mysql.h>
#else
#	include <mysql/mysql.h>
#endif
namespace Pol {
  namespace Core {
    class BSQLResultSet;

    class BSQLRow : public Bscript::BObjectImp
    {
    public:
      BSQLRow( BSQLResultSet *resultset );
      //BSQLRow(MYSQL_RES *_result, MYSQL_ROW _row);
      BSQLRow( BSQLResultSet *_result, MYSQL_ROW _row, MYSQL_FIELD *_fields );
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
      virtual size_t sizeEstimate() const { return sizeof( *this ); }
      virtual const char* typeOf() const { return "SQLRow"; }
      virtual int typeOfInt() const { return OTSQLRow; }
      virtual bool isTrue() const { return _row != 0; };
      virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj );

    private:
      MYSQL_ROW _row;
      BSQLResultSet* _result;
      MYSQL_FIELD* _fields;
    };

    class BSQLResultSet : public Bscript::BObjectImp
    {
    public:
      BSQLResultSet( MYSQL_RES* result );
      BSQLResultSet( MYSQL_RES* result, MYSQL_FIELD* fields );
      BSQLResultSet( int affected_rows );
      ~BSQLResultSet();
      int num_fields() const;
      int affected_rows() const;
      bool free();
      const char *field_name( unsigned int index ) const;
      int num_rows() const
      {
        if ( !_result ) return 0;
        return static_cast<int>( mysql_num_rows( _result ) );
      };
      virtual Bscript::BObjectImp* copy() const
      {
        if ( _affected_rows ) return new BSQLResultSet( _affected_rows );
        else return new BSQLResultSet( _result, _fields );
      };
      virtual std::string getStringRep() const;
      virtual size_t sizeEstimate() const { return sizeof( *this ); }
      virtual const char* typeOf() const { return "SQLResultSet"; }
      virtual int typeOfInt() const { return OTSQLResultSet; }
      virtual bool isTrue() const;
      // virtual BObjectRef OperSubscript( const BObject& obj );

      friend class BSQLRow;
    private:
      MYSQL_RES* _result;
      MYSQL_FIELD* _fields;
      int _affected_rows;
    };
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

      virtual Bscript::BObjectRef get_member( const char* membername );
      virtual Bscript::BObjectRef get_member_id( const int id ); //id test
      virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
      virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false );
      virtual Bscript::BObjectImp* copy() const;
      virtual std::string getStringRep() const;
      virtual size_t sizeEstimate() const { return sizeof( *this ); }
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
        MYSQL* connection_ptr() { return _conn; };
      private:
        MYSQL* _conn;
      };
    };

  }
}

#endif

#endif
