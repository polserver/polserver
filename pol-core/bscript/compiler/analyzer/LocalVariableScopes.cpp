#include "LocalVariableScopes.h"

#include "bscript/compiler/analyzer/Variables.h"

namespace Pol::Bscript::Compiler
{
LocalVariableScopes::LocalVariableScopes( Variables& local_variables, Report& report )
    : local_variables( local_variables ), report( report )
{
}

LocalVariableScope* LocalVariableScopes::current_local_scope()
{
  return local_variable_scopes.empty() ? nullptr : local_variable_scopes.back();
}
}  // namespace Pol::Bscript::Compiler
