/*
History
=======


Notes
=======

*/

#ifndef LOGFILES_H
#define LOGFILES_H

#include "../clib/nlogfile.h"
namespace Pol {
  namespace Core {
	extern Clib::LogFile script_log;

	void open_logfiles();
	void close_logfiles();
  }
}
#endif
