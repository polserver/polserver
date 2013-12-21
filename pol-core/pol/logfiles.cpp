/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/dualbuf.h"
#include "../clib/nlogfile.h"

#include "logfiles.h"
namespace Pol {
  namespace Core {
	Clib::LogFile script_log( "log/script.log" );
	// dualbuf db_script_log;

	void open_logfiles()
	{
	  script_log.open( "log/script.log" );
	  //db_script_log.setbufs( script_log.rdbuf(), cout.rdbuf() );
	  //db_script_log.install( &script_log );


	}

	void close_logfiles()
	{
	  script_log.close();
	}
  }
}