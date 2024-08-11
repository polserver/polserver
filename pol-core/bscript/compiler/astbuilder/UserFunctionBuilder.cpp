#include "UserFunctionBuilder.h"

#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/ClassBody.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/ClassParameterList.h"
#include "bscript/compiler/ast/DefaultConstructorFunction.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/VarStatement.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/astbuilder/FunctionResolver.h"
#include "bscript/compiler/model/CompilerWorkspace.h"

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
    EscriptGrammar::EscriptParser::ClassDeclarationContext* ctx, Node* class_body )
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

  bool has_constructor = false;
  std::vector<std::string> function_names;

  if ( auto classBody = ctx->classBody() )
  {
    for ( auto classStatement : classBody->classStatement() )
    {
      if ( auto func_decl = classStatement->functionDeclaration() )
      {
        auto func_name = text( func_decl->IDENTIFIER() );
        auto func_loc = location_for( *func_decl );
        // Register the user function as an available parse tree.
        workspace.function_resolver.register_available_scoped_function( func_loc, class_name,
                                                                        func_decl );

        function_names.push_back( func_name );

        // TODO this doesn't check for `this`, will be handled in the construction PR.
        bool is_constructor = Clib::caseInsensitiveEqual( class_name, func_name );

        has_constructor |= is_constructor;
        workspace.compiler_workspace.all_function_locations.emplace(
            ScopableName( class_name, func_name ).string(), func_loc );
      }
      else if ( auto var_statement = classStatement->varStatement() )
      {
        std::vector<std::unique_ptr<Statement>> statements;

        add_var_statements( var_statement, class_name, statements );

        for ( auto& statement : statements )
        {
          class_body->children.push_back( std::move( statement ) );
        }
      }
    }
  }

  std::unique_ptr<DefaultConstructorFunction> constructor;

  auto parameter_list =
      std::make_unique<ClassParameterList>( location_for( *ctx ), std::move( parameters ) );

  if ( !has_constructor )
  {
    // If no user-defined constructor present, create a constructor that just calls 'super()'. The
    // semantic analyzer will catch errors re. missing parameters, etc.
    constructor = std::make_unique<DefaultConstructorFunction>( location_for( *ctx ), class_name );

    workspace.function_resolver.register_user_function( class_name, constructor.get() );
  }

  auto class_decl = std::make_unique<ClassDeclaration>(
      location_for( *ctx ), class_name, std::move( parameter_list ), std::move( function_names ),
      class_body, std::move( constructor ) );

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
        bool is_this_arg = false;

        if ( first )
        {
          if ( Clib::caseInsensitiveEqual( parameter_name, "this" ) )
          {
            class_method = true;
            is_this_arg = true;
          }

          first = false;
        }

        std::unique_ptr<FunctionParameterDeclaration> parameter_declaration;
        bool byref = param->BYREF() != nullptr || is_this_arg;
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
                                         class_name, std::move( name ), std::move( parameter_list ),
                                         std::move( body ), location_for( *end_token ) );
}
}  // namespace Pol::Bscript::Compiler
