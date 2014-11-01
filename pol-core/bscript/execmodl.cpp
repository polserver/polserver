/*
History
=======

Notes
=======

*/


#include "execmodl.h"

#include "../clib/clib.h"
#include "../clib/stlutil.h"

#include "eprog.h"
#include "executor.h"
#include "impstr.h"

#include <string>

namespace Pol {
  namespace Bscript {
	const std::string& ExecutorModule::scriptname() const
	{
	  return exec.prog_->name;
	}
  }
}