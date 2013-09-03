/*
History
=======

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include "../../bscript/bobject.h"
#include "../../bscript/berror.h"
#include "../../bscript/impstr.h"

#include "../sqlscrobj.h"
#include "sqlmod.h"

template<>
TmplExecutorModule<SQLExecutorModule>::FunctionDef
TmplExecutorModule<SQLExecutorModule>::function_table[] =
{
	{ "MySQL_Connect",					&SQLExecutorModule::mf_ConnectToDB },
	{ "MySQL_Query",					&SQLExecutorModule::mf_Query },
	{ "MySQL_Close",					&SQLExecutorModule::mf_Close },
	{ "MySQL_Free_Result",					&SQLExecutorModule::mf_FreeResult },
	{ "MySQL_Num_Fields",					&SQLExecutorModule::mf_NumFields },
	{ "MySQL_Fetch_Row",					&SQLExecutorModule::mf_FetchRow },
	{ "MySQL_Affected_Rows",					&SQLExecutorModule::mf_AffectedRows },
	{ "MySQL_Num_Rows",					&SQLExecutorModule::mf_NumRows },
	{ "MySQL_Select_Db",					&SQLExecutorModule::mf_SelectDb },
	{ "MySQL_Field_Name",					&SQLExecutorModule::mf_FieldName }
};
template<>
int TmplExecutorModule<SQLExecutorModule>::function_table_size = arsize(function_table);

#ifdef HAVE_MYSQL

BObjectImp* SQLExecutorModule::mf_ConnectToDB()
{

	const String *host, *username, *password;
	if ( !(host = getStringParam(0) ) || !(username = getStringParam(1)) || !(password = getStringParam(2))) {
		return new BError("Invalid parameters");
	}

	BSQLConnection *sql = new BSQLConnection();
	if (sql->getLastErrNo()) {
		return new BError("Insufficient memory");
	}
	if (!sql->connect(host->data(),username->data(),password->data())) {
		return new BError(sql->getLastError());
	}
	return sql;
	//return new BLong(1);
}
BObjectImp* SQLExecutorModule::mf_SelectDb()
{
	BSQLConnection *sql;
	const String *db;
	if ( !(sql = EXPLICIT_CAST(BSQLConnection*,BObjectImp*)(getParamImp(0, BObjectImp::OTSQLConnection))) || !(db = getStringParam(1))) {
		return new BError("Invalid parameters");
	}
	if (!sql->select_db(*db))
		return new BError(sql->getLastError());
	return new BLong(1);
}

BObjectImp* SQLExecutorModule::mf_Query()
{
	BSQLConnection *sql;
	const String *query;
	if ( !(sql = EXPLICIT_CAST(BSQLConnection*,BObjectImp*)(getParamImp(0, BObjectImp::OTSQLConnection))) || !(query = getStringParam(1))) {
		return new BError("Invalid parameters");
	}
	if (!sql->query( query->getStringRep().c_str() )) {
		return new BError(sql->getLastError());
	}

	return sql->getResultSet();
	//return new BLong(0);
}

BObjectImp* SQLExecutorModule::mf_NumFields() {
	BSQLResultSet *result;
	if ( !(result = EXPLICIT_CAST(BSQLResultSet*,BObjectImp*)(getParamImp(0, BObjectImp::OTSQLResultSet)))) {
		return new BError("Invalid parameters");
	}
	return new BLong(result->num_fields());
}

BObjectImp* SQLExecutorModule::mf_FieldName() {
	BSQLResultSet *result;
	int index;
	//BLong& lng = (BLong &) right;
	//unsigned index = (unsigned) lng.value();

	if ( !(result = EXPLICIT_CAST(BSQLResultSet*,BObjectImp*)(getParamImp(0, BObjectImp::OTSQLResultSet))) || !getParam(1, index))  {
		return new BError("Invalid parameters");
	}
	const char *name = result->field_name(index);
	if (name == 0)
		return new BError("Column does not exist");
	return new String(name);
}

BObjectImp* SQLExecutorModule::mf_AffectedRows() {
	BSQLResultSet *result;
	if ( !(result = EXPLICIT_CAST(BSQLResultSet*,BObjectImp*)(getParamImp(0, BObjectImp::OTSQLResultSet)))) {
		return new BError("Invalid parameters");
	}
	return new BLong(result->affected_rows());
}

BObjectImp* SQLExecutorModule::mf_NumRows() {
	BSQLResultSet *result;
	if ( !(result = EXPLICIT_CAST(BSQLResultSet*,BObjectImp*)(getParamImp(0, BObjectImp::OTSQLResultSet)))) {
		return new BError("Invalid parameters");
	}
	return new BLong(result->num_rows());
}

BObjectImp* SQLExecutorModule::mf_Close() {
	BSQLConnection *sql;
	if ( !(sql = EXPLICIT_CAST(BSQLConnection*,BObjectImp*)(getParamImp(0, BObjectImp::OTSQLConnection))))
		return new BError("Invalid parameters");
	sql->close();
	return new BLong(1);
}
BObjectImp* SQLExecutorModule::mf_FreeResult() {
	BSQLResultSet *result;
	if ( !(result = EXPLICIT_CAST(BSQLResultSet*,BObjectImp*)(getParamImp(0, BObjectImp::OTSQLResultSet)))) {
		return new BError("Invalid parameters");
	}
	result->free();
	return new BLong(1);
}

BObjectImp* SQLExecutorModule::mf_FetchRow() {
	BSQLResultSet *result;
	if ( !(result = EXPLICIT_CAST(BSQLResultSet*,BObjectImp*)(getParamImp(0, BObjectImp::OTSQLResultSet)))) {
		return new BError("Invalid parameters");
	}
	return new BSQLRow(result);
}

#else

#define MF_NO_MYSQL(funcName) BObjectImp* SQLExecutorModule::funcName() { return new BError("POL was not compiled with MySQL support."); }
MF_NO_MYSQL(mf_ConnectToDB)
MF_NO_MYSQL(mf_SelectDb)
MF_NO_MYSQL(mf_Query)
MF_NO_MYSQL(mf_NumFields)
MF_NO_MYSQL(mf_FieldName)
MF_NO_MYSQL(mf_AffectedRows)
MF_NO_MYSQL(mf_NumRows)
MF_NO_MYSQL(mf_Close)
MF_NO_MYSQL(mf_FreeResult)
MF_NO_MYSQL(mf_FetchRow)
#endif
