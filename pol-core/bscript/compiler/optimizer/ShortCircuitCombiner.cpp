#include "ShortCircuitCombiner.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperatorShortCircuit.h"

namespace Pol::Bscript::Compiler
{
class ShortCircuitCombinerChildVisitor : public NodeVisitor
{
public:
  ShortCircuitCombinerChildVisitor() = default;
  void visit_children( Node& ) override {};
  void visit_binary_operator_short_circuit( BinaryOperatorShortCircuit& child ) override
  {
    // its a child so no need to add convert instruction
    child.generate_logical_convert = false;
  };
};

ShortCircuitCombiner::ShortCircuitCombiner( Report& ) {}

void ShortCircuitCombiner::visit_children( Node& ) {}

void ShortCircuitCombiner::visit_binary_operator_short_circuit( BinaryOperatorShortCircuit& op )
{
  // dont visit recursivly just direct both sides
  ShortCircuitCombinerChildVisitor lhs_child{};
  op.lhs().accept( lhs_child );
  ShortCircuitCombinerChildVisitor rhs_child{};
  op.rhs().accept( rhs_child );
}
}  // namespace Pol::Bscript::Compiler
