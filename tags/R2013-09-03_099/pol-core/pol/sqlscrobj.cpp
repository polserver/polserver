/*
History
=======

Notes
=======

*/

#ifdef HAVE_MYSQL

#include "../clib/stl_inc.h"
#include "../clib/strutil.h"
#include "../clib/stlutil.h"
#include "../clib/endian.h"

#include "sqlscrobj.h"

#include "../bscript/executor.h"
#include "../bscript/execmodl.h"
#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"

#include "../plib/pkg.h"
#include "module/filemod.h"

BSQLRow::BSQLRow(BSQLResultSet *result) : BObjectImp(OTSQLRow) {
	_result = result; //->_result;
	_row = mysql_fetch_row(_result->_result);
	_fields = mysql_fetch_fields(_result->_result);
}
BSQLRow::BSQLRow(BSQLResultSet *result, MYSQL_ROW row, MYSQL_FIELD* fields) : BObjectImp(OTSQLRow), _row(row), _result(result), _fields(fields) {

}
BObjectRef BSQLRow::OperSubscript(const BObject& obj) {
	const BObjectImp& right = obj.impref();
	if (_result == 0)
		return BObjectRef( new BError( "No result" ) );
	unsigned int num_fields = mysql_num_fields(_result->_result);
	if (right.isa( OTLong )) // vector
	{
		BLong& lng = (BLong &) right;

		unsigned index = (unsigned) lng.value();
		if (index > num_fields || index <= 0) {
			return BObjectRef( new BError("Index out of bounds" ) );
		} else if (_row[index-1] == 0) {
			return BObjectRef( UninitObject::create() );
		} else if (IS_NUM(_fields[index-1].type) && _fields[index-1].type != MYSQL_TYPE_TIMESTAMP) {
			if (_fields[index-1].type == MYSQL_TYPE_DECIMAL || _fields[index-1].type == MYSQL_TYPE_NEWDECIMAL || _fields[index-1].type == MYSQL_TYPE_FLOAT || _fields[index-1].type == MYSQL_TYPE_DOUBLE)
				return BObjectRef( new Double( strtod( _row[index-1], NULL ) ) );
			return BObjectRef( new BLong( strtoul( _row[index-1], NULL , 0) ) );
		}
		return BObjectRef( new String(_row[index-1]) );
	} else if (right.isa(OTString)) {
		String& string = (String &) right;
		for (unsigned int i = 0; i < num_fields; i++) {
			if (!strncmp(_fields[i].name,string.data(),_fields[i].name_length)) {
				if (_row[i] == 0) {
					return BObjectRef( UninitObject::create() );
				} else if (IS_NUM(_fields[i].type) && _fields[i].type != MYSQL_TYPE_TIMESTAMP) {
					if (_fields[i].type == MYSQL_TYPE_DECIMAL || _fields[i].type == MYSQL_TYPE_NEWDECIMAL || _fields[i].type == MYSQL_TYPE_FLOAT || _fields[i].type == MYSQL_TYPE_DOUBLE)
						return BObjectRef( new Double( strtod( _row[i], NULL ) ) );
					return BObjectRef( new BLong( strtoul( _row[i], NULL , 0) ) );
				}
				return BObjectRef( new String(_row[i]) );
			}
		}
		return BObjectRef( new BError("Column does not exist") );
	}
	else
	{
		return BObjectRef( new BError( "SQLRow keys must be integer" ) );
	}
}
BSQLRow::~BSQLRow() {
//	cout<<"Destructed row"<<endl;
}
BSQLResultSet::BSQLResultSet(MYSQL_RES *result) : BObjectImp(OTSQLResultSet) , _result(result), _affected_rows(0) {
	_fields = mysql_fetch_fields(result);
}
BSQLResultSet::BSQLResultSet(MYSQL_RES *result, MYSQL_FIELD* fields) : BObjectImp(OTSQLResultSet) , _result(result), _fields(fields), _affected_rows(0) {
}
BSQLResultSet::BSQLResultSet(int affected_rows) : BObjectImp(OTSQLResultSet) , _result(0), _affected_rows(affected_rows) {

}
const char * BSQLResultSet::field_name(unsigned int index) const {
	if (_result == 0) return 0;
	if (index <= 0 || index > mysql_num_fields(_result)) {
		return 0;
	}
	return _fields[index-1].name;
}
int BSQLResultSet::num_fields() const {
	if (_result)
		return mysql_num_fields(_result);
	return 0;
}
int BSQLResultSet::affected_rows() const {
	return _affected_rows;
}
bool BSQLResultSet::free() {
	if (_result)
		mysql_free_result(_result);
	_result = 0;
	return true;
}
BSQLResultSet::~BSQLResultSet() {
	if (_result != 0) mysql_free_result(_result);
}
bool BSQLResultSet::isTrue() const {
	return true;
}
string BSQLResultSet::getStringRep() const {
	return "SQLResultSet";
}

