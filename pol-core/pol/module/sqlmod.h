/*
History
=======

Notes
=======

*/


#ifndef SQLMOD_H
#define SQLMOD_H

#include "../../bscript/execmodl.h"

class SQLExecutorModule : public TmplExecutorModule<SQLExecutorModule>
{
public:
	SQLExecutorModule(Executor& exec) :
	  TmplExecutorModule<SQLExecutorModule>("sql", exec) {}
	
	BObjectImp* mf_ConnectToDB();
	BObjectImp* mf_Query();
	BObjectImp* mf_Close();
	BObjectImp* mf_FreeResult();
	BObjectImp* mf_NumFields();
	BObjectImp* mf_AffectedRows();
	BObjectImp* mf_FetchRow();
	BObjectImp* mf_NumRows();
	BObjectImp* mf_SelectDb();
	BObjectImp* mf_FieldName();

};

#endif
