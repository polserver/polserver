#include "UserFunctionBuilder.h"

#include "bscript/compiler/ast/ClassBody.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/ClassParameterList.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/VarStatement.h"
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
    EscriptParser::FunctionDeclarationContext* ctx, const std::string& class_name )
{
  std::string name = text( ctx->IDENTIFIER() );
  return make_user_function( name, ctx, ctx->EXPORTED(), class_name, ctx->ENDFUNCTION() );
}

std::unique_ptr<UserFunction> UserFunctionBuilder::function_expression(
    EscriptGrammar::EscriptParser::FunctionExpressionContext* ctx )
{
  std::string name = FunctionResolver::function_expression_name( location_for( *ctx->AT() ) );
  return make_user_function( name, ctx, false, "", ctx->RBRACE() );
}

std::unique_ptr<ClassDeclaration> UserFunctionBuilder::class_declaration(
    EscriptGrammar::EscriptParser::ClassDeclarationContext* ctx )
{
  std::string class_name = text( ctx->IDENTIFIER() );
  std::vector<std::unique_ptr<Identifier>> parameters;

  if ( auto function_parameters = ctx->classParameters() )
  {
    if ( auto param_list = function_parameters->classParameterList() )
    {
      for ( auto parameter_name : param_list->IDENTIFIER() )
      {
        parameters.push_back( std::make_unique<Identifier>( location_for( *parameter_name ),
                                                            text( parameter_name ) ) );
      }
    }
  }

  NodeVector statements;

  if ( auto classBody = ctx->classBody() )
  {
    for ( auto classStatement : classBody->classStatement() )
    {
      // var statements are already included in top_level_statements by the SourceFileProcessor
      if ( auto method = classStatement->functionDeclaration() )
      {
        auto func_decl = function_declaration( method, class_name );
        statements.push_back( std::move( func_decl ) );
      }
    }
  }

  auto body = std::make_unique<ClassBody>( location_for( *ctx ), std::move( statements ) );

  auto parameter_list =
      std::make_unique<ClassParameterList>( location_for( *ctx ), std::move( parameters ) );

  auto class_decl = std::make_unique<ClassDeclaration>(
      location_for( *ctx ), class_name, std::move( parameter_list ), std::move( body ) );

  return class_decl;
}

template <typename ParserContext>
std::unique_ptr<UserFunction> UserFunctionBuilder::make_user_function(
    const std::string& name, ParserContext* ctx, bool exported, const std::string& class_name,
    antlr4::tree::TerminalNode* end_token )
{
  std::vector<std::unique_ptr<FunctionParameterDeclaration>> parameters;
  bool class_method = false;
  if ( auto function_parameters = ctx->functionParameters() )
  {
    if ( auto param_list = function_parameters->functionParameterList() )
    {
      // Determine if the function is a class method by checking if the first parameter is named
      // `this`. Only check if the function is a method (ie. class name is not empty).
      bool first = !class_name.empty();
      for ( auto param : param_list->functionParameter() )
      {
        std::string parameter_name = text( param->IDENTIFIER() );

        if ( first )
        {
          if ( Clib::caseInsensitiveEqual( parameter_name, "this" ) )
            class_method = true;

          first = false;
        }

        std::unique_ptr<FunctionParameterDeclaration> parameter_declaration;
        bool byref = param->BYREF() != nullptr;
        bool unused = param->UNUSED() != nullptr;
        bool rest = param->ELLIPSIS() != nullptr;

        if ( auto expr_ctx = param->expression() )
        {
          auto default_value = expression( expr_ctx );
          parameter_declaration = std::make_unique<FunctionParameterDeclaration>(
              location_for( *param ), std::move( parameter_name ), byref, unused, rest,
              std::move( default_value ) );
        }
        else
        {
          parameter_declaration = std::make_unique<FunctionParameterDeclaration>(
              location_for( *param ), std::move( parameter_name ), byref, unused, rest );
        }

        parameters.push_back( std::move( parameter_declaration ) );
      }
    }
  }
  auto parameter_list =
      std::make_unique<FunctionParameterList>( location_for( *ctx ), std::move( parameters ) );
  auto body =
      std::make_unique<FunctionBody>( location_for( *ctx ), block_statements( ctx->block() ) );

  constexpr bool expression =
      std::is_same<ParserContext, EscriptGrammar::EscriptParser::FunctionExpressionContext>::value;

  bool constructor_method = class_method && Clib::caseInsensitiveEqual( name, class_name );

  UserFunctionType type = !class_method        ? UserFunctionType::Static
                          : constructor_method ? UserFunctionType::Constructor
                                               : UserFunctionType::Method;

  return std::make_unique<UserFunction>( location_for( *ctx ), exported, expression, type,
                                         std::move( name ), std::move( parameter_list ),
                                         std::move( body ), location_for( *end_token ) );
}
}  // namespace Pol::Bscript::Compiler
