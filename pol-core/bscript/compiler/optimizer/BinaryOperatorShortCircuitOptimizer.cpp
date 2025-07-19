#include "BinaryOperatorShortCircuitOptimizer.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/BinaryOperatorShortCircuit.h"

#include "BinaryOperatorWithBooleanOptimizer.h"
#include "BinaryOperatorWithFloatOptimizer.h"
#include "BinaryOperatorWithIntegerOptimizer.h"
#include "BinaryOperatorWithStringOptimizer.h"
#include <memory>

namespace Pol::Bscript::Compiler
{
BinaryOperatorShortCircuitOptimizer::BinaryOperatorShortCircuitOptimizer( Report& report )
    : op( nullptr ), report( report )
{
}

std::unique_ptr<Expression> BinaryOperatorShortCircuitOptimizer::optimize()
{
  if ( op )
  {
    optimized_result = std::make_unique<BinaryOperatorShortCircuit>(
        op->source_location, op->take_lhs(), op->op, op->token_id, op->take_rhs() );
  }
  return std::move( optimized_result );
}

void BinaryOperatorShortCircuitOptimizer::visit_children( Node& ) {}
void BinaryOperatorShortCircuitOptimizer::visit_binary_operator( BinaryOperator& binary_operator )
{
  switch ( binary_operator.token_id )
  {
  case TOK_OR:
  case TOK_AND:
    op = &binary_operator;
    break;
  default:
    break;
  }
}


}  // namespace Pol::Bscript::Compiler
