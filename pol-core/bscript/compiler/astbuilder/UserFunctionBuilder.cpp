#include "UserFunctionBuilder.h"

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/astbuilder/FunctionResolver.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
UserFunctionBuilder::UserFunctionBuilder( const SourceFileIdentifier& source_file_identifier,
                                          BuilderWorkspace& workspace )
    : CompoundStatementBuilder( source_file_identifier, workspace )
{
}

std::unique_ptr<UserFunction> UserFunctionBuilder::function_declaration(
    EscriptParser::FunctionDeclarationContext* ctx )
{
  std::string name = text( ctx->IDENTIFIER() );
  return make_user_function( name, ctx, ctx->EXPORTED(), ctx->ENDFUNCTION() );
}

std::unique_ptr<UserFunction> UserFunctionBuilder::function_expression(
    EscriptGrammar::EscriptParser::FunctionExpressionContext* ctx )
{
  std::string name = FunctionResolver::function_expression_name( location_for( *ctx->FUNCTION() ) );
  return make_user_function( name, ctx, false, ctx->RBRACE() );
}

template <typename ParserContext>
std::unique_ptr<UserFunction> UserFunctionBuilder::make_user_function(
    const std::string& name, ParserContext* ctx, bool exported,
    antlr4::tree::TerminalNode* end_token )
{
  std::vector<std::unique_ptr<FunctionParameterDeclaration>> parameters;
  if ( auto function_parameters = ctx->functionParameters() )
  {
    if ( auto param_list = function_parameters->functionParameterList() )
    {
      for ( auto param : param_list->functionParameter() )
      {
        std::string parameter_name = text( param->IDENTIFIER() );
        std::unique_ptr<FunctionParameterDeclaration> parameter_declaration;
        bool byref = param->BYREF() != nullptr;
        bool unused = param->UNUSED() != nullptr;

        if ( auto expr_ctx = param->expression() )
        {
          auto default_value = expression( expr_ctx );
          parameter_declaration = std::make_unique<FunctionParameterDeclaration>(
              location_for( *param ), std::move( parameter_name ), byref, unused,
              std::move( default_value ) );
        }
        else
        {
          parameter_declaration = std::make_unique<FunctionParameterDeclaration>(
              location_for( *param ), std::move( parameter_name ), byref, unused );
        }

        parameters.push_back( std::move( parameter_declaration ) );
      }
    }
  }
  auto parameter_list =
      std::make_unique<FunctionParameterList>( location_for( *ctx ), std::move( parameters ) );
  auto body =
      std::make_unique<FunctionBody>( location_for( *ctx ), block_statements( ctx->block() ) );

  return std::make_unique<UserFunction>( location_for( *ctx ), exported, std::move( name ),
                                         std::move( parameter_list ), std::move( body ),
                                         location_for( *end_token ) );
}
}  // namespace Pol::Bscript::Compiler
