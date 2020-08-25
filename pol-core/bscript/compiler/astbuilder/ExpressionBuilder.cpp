#include "ExpressionBuilder.h"

#include "compiler/Report.h"

#include "compiler/ast/Argument.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/FunctionParameterDeclaration.h"
#include "compiler/ast/FunctionParameterList.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/StringValue.h"
#include "compiler/astbuilder/BuilderWorkspace.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
ExpressionBuilder::ExpressionBuilder( const SourceFileIdentifier& source_file_identifier,
                                      BuilderWorkspace& workspace )
  : ValueBuilder( source_file_identifier, workspace )
{
}

std::unique_ptr<Expression> ExpressionBuilder::expression( EscriptParser::ExpressionContext* ctx )
{
  if ( auto prim = ctx->primary() )
    return primary( prim );

  location_for( *ctx ).internal_error( "unhandled expression" );
}

std::unique_ptr<FunctionCall> ExpressionBuilder::function_call(
    EscriptParser::FunctionCallContext* ctx, const std::string& scope )
{
  auto method_name = text( ctx->IDENTIFIER() );

  auto arguments = value_arguments( ctx->expressionList() );

  auto function_call = std::make_unique<FunctionCall>( location_for( *ctx ), scope, method_name,
                                                       std::move( arguments ) );

  std::string key = scope.empty() ? method_name : ( scope + "::" + method_name );
  workspace.function_resolver.register_function_link( key, function_call->function_link );

  return function_call;
}

std::unique_ptr<Expression> ExpressionBuilder::primary( EscriptParser::PrimaryContext* ctx )
{
  if ( auto literal = ctx->literal() )
  {
    return value( literal );
  }
  else if ( auto par_expression = ctx->parExpression() )
  {
    return expression( par_expression->expression() );
  }
  else if ( auto identifier = ctx->IDENTIFIER() )
  {
    return std::make_unique<Identifier>( location_for( *ctx ), text( identifier ) );
  }
  else if ( auto f_call = ctx->functionCall() )
  {
    return function_call( f_call, "" );
  }
  else if ( auto scoped_f_call = ctx->scopedFunctionCall() )
  {
    return scoped_function_call( scoped_f_call );
  }

  location_for( *ctx ).internal_error( "unhandled primary expression" );
}

std::unique_ptr<FunctionCall> ExpressionBuilder::scoped_function_call(
    EscriptParser::ScopedFunctionCallContext* ctx )
{
  return function_call( ctx->functionCall(), text( ctx->IDENTIFIER() ) );
}

std::vector<std::unique_ptr<Argument>> ExpressionBuilder::value_arguments(
    EscriptGrammar::EscriptParser::ExpressionListContext* ctx )
{
  std::vector<std::unique_ptr<Argument>> arguments;

  if ( ctx )
  {
    for ( auto argument_context : ctx->expression() )
    {
      auto loc = location_for( *argument_context );

      std::string name;
      auto value = expression( argument_context );

      auto argument = std::make_unique<Argument>( loc, std::move( name ), std::move( value ) );
      arguments.push_back( std::move( argument ) );
    }
  }

  return arguments;
}

}  // namespace Pol::Bscript::Compiler
