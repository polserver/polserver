#include "Optimizer.h"

#include "../clib/logfacility.h"

#include "compiler/Report.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
{
Optimizer::Optimizer( Report& report )
  : report( report )
{
}

void Optimizer::optimize( CompilerWorkspace& workspace )
{
  ReferencedFunctionGatherer gatherer( workspace.module_function_declarations );
  workspace.top_level_statements->accept( gatherer );
  workspace.referenced_module_function_declarations =
      gatherer.take_referenced_module_function_declarations();
}

}  // namespace Pol::Bscript::Compiler
