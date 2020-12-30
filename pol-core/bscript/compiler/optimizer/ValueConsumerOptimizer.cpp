#include "ValueConsumerOptimizer.h"

#include "tokens.h"

#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/ElementAssignment.h"
#include "bscript/compiler/ast/ElementIndexes.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/MemberAccess.h"
#include "bscript/compiler/ast/MemberAssignment.h"
#include "bscript/compiler/ast/MemberAssignmentByOperator.h"
#include "bscript/compiler/ast/ValueConsumer.h"
#include "bscript/compiler/ast/VariableAssignmentStatement.h"

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
      optimized_result = std::make_unique<VariableAssignmentStatement>(
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
  else if ( operator_id == TOK_PLUSEQUAL || operator_id == TOK_MINUSEQUAL ||
            operator_id == TOK_TIMESEQUAL || operator_id == TOK_DIVIDEEQUAL ||
            operator_id == TOK_MODULUSEQUAL )
  {
    if ( auto get_member = dynamic_cast<MemberAccess*>( &binary_operator.lhs() ) )
    {
      if ( get_member->known_member )
      {
        BTokenId token_id;
        switch ( operator_id )
        {
        case TOK_PLUSEQUAL:
          token_id = INS_SET_MEMBER_ID_CONSUME_PLUSEQUAL;
          break;
        case TOK_MINUSEQUAL:
          token_id = INS_SET_MEMBER_ID_CONSUME_MINUSEQUAL;
          break;
        case TOK_TIMESEQUAL:
          token_id = INS_SET_MEMBER_ID_CONSUME_TIMESEQUAL;
          break;
        case TOK_DIVIDEEQUAL:
          token_id = INS_SET_MEMBER_ID_CONSUME_DIVIDEEQUAL;
          break;
        case TOK_MODULUSEQUAL:
          token_id = INS_SET_MEMBER_ID_CONSUME_MODULUSEQUAL;
          break;
        default:
          get_member->internal_error( "cannot optimize (table entry missing)" );
        }
        bool consume = true;
        auto entity = get_member->take_entity();
        auto rhs = binary_operator.take_rhs();
        optimized_result = std::make_unique<MemberAssignmentByOperator>(
            get_member->source_location, consume, std::move( entity ), get_member->name, token_id,
            std::move( rhs ), *get_member->known_member );
      }
    }
  }
}

std::unique_ptr<Statement> ValueConsumerOptimizer::optimize( ValueConsumer& consume_value )
{
  consume_value.children.at( 0 )->accept( *this );

  return std::move( optimized_result );
}

}  // namespace Pol::Bscript::Compiler
