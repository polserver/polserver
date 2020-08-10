#include "ExpressionBuilder.h"

#include "compiler/Report.h"
#include "compiler/ast/Value.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
ExpressionBuilder::ExpressionBuilder( const SourceFileIdentifier& source_file_identifier,
                                      BuilderWorkspace& workspace )
  : ValueBuilder( source_file_identifier, workspace )
{
}

std::unique_ptr<Expression> ExpressionBuilder::additive_expression(
    EscriptParser::AdditiveExpressionContext* ctx )
{
  auto operands = ctx->multiplicativeExpression();

  if ( operands.size() == 1 )
    return multiplicative_expression( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::atomic( EscriptParser::AtomicExpressionContext* ctx )
{
  if ( auto literal = ctx->literal() )
  {
    return value( literal );
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled atomic" );
  }
}

std::unique_ptr<Expression> ExpressionBuilder::bitshift_left(
    EscriptParser::BitshiftLeftContext* ctx )
{
  auto operands = ctx->comparison();

  if ( operands.size() == 1 )
    return comparison( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::bitshift_right(
    EscriptParser::BitshiftRightContext* ctx )
{
  auto operands = ctx->bitshiftLeft();

  if ( operands.size() == 1 )
    return bitshift_left( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::bitwise_conjunction(
    EscriptParser::BitwiseConjunctionContext* ctx )
{
  auto operands = ctx->bitshiftRight();

  if ( operands.size() == 1 )
    return bitshift_right( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::bitwise_disjunction(
    EscriptParser::BitwiseDisjunctionContext* ctx )
{
  auto operands = ctx->bitwiseXor();

  if ( operands.size() == 1 )
    return bitwise_xor( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::bitwise_xor( EscriptParser::BitwiseXorContext* ctx )
{
  auto operands = ctx->bitwiseConjunction();

  if ( operands.size() == 1 )
    return bitwise_conjunction( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::comparison( EscriptParser::ComparisonContext* ctx )
{
  auto operands = ctx->infixOperation();

  if ( operands.size() == 1 )
    return infix_operation( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::conjunction( EscriptParser::ConjunctionContext* ctx )
{
  auto operands = ctx->equality();

  if ( operands.size() == 1 )
    return equality( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::disjunction( EscriptParser::DisjunctionContext* ctx )
{
  auto operands = ctx->conjunction();

  if ( operands.size() == 1 )
    return conjunction( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::elvis_expression(
    EscriptParser::ElvisExpressionContext* ctx )
{
  auto operands = ctx->additiveExpression();

  if ( operands.size() == 1 )
    return additive_expression( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::equality( EscriptParser::EqualityContext* ctx )
{
  auto operands = ctx->bitwiseDisjunction();

  if ( operands.size() == 1 )
    return bitwise_disjunction( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::expression( EscriptParser::ExpressionContext* ctx )
{
  if ( auto prim = ctx->primary() )
    return primary( prim );

  location_for( *ctx ).internal_error( "unhandled expression" );
}

std::unique_ptr<Expression> ExpressionBuilder::expression(
    EscriptParser::UnambiguousExpressionContext* ctx )
{
  auto operands = ctx->membership();
  if ( operands.size() == 1 )
    return membership( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::infix_operation(
    EscriptParser::InfixOperationContext* ctx )
{
  auto operands = ctx->elvisExpression();

  if ( operands.size() == 1 )
    return elvis_expression( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::membership( EscriptParser::MembershipContext* ctx )
{
  auto operands = ctx->disjunction();
  if ( operands.size() == 1 )
    return disjunction( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::multiplicative_expression(
    EscriptParser::MultiplicativeExpressionContext* ctx )
{
  auto operands = ctx->prefixUnaryInversionExpression();

  if ( operands.size() == 1 )
    return prefix_unary_inversion( operands[0] );

  location_for( *ctx ).internal_error( "unhandled" );
}

std::unique_ptr<Expression> ExpressionBuilder::prefix_unary_arithmetic(
    EscriptParser::PrefixUnaryArithmeticExpressionContext* ctx )
{
  auto result = postfix_unary( ctx->postfixUnaryExpression() );

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::prefix_unary_inversion(
    EscriptParser::PrefixUnaryInversionExpressionContext* ctx )
{
  auto result = prefix_unary_arithmetic( ctx->prefixUnaryArithmeticExpression() );

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::postfix_unary(
    EscriptParser::PostfixUnaryExpressionContext* ctx )
{
  auto result = atomic( ctx->atomicExpression() );

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::primary( EscriptParser::PrimaryContext* ctx )
{
  if ( auto literal = ctx->literal() )
    return value( literal );

  location_for( *ctx ).internal_error( "unhandled primary expression" );
}

}  // namespace Pol::Bscript::Compiler
