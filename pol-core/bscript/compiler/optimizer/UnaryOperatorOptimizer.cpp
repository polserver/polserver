#include "bscript/compiler/optimizer/UnaryOperatorOptimizer.h"

#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/MemberAccess.h"
#include "bscript/compiler/ast/MemberAssignmentByOperator.h"
#include "bscript/compiler/ast/UnaryOperator.h"
#include "bscript/compiler/ast/UninitializedValue.h"
#include "bscript/compiler/optimizer/ConstantFolder.h"

namespace Pol::Bscript::Compiler
{
UnaryOperatorOptimizer::UnaryOperatorOptimizer( UnaryOperator& unary_operator )
    : unary_operator( unary_operator )
{
}

std::unique_ptr<Expression> UnaryOperatorOptimizer::optimize()
{
  optimized_result = ConstantFolder::fold( unary_operator );
  if ( !optimized_result )
    unary_operator.operand().accept( *this );
  return std::move( optimized_result );
}

void UnaryOperatorOptimizer::visit_children( Node& ) {}

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

void UnaryOperatorOptimizer::visit_uninitialized_value( UninitializedValue& uninit )
{
  if ( unary_operator.token_id == TOK_LOG_NOT )
  {
    optimized_result = std::make_unique<IntegerValue>( uninit.source_location, 1 );
  }
}
}  // namespace Pol::Bscript::Compiler
