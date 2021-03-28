#include "BinaryOperatorWithFloatOptimizer.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/FloatValue.h"

namespace Pol::Bscript::Compiler
{
BinaryOperatorWithFloatOptimizer::BinaryOperatorWithFloatOptimizer( FloatValue& lhs,
                                                                    BinaryOperator& op,
                                                                    Report& report )
    : lhs( lhs ), op( op ), report( report )
{
}

void BinaryOperatorWithFloatOptimizer::visit_children( Node& ) {}

void BinaryOperatorWithFloatOptimizer::visit_float_value( FloatValue& rhs )
{
  double dval = 0;
  switch ( op.token_id )
  {
  case TOK_ADD:
    dval = lhs.value + rhs.value;
    break;
  case TOK_SUBTRACT:
    dval = lhs.value - rhs.value;
    break;
  case TOK_MULT:
    dval = lhs.value * rhs.value;
    break;
  case TOK_DIV:
    if ( rhs.value == 0.0 )
    {
      report.error( op, "Expression would divide by zero." );
      return;
    }
    dval = lhs.value / rhs.value;
    break;

  default:
    return;
  }

  optimized_result = std::make_unique<FloatValue>( op.source_location, dval );
}

}  // namespace Pol::Bscript::Compiler
