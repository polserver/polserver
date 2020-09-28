#include "UnaryOperatorOptimizer.h"

#include "compiler/ast/FloatValue.h"
#include "compiler/ast/IntegerValue.h"
#include "compiler/ast/MemberAccess.h"
#include "compiler/ast/MemberAssignmentByOperator.h"
#include "compiler/ast/UnaryOperator.h"

namespace Pol::Bscript::Compiler
{
UnaryOperatorOptimizer::UnaryOperatorOptimizer( UnaryOperator& unary_operator )
    : unary_operator( unary_operator )
{
}

std::unique_ptr<Expression> UnaryOperatorOptimizer::optimize()
{
  unary_operator.operand().accept( *this );
  return std::move( optimized_result );
}

void UnaryOperatorOptimizer::visit_children( Node& ) {}

void UnaryOperatorOptimizer::visit_float_value( FloatValue& fv )
{
  double value;

  switch ( unary_operator.token_id )
  {
  case TOK_UNMINUS:
    value = -fv.value;
    break;

  default:
    return;
  }

  optimized_result = std::make_unique<FloatValue>( fv.source_location, value );
}

void UnaryOperatorOptimizer::visit_integer_value( IntegerValue& iv )
{
  int value;

  switch ( unary_operator.token_id )
  {
  case TOK_UNMINUS:
    value = -iv.value;
    break;
  case TOK_LOG_NOT:
    value = !iv.value;
    break;
  case TOK_BITWISE_NOT:
    value = static_cast<int>( ~static_cast<unsigned>( iv.value ) );
    break;

  default:
    return;
  }

  optimized_result = std::make_unique<IntegerValue>( iv.source_location, value );
}

void UnaryOperatorOptimizer::visit_member_access( MemberAccess& gm )
{
  if ( !gm.known_member )
    return;

  BTokenId new_token_id;
  switch ( unary_operator.token_id )
  {
  case TOK_UNPLUSPLUS:
    new_token_id = INS_SET_MEMBER_ID_UNPLUSPLUS;
    break;
  case TOK_UNMINUSMINUS:
    new_token_id = INS_SET_MEMBER_ID_UNMINUSMINUS;
    break;
  case TOK_UNPLUSPLUS_POST:
    new_token_id = INS_SET_MEMBER_ID_UNPLUSPLUS_POST;
    break;
  case TOK_UNMINUSMINUS_POST:
    new_token_id = INS_SET_MEMBER_ID_UNMINUSMINUS_POST;
    break;
  default:
    return;
  }

  bool consume = false;
  auto entity = gm.take_entity();
  optimized_result = std::make_unique<MemberAssignmentByOperator>(
      gm.source_location, consume, std::move( entity ), gm.name, new_token_id, *gm.known_member );
}

}  // namespace Pol::Bscript::Compiler
