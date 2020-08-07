#ifndef POLSERVER_SCOPES_H
#define POLSERVER_SCOPES_H

#include <vector>

namespace Pol::Bscript::Compiler
{
class LocalVariableScope;
class Report;
class Variables;

class Scopes
{
public:
  Scopes( Variables& locals, Report& report );

  LocalVariableScope* current_local_scope();

private:
  friend class LocalVariableScope;

  Variables& local_variables;
  std::vector<LocalVariableScope*> local_variable_scopes;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_SCOPES_H
