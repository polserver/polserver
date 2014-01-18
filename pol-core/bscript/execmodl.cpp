/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <string.h>

#include "../clib/clib.h"
#include "../clib/stlutil.h"

#include "eprog.h"
#include "executor.h"
#include "execmodl.h"
#include "impstr.h"
 
namespace Pol {
  namespace Bscript {
	const string& ExecutorModule::scriptname() const
	{
	  return exec.prog_->name;
	}
  }
}