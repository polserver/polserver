#include "BinaryOperatorOptimizer.h"

#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/BinaryOperatorShortCircuit.h"
#include "bscript/compilercfg.h"

#include "BinaryOperatorShortCircuitOptimizer.h"
#include "BinaryOperatorWithBooleanOptimizer.h"
#include "BinaryOperatorWithFloatOptimizer.h"
#include "BinaryOperatorWithIntegerOptimizer.h"
#include "BinaryOperatorWithStringOptimizer.h"
#include "ShortCircuitCombiner.h"

namespace Pol::Bscript::Compiler
{
BinaryOperatorOptimizer::BinaryOperatorOptimizer( BinaryOperator& op, Report& report )
    : op( op ), report( report )
{
}

std::unique_ptr<Expression> BinaryOperatorOptimizer::optimize()
{
  op.lhs().accept( *this );

  // second parse step
  if ( compilercfg.ShortCircuitEvaluation )
  {
    BinaryOperatorShortCircuitOptimizer shortcircuit{ report };
    if ( !optimized_result )
      op.accept( shortcircuit );
    else
      optimized_result->accept( shortcircuit );
    if ( shortcircuit.optimized_result )
    {
      optimized_result = std::move( shortcircuit.optimized_result );
      // third parse step
      ShortCircuitCombiner combiner{ report };
      optimized_result->accept( combiner );
    }
  }
  return std::move( optimized_result );
}

void BinaryOperatorOptimizer::visit_children( Node& ) {}

void BinaryOperatorOptimizer::visit_boolean_value( BooleanValue& lhs )
{
  BinaryOperatorWithBooleanOptimizer optimizer( lhs, op, report );
  op.rhs().accept( optimizer );
  optimized_result = std::move( optimizer.optimized_result );
}

void BinaryOperatorOptimizer::visit_float_value( FloatValue& lhs )
{
  BinaryOperatorWithFloatOptimizer optimizer( lhs, op, report );
  op.rhs().accept( optimizer );
  optimized_result = std::move( optimizer.optimized_result );
}

void BinaryOperatorOptimizer::visit_integer_value( IntegerValue& lhs )
{
  BinaryOperatorWithIntegerOptimizer optimizer( lhs, op, report );
  op.rhs().accept( optimizer );
  optimized_result = std::move( optimizer.optimized_result );
}

void BinaryOperatorOptimizer::visit_string_value( StringValue& lhs )
{
  BinaryOperatorWithStringOptimizer optimizer( lhs, op );
  op.rhs().accept( optimizer );
  optimized_result = std::move( optimizer.optimized_result );
}

}  // namespace Pol::Bscript::Compiler
