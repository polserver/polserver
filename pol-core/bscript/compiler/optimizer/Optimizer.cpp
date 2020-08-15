#include "Optimizer.h"

#include "clib/logfacility.h"
#include "compiler/Report.h"
#include "compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
{
Optimizer::Optimizer( Report& report )
  : report( report )
{
}

void Optimizer::optimize( CompilerWorkspace& )
{
}

}  // namespace Pol::Bscript::Compiler
