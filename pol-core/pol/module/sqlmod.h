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

};

#endif
