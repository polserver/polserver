#include "ExpressionBuilder.h"

#include "compiler/Report.h"
#include <boost/range/adaptor/reversed.hpp>

#include "BuilderWorkspace.h"
#include "compiler/Report.h"
#include "compiler/ast/Argument.h"
#include "compiler/ast/FunctionCall.h"
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
  else if ( auto fcall = ctx->unambiguousFunctionCall() )
  {
    return function_call( fcall, "" );
  }
  else if ( auto scoped = ctx->unambiguousScopedFunctionCall() )
  {
    return scoped_function_call( scoped );
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
  else if ( auto scoped_method_call = ctx->scopedMethodCall() )
    return scoped_function_call( scoped_method_call );

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

std::unique_ptr<FunctionCall> ExpressionBuilder::function_call(
    EscriptParser::MethodCallContext* ctx, const std::string& scope )
{
  auto method_name = text( ctx->IDENTIFIER() );

  std::vector<std::unique_ptr<Argument>> arguments;

  if ( auto argList = ctx->methodCallArgumentList() )
  {
    for ( auto argument_context : argList->methodCallArgument() )
    {
      std::string identifier =
          argument_context->IDENTIFIER() ? text( argument_context->IDENTIFIER() ) : "";
      auto value = expression( argument_context->expression() );
      auto argument = std::make_unique<Argument>( location_for( *argument_context ),
                                                    std::move( identifier ), std::move( value ) );
      arguments.push_back( std::move( argument ) );
    }
  }
  auto function_call = std::make_unique<FunctionCall>( location_for( *ctx ), scope, method_name,
                                                         std::move( arguments ) );

  std::string key = scope.empty() ? method_name : ( scope + "::" + method_name );
  workspace.function_resolver.register_function_link( key, function_call->function_link );

  return function_call;
}

std::unique_ptr<FunctionCall> ExpressionBuilder::function_call(
    EscriptParser::UnambiguousFunctionCallContext* ctx, const std::string& scope )
{
  auto function_name = text( ctx->IDENTIFIER() );
  auto arguments = value_arguments( ctx->valueArguments() );
  auto function_call = std::make_unique<FunctionCall>( location_for( *ctx ), scope, function_name,
                                                         std::move( arguments ) );

  std::string key = scope.empty() ? function_name : ( scope + "::" + function_name );
  workspace.function_resolver.register_function_link( key, function_call->function_link );

  return function_call;
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

std::unique_ptr<FunctionCall> ExpressionBuilder::scoped_function_call(
    EscriptParser::ScopedMethodCallContext* ctx )
{
  return function_call( ctx->methodCall(), text( ctx->IDENTIFIER() ) );
}

std::unique_ptr<FunctionCall> ExpressionBuilder::scoped_function_call(
    EscriptParser::UnambiguousScopedFunctionCallContext* ctx )
{
  return function_call( ctx->unambiguousFunctionCall(), text( ctx->IDENTIFIER() ) );
}

std::vector<std::unique_ptr<Argument>> ExpressionBuilder::value_arguments(
    EscriptParser::ValueArgumentsContext* ctx )
{
  std::vector<std::unique_ptr<Argument>> arguments;

  if ( ctx )
  {
    if ( auto argList = ctx->unambiguousFunctionCallArgumentList() )
    {
      for ( auto argument_context : argList->unambiguousFunctionCallArgument() )
      {
        auto loc = location_for( *argument_context );

        std::string name;
        auto value = expression( argument_context->unambiguousExpression() );

        auto argument = std::make_unique<Argument>( loc, std::move( name ), std::move( value ) );
        arguments.push_back( std::move( argument ) );
      }
    }
  }
  return arguments;
}

}  // namespace Pol::Bscript::Compiler
