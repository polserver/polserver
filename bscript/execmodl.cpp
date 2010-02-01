/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <string.h>

#include "../clib/clib.h"
#include "../clib/mlog.h"
#include "../clib/stlutil.h"

#include "eprog.h"
#include "executor.h"
#include "execmodl.h"
#include "impstr.h"
 

const string& ExecutorModule::scriptname() const
{
    return exec.prog_->name;
}
