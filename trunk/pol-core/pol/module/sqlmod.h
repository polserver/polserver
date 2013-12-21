/*
History
=======

Notes
=======

*/


#ifndef SQLMOD_H
#define SQLMOD_H

#include "../../bscript/execmodl.h"
namespace Pol {
  namespace Module {
	class SQLExecutorModule : public Bscript::TmplExecutorModule<SQLExecutorModule>
	{
	public:
      SQLExecutorModule( Bscript::Executor& exec ) :
        Bscript::TmplExecutorModule<SQLExecutorModule>( "sql", exec ) {}

	  Bscript::BObjectImp* mf_ConnectToDB();
	  Bscript::BObjectImp* mf_Query();
	  Bscript::BObjectImp* mf_Close();
	  Bscript::BObjectImp* mf_FreeResult();
	  Bscript::BObjectImp* mf_NumFields();
	  Bscript::BObjectImp* mf_AffectedRows();
	  Bscript::BObjectImp* mf_FetchRow();
	  Bscript::BObjectImp* mf_NumRows();
	  Bscript::BObjectImp* mf_SelectDb();
	  Bscript::BObjectImp* mf_FieldName();

	};
  }
}
#endif
