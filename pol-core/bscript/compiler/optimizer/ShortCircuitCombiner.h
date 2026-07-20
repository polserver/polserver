#pragma once

#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/representation/CompiledScript.h"

namespace Pol::Bscript::Compiler
{
class Report;

class ShortCircuitCombiner : public NodeVisitor
{
public:
  ShortCircuitCombiner( Report& );
  void visit_children( Node& ) override;
  void visit_binary_operator_short_circuit( BinaryOperatorShortCircuit& ) override;

  static void optimize_jumps( CodeSection& code );
};

}  // namespace Pol::Bscript::Compiler