bool BSQLConnection::close() {
	if (_conn)
		mysql_close(_conn);
	_conn = 0;
	return true;
}
BObjectImp *BSQLConnection::getResultSet() const {
	if (_errno) return new BError(_error);
	MYSQL_RES *result;

	result = mysql_store_result(_conn);
	if (result)  // there are rows
	{
		return new BSQLResultSet(result);
		// retrieve rows, then call mysql_free_result(result)
	}
	else  // mysql_store_result() returned nothing; should it have?
	{
/*		if (mysql_errno(_conn))
		{
			_error = mysql_error(_conn);
			_errno = mysql_errno(_conn);
			return new BError(_error);
		}
		else */if (mysql_field_count(_conn) == 0)
		{
			return new BSQLResultSet(static_cast<int>(mysql_affected_rows(_conn)));
		}
	}
	return new BError("Unknown error getting ResultSet");

}
BSQLConnection::BSQLConnection() : BObjectImp(OTSQLConnection) , _errno(0) {
	_conn = mysql_init(NULL);
	if (!_conn) {
		_error = "Insufficient memory";
		_errno = 1;
	}
}

BSQLConnection::BSQLConnection(std::string host, std::string user, std::string password) : BObjectImp(OTSQLConnection), _errno(0)
{

}
BSQLConnection::~BSQLConnection() {
	if (_conn) mysql_close(_conn);
	_conn = 0;
//	cout<<"Destructed connection"<<endl;
}
std::string BSQLConnection::getStringRep() const {
	return "SQLConnection";
}
bool BSQLConnection::isTrue() const {
	return true;
}
bool BSQLConnection::connect(const char *host, const char *user, const char *passwd) {
	//return true;
	if (!_conn) {
		_errno = -1;
		_error = "No active MYSQL object instance.";
		return false;
	}
	if (!mysql_real_connect(_conn,host,user,passwd,NULL,0,NULL,0))
	{
		_errno = mysql_errno(_conn);
		_error = mysql_error(_conn);
		return false;
	}
	return true;
}
bool BSQLConnection::select_db(const char *db) {
	if (!_conn) {
		_errno = -1;
		_error = "No active MYSQL object instance.";
		return false;
	}
	else if (mysql_select_db(_conn,db))
	{
		_errno = mysql_errno(_conn);
		_error = mysql_error(_conn);
		return false;
	}
	return true;
}
bool BSQLConnection::query(const char *query) {
	if (!_conn) {
		_errno = -1;
		_error = "No active MYSQL object instance.";
		return false;
	}
	if (mysql_query(_conn,query))
	{
		_errno = mysql_errno(_conn);
		_error = mysql_error(_conn);
		return false;
	}


	return true;
}


BObjectRef BSQLConnection::get_member_id( const int id )//id test
{
	return BObjectRef(UninitObject::create());
	//switch(id)
	//{

	//	default: return BObjectRef(UninitObject::create());
	//}
}
BObjectRef BSQLConnection::get_member( const char* membername )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if( objmember != NULL )
		return this->get_member_id(objmember->id);
	else
		return BObjectRef(UninitObject::create());
}

BObjectImp* BSQLConnection::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return NULL;
}

BObjectImp* BSQLConnection::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
	return new BLong(0);
}

BObjectImp* BSQLConnection::copy() const
{
	return new BSQLConnection();
}

#endif
