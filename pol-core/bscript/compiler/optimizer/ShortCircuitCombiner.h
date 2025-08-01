#pragma once

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
class Report;

class ShortCircuitCombiner : public NodeVisitor
{
public:
  ShortCircuitCombiner( Report& );
  void visit_children( Node& ) override;
  void visit_binary_operator_short_circuit( BinaryOperatorShortCircuit& ) override;
};

}  // namespace Pol::Bscript::Compiler
