#include "ShortCircuitCombiner.h"

#include "bscript/StoredToken.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperatorShortCircuit.h"
#include "bscript/tokens.h"


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

void ShortCircuitCombiner::optimize_jumps( CodeSection& code )
{
  // recursivly check if a logical jump would jump to another jump and update the final jump
  // location
  auto combine = [&]( StoredToken& jump, auto&& combine )
  {
    const auto& loc = code[jump.offset];
    if ( loc.id == RSV_JMPIFFALSE )
    {
      if ( jump.type == TYP_LOGICAL_JUMP_FALSE )
        jump.offset = loc.offset;
      else
        jump.offset++;
    }
    else if ( loc.id == RSV_JMPIFTRUE )
    {
      if ( jump.type != TYP_LOGICAL_JUMP_FALSE )
        jump.offset = loc.offset;
      else
        jump.offset++;
    }
    else if ( loc.id == INS_LOGICAL_JUMP )
    {
      if ( loc.type == jump.type )
        jump.offset = loc.offset;
      else
        jump.offset++;
    }
    else if ( loc.id == INS_LOGICAL_CONVERT )
    {
      jump.offset++;
    }
    else
    {
      return;
    }
    return combine( jump, combine );
  };
  for ( auto& c : code )
    if ( c.id == INS_LOGICAL_JUMP )
      combine( c, combine );
}

}  // namespace Pol::Bscript::Compiler
