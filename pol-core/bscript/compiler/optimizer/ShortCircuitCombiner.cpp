#include "ShortCircuitCombiner.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperatorShortCircuit.h"


namespace Pol::Bscript::Compiler
{
class ShortCircuitCombinerChildVisitor : public NodeVisitor
{
public:
  ShortCircuitCombinerChildVisitor( BinaryOperatorShortCircuit& parent, bool rhs )
      : parent( parent ), rhs( rhs ){};
  void visit_children( Node& ) override{};
  void visit_binary_operator_short_circuit( BinaryOperatorShortCircuit& child ) override
  {
    // create a new jmplink or use the existing
    auto parent_endjmp = parent.linked_jmp_label
                             ? parent.linked_jmp_label
                             : std::make_shared<JmpLocationLink>( parent.end_label, parent.oper );
    // its a child so no need to add convert instruction
    child.generate_logical_convert = false;
    // on the left side dont jump if the operand is different (would skip the rhs)
    if ( !rhs && child.oper != parent_endjmp->oper )
      return;
    // update the jmp, so that it will propagate
    if ( child.linked_jmp_label )
      child.linked_jmp_label->update( *parent_endjmp );
    else
      child.linked_jmp_label = parent_endjmp;
    // set the parent also, thus if its a child it would propagate to its children
    if ( !parent.linked_jmp_label )
      parent.linked_jmp_label = parent_endjmp;
  };

private:
  BinaryOperatorShortCircuit& parent;
  bool rhs;
};


ShortCircuitCombiner::ShortCircuitCombiner( Report& ) {}

void ShortCircuitCombiner::visit_children( Node& ) {}

void ShortCircuitCombiner::visit_binary_operator_short_circuit( BinaryOperatorShortCircuit& op )
{
  // dont visit recursivly just direct both sides
  ShortCircuitCombinerChildVisitor lhs_child{ op, false };
  op.lhs().accept( lhs_child );
  ShortCircuitCombinerChildVisitor rhs_child{ op, true };
  op.rhs().accept( rhs_child );
}

}  // namespace Pol::Bscript::Compiler
