#include "BinaryOperatorWithStringOptimizer.h"

#include "bscript/bobject.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/FloatValue.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/StringValue.h"

namespace Pol::Bscript::Compiler
{
BinaryOperatorWithStringOptimizer::BinaryOperatorWithStringOptimizer( StringValue& lhs,
                                                                      BinaryOperator& op )
    : lhs( lhs ), op( op )
{
}

void BinaryOperatorWithStringOptimizer::visit_children( Node& ) {}

void BinaryOperatorWithStringOptimizer::visit_string_value( StringValue& rhs )
{
  std::string value;
  switch ( op.token_id )
  {
  case TOK_ADD:
  {
    value = lhs.value + rhs.value;
    break;
  }

  default:
    return;
  }

  optimized_result = std::make_unique<StringValue>( op.source_location, value );
}

void BinaryOperatorWithStringOptimizer::visit_float_value( FloatValue& rhs )
{
  std::string value;
  switch ( op.token_id )
  {
  case TOK_ADD:
  {
    value = fmt::format( "{}{}", lhs.value, Bscript::Double::double_to_string( rhs.value ) );
    break;
  }

  default:
    return;
  }

  optimized_result = std::make_unique<StringValue>( op.source_location, value );
}

void BinaryOperatorWithStringOptimizer::visit_integer_value( IntegerValue& rhs )
{
  std::string value;
  switch ( op.token_id )
  {
  case TOK_ADD:
  {
    value = fmt::format( "{}{}", lhs.value, rhs.value );
    break;
  }

  default:
    return;
  }

  optimized_result = std::make_unique<StringValue>( op.source_location, value );
}
}  // namespace Pol::Bscript::Compiler
