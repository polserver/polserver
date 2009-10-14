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

#include "sqlmod.h"

template<>
TmplExecutorModule<SQLExecutorModule>::FunctionDef
TmplExecutorModule<SQLExecutorModule>::function_table[] =
{
	{ "Connect",					&SQLExecutorModule::mf_ConnectToDB }
};
template<>
int TmplExecutorModule<SQLExecutorModule>::function_table_size = arsize(function_table);

BObjectImp* SQLExecutorModule::mf_ConnectToDB()
{
	return new BLong(1);
}