#include "LocalVariableScopes.h"

#include <limits>

#include "bscript/compiler/analyzer/LocalVariableScope.h"
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

FunctionDepth LocalVariableScopes::current_function_depth()
{
  return local_variable_scopes.empty() ? std::numeric_limits<size_t>::max()
                                       : local_variable_scopes.back()->function_depth;
}

}  // namespace Pol::Bscript::Compiler
