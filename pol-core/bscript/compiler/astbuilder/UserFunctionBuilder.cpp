#include "UserFunctionBuilder.h"

#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/ClassBody.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/ClassParameterDeclaration.h"
#include "bscript/compiler/ast/ClassParameterList.h"
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
#include "bscript/compiler/model/FunctionLink.h"

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
  std::vector<std::unique_ptr<ClassParameterDeclaration>> parameters;

  if ( auto function_parameters = ctx->classParameters() )
  {
    if ( auto param_list = function_parameters->classParameterList() )
    {
      for ( auto parameter_name : param_list->IDENTIFIER() )
      {
        auto baseclass_name = text( parameter_name );

        auto class_param_decl = std::make_unique<ClassParameterDeclaration>(
            location_for( *parameter_name ), baseclass_name );

        // Register with the FunctionResolver the class parameter's constructor
        // link. It will get resolved to the class constructor during the
        // second-pass AST visiting.
        workspace.function_resolver.register_function_link(
            ScopableName( baseclass_name, baseclass_name ), class_param_decl->constructor_link );

        parameters.push_back( std::move( class_param_decl ) );
      }
    }
  }

  std::vector<std::string> function_names;
  std::shared_ptr<FunctionLink> constructor_link = nullptr;

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

        // Check if the function is a constructor:
        // 1. The function name is the same as the class name.
        if ( func_name == class_name )
        {
          // 2. The function has parameters.
          if ( auto param_list = func_decl->functionParameters()->functionParameterList() )
          {
            for ( auto param : param_list->functionParameter() )
            {
              std::string parameter_name = text( param->IDENTIFIER() );

              // 3. The first parameter is named `this`.
              if ( Clib::caseInsensitiveEqual( parameter_name, "this" ) )
              {
                constructor_link = std::make_shared<FunctionLink>( func_loc, class_name,
                                                                   true /* requires_ctor */ );
                workspace.function_resolver.register_function_link(
                    ScopableName( class_name, class_name ), constructor_link );
              }
              break;
            }
          }
        }

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

  auto parameter_list =
      std::make_unique<ClassParameterList>( location_for( *ctx ), std::move( parameters ) );


  auto class_decl = std::make_unique<ClassDeclaration>(
      location_for( *ctx ), class_name, std::move( parameter_list ), std::move( function_names ),
      class_body, std::move( constructor_link ) );

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
