#include "FunctionVariableScope.h"

#include "bscript/compiler/analyzer/Variables.h"

namespace Pol::Bscript::Compiler
{
FunctionVariableScope::FunctionVariableScope( Variables& variables ) : variables( variables )
{
  variables.variable_info_stack.emplace_back();
}

FunctionVariableScope::~FunctionVariableScope()
{
  variables.variable_info_stack.pop_back();
}

}  // namespace Pol::Bscript::Compiler
