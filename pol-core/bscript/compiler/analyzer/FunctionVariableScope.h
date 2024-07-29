#pragma once

namespace Pol::Bscript::Compiler
{
class Variables;

// RAII class to add a new `VariablesInfo` object onto `Variables`' stack.
class FunctionVariableScope
{
public:
  FunctionVariableScope( Variables& variables );
  ~FunctionVariableScope();

private:
  Variables& variables;
};

}  // namespace Pol::Bscript::Compiler
