#include "BinaryOperatorOptimizer.h"

#include "compiler/ast/BinaryOperator.h"

#include "BinaryOperatorWithFloatOptimizer.h"
#include "BinaryOperatorWithIntegerOptimizer.h"
#include "BinaryOperatorWithStringOptimizer.h"

namespace Pol::Bscript::Compiler
{
BinaryOperatorOptimizer::BinaryOperatorOptimizer( BinaryOperator& op, Report& report )
    : op( op ), report( report )
{
}

std::unique_ptr<Expression> BinaryOperatorOptimizer::optimize()
{
  op.lhs().accept( *this );
  return std::move( optimized_result );
}

void BinaryOperatorOptimizer::visit_children( Node& )
{
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
