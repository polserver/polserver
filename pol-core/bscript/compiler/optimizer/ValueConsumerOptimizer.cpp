#include "ValueConsumerOptimizer.h"

#include "tokens.h"

#include "compiler/ast/AssignVariableConsume.h"
#include "compiler/ast/BinaryOperator.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/ValueConsumer.h"

namespace Pol::Bscript::Compiler
{
void ValueConsumerOptimizer::visit_children( Node& ) {}

void ValueConsumerOptimizer::visit_binary_operator( BinaryOperator& binary_operator )
{
  BTokenId operator_id = binary_operator.token_id;
  if ( operator_id == TOK_ASSIGN )
  {
    if ( dynamic_cast<Identifier*>( &binary_operator.lhs() ) )
    {
      auto lhs = static_unique_pointer_cast<Identifier>( binary_operator.take_lhs() );
      auto rhs = binary_operator.take_rhs();
      optimized_result = std::make_unique<AssignVariableConsume>(
          binary_operator.source_location, std::move( lhs ), std::move( rhs ) );
    }
    else
    {
      auto lhs = binary_operator.take_lhs();
      auto rhs = binary_operator.take_rhs();
      optimized_result =
          std::make_unique<BinaryOperator>( binary_operator.source_location, std::move( lhs ),
                                            ":= #", INS_ASSIGN_CONSUME, std::move( rhs ) );
    }
  }
}

std::unique_ptr<Statement> ValueConsumerOptimizer::optimize( ValueConsumer& consume_value )
{
  consume_value.children.at( 0 )->accept( *this );

  return std::move( optimized_result );
}

}  // namespace Pol::Bscript::Compiler
