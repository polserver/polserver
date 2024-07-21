#ifndef POLSERVER_LOCALVARIABLESCOPES_H
#define POLSERVER_LOCALVARIABLESCOPES_H

#include <vector>

#include "bscript/compiler/model/SimpleTypes.h"

namespace Pol::Bscript::Compiler
{
class LocalVariableScope;
class Report;
class Variables;

class LocalVariableScopes
{
public:
  LocalVariableScopes( Variables& locals, Report& report );

  LocalVariableScope* current_local_scope();
  FunctionDepth current_function_depth();

private:
  friend class LocalVariableScope;

  Variables& local_variables;
  std::vector<LocalVariableScope*> local_variable_scopes;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_LOCALVARIABLESCOPES_H
