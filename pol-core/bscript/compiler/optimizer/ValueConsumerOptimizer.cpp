#include "ValueConsumerOptimizer.h"

#include "tokens.h"

#include "compiler/ast/ArrayElementIndexes.h"
#include "compiler/ast/AssignSubscript.h"
#include "compiler/ast/AssignVariableConsume.h"
#include "compiler/ast/BinaryOperator.h"
#include "compiler/ast/GetMember.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/SetMember.h"
#include "compiler/ast/SetMemberByOperator.h"
#include "compiler/ast/ValueConsumer.h"

namespace Pol::Bscript::Compiler
{
void ValueConsumerOptimizer::visit_children( Node& ) {}

void ValueConsumerOptimizer::visit_assign_subscript( AssignSubscript& node )
{
  if ( node.indexes().children.size() == 1 )
  {
    auto indexes = node.take_indexes();
    auto entity = node.take_entity();
    auto rhs = node.take_rhs();
    optimized_result = std::make_unique<AssignSubscript>(
        node.source_location, true, std::move( entity ), std::move( indexes ), std::move( rhs ) );
  }
}

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
      return;
    }

    auto lhs = binary_operator.take_lhs();
    auto rhs = binary_operator.take_rhs();
    optimized_result =
        std::make_unique<BinaryOperator>( binary_operator.source_location, std::move( lhs ),
                                            ":= #", INS_ASSIGN_CONSUME, std::move( rhs ) );
  }
  else if ( operator_id == TOK_PLUSEQUAL || operator_id == TOK_MINUSEQUAL ||
            operator_id == TOK_TIMESEQUAL || operator_id == TOK_DIVIDEEQUAL ||
            operator_id == TOK_MODULUSEQUAL )
  {
    if ( auto get_member = dynamic_cast<GetMember*>( &binary_operator.lhs() ) )
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
        optimized_result = std::make_unique<SetMemberByOperator>(
            get_member->source_location, consume, std::move( entity ), get_member->name, token_id,
            std::move( rhs ), *get_member->known_member );
      }
    }
  }
}

void ValueConsumerOptimizer::visit_set_member( SetMember& node )
{
  if ( node.consume )
    node.internal_error(
        "ConsumeValueOptimizer did not expect an already-consumed SetMember node" );
  auto entity = node.take_entity();
  auto rhs = node.take_rhs();
  optimized_result =
      std::make_unique<SetMember>( node.source_location, true, std::move( entity ), node.name,
                                     std::move( rhs ), node.known_member );
}

std::unique_ptr<Statement> ValueConsumerOptimizer::optimize( ValueConsumer& consume_value )
{
  consume_value.children.at( 0 )->accept( *this );

  return std::move( optimized_result );
}

}  // namespace Pol::Bscript::Compiler
