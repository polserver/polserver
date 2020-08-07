#include "BinaryOperatorWithStringOptimizer.h"

#include "compiler/ast/BinaryOperator.h"
#include "compiler/ast/StringValue.h"

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

}  // namespace Pol::Bscript::Compiler
