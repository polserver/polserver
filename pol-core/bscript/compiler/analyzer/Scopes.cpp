#include "Scopes.h"
#include "Variables.h"

namespace Pol::Bscript::Compiler
{
Scopes::Scopes( Variables& local_variables, Report& report )
    : local_variables( local_variables ), report( report )
{
}

LocalVariableScope* Scopes::current_local_scope()
{
  return local_variable_scopes.empty() ? nullptr : local_variable_scopes.back();
}

}  // namespace Pol::Bscript::Compiler
