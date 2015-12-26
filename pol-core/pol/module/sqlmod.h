/*
History
=======

Notes
=======

*/


#ifndef SQLMOD_H
#define SQLMOD_H

#include "../../bscript/execmodl.h"
#include "../uoexec.h"

namespace Pol {
  namespace Core {
    class BSQLConnection;
  }
  namespace Module {
	class SQLExecutorModule : public Bscript::TmplExecutorModule<SQLExecutorModule>
	{
	public:
      SQLExecutorModule( Bscript::Executor& exec ) :
        Bscript::TmplExecutorModule<SQLExecutorModule>( "sql", exec ),
        uoexec( static_cast<Core::UOExecutor&>( exec ) )
      {};

	  Bscript::BObjectImp* mf_ConnectToDB();
	  Bscript::BObjectImp* mf_Query();
	  Bscript::BObjectImp* mf_Close();
	  Bscript::BObjectImp* mf_NumFields();
	  Bscript::BObjectImp* mf_AffectedRows();
	  Bscript::BObjectImp* mf_FetchRow();
	  Bscript::BObjectImp* mf_NumRows();
	  Bscript::BObjectImp* mf_SelectDb();
	  Bscript::BObjectImp* mf_FieldName();

      static Bscript::BObjectImp* background_connect( Core::UOExecutor* uoexec, const std::string host, const std::string username, const std::string password );
      static Bscript::BObjectImp* background_select( Core::UOExecutor* uoexec, Core::BSQLConnection *sql, const std::string db );
      static Bscript::BObjectImp* background_query( Core::UOExecutor* uoexec, Core::BSQLConnection *sql, const std::string query );
    private:
      Core::UOExecutor& uoexec;
	};
  }
}
#endif
