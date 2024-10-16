#include "LocalVariableScopes.h"

#include "bscript/compiler/analyzer/Variables.h"
#include "bscript/compiler/model/ScopeTree.h"

namespace Pol::Bscript::Compiler
{
LocalVariableScopes::LocalVariableScopes( ScopeTree& scope_tree, Variables& local_variables,
                                          Report& report )
    : local_variables( local_variables ), tree( scope_tree ), report( report )
{
}

LocalVariableScope* LocalVariableScopes::current_local_scope()
{
  return local_variable_scopes.empty() ? nullptr : local_variable_scopes.back();
}
}  // namespace Pol::Bscript::Compiler
