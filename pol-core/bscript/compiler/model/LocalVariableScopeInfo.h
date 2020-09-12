#ifndef POLSERVER_LOCALVARIABLESCOPEINFO_H
#define POLSERVER_LOCALVARIABLESCOPEINFO_H

#include <memory>
#include <vector>

namespace Pol::Bscript::Compiler
{
class Variable;

class LocalVariableScopeInfo
{
public:
  unsigned base_index = 0;
  std::vector<std::shared_ptr<Variable>> variables;
};

}  // namespace Pol::Bscript::Compiler
#endif  // POLSERVER_LOCALVARIABLESCOPEINFO_H
