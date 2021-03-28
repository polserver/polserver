#ifndef POLSERVER_LOCALVARIABLESCOPES_H
#define POLSERVER_LOCALVARIABLESCOPES_H

#include <vector>

namespace Pol::Bscript::Compiler
{
class LocalVariableScope;
class Report;
class ScopeTree;
class Variables;

class LocalVariableScopes
{
public:
  LocalVariableScopes( ScopeTree& scope_tree, Variables& locals, Report& report );

  LocalVariableScope* current_local_scope();

private:
  friend class LocalVariableScope;

  Variables& local_variables;
  std::vector<LocalVariableScope*> local_variable_scopes;
  ScopeTree& tree;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_LOCALVARIABLESCOPES_H
