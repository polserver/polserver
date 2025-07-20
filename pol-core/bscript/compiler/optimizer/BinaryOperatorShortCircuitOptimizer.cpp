#include "BinaryOperatorShortCircuitOptimizer.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/BinaryOperatorShortCircuit.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
BinaryOperatorShortCircuitOptimizer::BinaryOperatorShortCircuitOptimizer( Report& ) {}

void BinaryOperatorShortCircuitOptimizer::visit_children( Node& ) {}
void BinaryOperatorShortCircuitOptimizer::visit_binary_operator( BinaryOperator& op )
{
  switch ( op.token_id )
  {
  case TOK_OR:
  case TOK_AND:
    optimized_result = std::make_unique<BinaryOperatorShortCircuit>(
        op.source_location, op.take_lhs(), op.op, op.token_id, op.take_rhs() );
    break;
  default:
    break;
  }
}


}  // namespace Pol::Bscript::Compiler
