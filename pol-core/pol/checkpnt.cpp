/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/logfacility.h"


#include "polcfg.h"
#include "checkpnt.h"
namespace Pol {
  namespace Core {
	const char* last_checkpoint = "";
	void checkpoint( const char* msg, unsigned short minlvl /* = 11 */ )
	{
	  last_checkpoint = msg;
	  if ( config.loglevel >= minlvl )
	  {
        POLLOG_INFO << "checkpoint: " << msg << "\n";
	  }
	}

	Checkpoint::Checkpoint( const char* file ) : _file( file ), _line( 0 )
	{}
	Checkpoint::~Checkpoint()
	{
      if ( _line )
        POLLOG_ERROR << "Abnormal end after checkpoint: File " << _file << ", line " << _line << "\n";
	}
  }
}