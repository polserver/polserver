#pragma once

#include "bscript/compiler/representation/CompiledScript.h"

namespace Pol::Bscript::Compiler
{
class CodeSectionOptimizer
{
public:
  CodeSectionOptimizer() = default;
  void optimize( CodeSection& code ) const;

private:
  void short_circuit_jumps( CodeSection& code ) const;
};
}  // namespace Pol::Bscript::Compiler
