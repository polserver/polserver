#include "UserFunctionBuilder.h"

#include "bscript/compiler/Report.h"
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
#include "bscript/compiler/model/ClassLink.h"
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

  if ( Clib::caseInsensitiveEqual( class_name, "super" ) )
  {
    workspace.report.error( location_for( *ctx->IDENTIFIER() ),
                            "The class name 'super' is reserved." );
    return nullptr;
  }

  std::vector<std::unique_ptr<ClassParameterDeclaration>> parameters;
  std::vector<std::shared_ptr<ClassLink>> base_classes;
  std::vector<std::string> method_names;
  std::unique_ptr<FunctionLink> constructor_link;
  bool is_child = false;

  // True if the function and class name are equal. "Maybe" because it may not have `this`
  // as a first parameter.
  bool maybe_has_ctor = false;

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
        base_classes.push_back(
            std::make_shared<ClassLink>( location_for( *parameter_name ), baseclass_name ) );

        workspace.function_resolver.register_class_link( ScopeName( baseclass_name ),
                                                         base_classes.back() );

        workspace.function_resolver.register_function_link(
            ScopableName( baseclass_name, baseclass_name ), class_param_decl->constructor_link );

        is_child = true;
        parameters.push_back( std::move( class_param_decl ) );
      }
    }
  }

  if ( auto classBody = ctx->classBody() )
  {
    for ( auto classStatement : classBody->classStatement() )
    {
      if ( auto func_decl = classStatement->functionDeclaration() )
      {
        auto func_name = text( func_decl->IDENTIFIER() );
        auto func_loc = location_for( *func_decl );

        // Register the user function as an available parse tree only if it is not `super` for child
        // classes.
        auto is_super = Clib::caseInsensitiveEqual( func_name, "super" );

        maybe_has_ctor |= Clib::caseInsensitiveEqual( func_name, class_name );

        if ( is_super && is_child )
        {
          workspace.report.error( func_loc, "The 'super' function is reserved for child classes." );
        }
        else
        {
          workspace.function_resolver.register_available_scoped_function( func_loc, class_name,
                                                                          func_decl );
        }


        // Check if the function is a constructor:
        // 1. The function has parameters.
        if ( auto param_list = func_decl->functionParameters()->functionParameterList() )
        {
          if ( auto func_params = param_list->functionParameter(); !func_params.empty() )
          {
            std::string parameter_name = text( func_params.front()->IDENTIFIER() );

            // 2. The first parameter is named `this`.
            if ( Clib::caseInsensitiveEqual( parameter_name, "this" ) )
            {
              // 3. The function name is the same as the class name: constructor
              if ( Clib::caseInsensitiveEqual( func_name, class_name ) )
              {
                constructor_link = std::make_unique<FunctionLink>( func_loc, class_name,
                                                                   true /* requires_ctor */ );
              }
              // 3b. Otherwise: method
              else
              {
                method_names.push_back( func_name );
              }
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
      location_for( *ctx ), class_name, std::move( parameter_list ), std::move( constructor_link ),
      std::move( method_names ), class_body, std::move( base_classes ) );

  // Only register the ClassDeclaration's ctor FunctionLink if there _is_ a ctor.
  if ( class_decl->constructor_link )
  {
    workspace.function_resolver.register_function_link( ScopableName( class_name, class_name ),
                                                        class_decl->constructor_link );

    // Only register the super() function if the class is a child.
    if ( is_child )
    {
      workspace.function_resolver.register_available_generated_function(
          location_for( *ctx ), ScopableName( class_name, "super" ), class_decl.get(),
          UserFunctionType::Super );
    }
  }
  // Can only create a constructor if (1) there is no function already defined
  // with the class name (regardless if it is an actual constructor or not) and
  // (2) there are parameters.
  else if ( !maybe_has_ctor && class_decl->parameters().size() > 0 )
  {
    workspace.function_resolver.register_available_generated_function(
        location_for( *ctx ), ScopableName( class_name, class_name ), class_decl.get(),
        UserFunctionType::Constructor );
  }

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
        ScopableName parameter_name( ScopeName::None, text( param->IDENTIFIER() ) );
        bool is_this_arg = false;

        if ( first )
        {
          if ( Clib::caseInsensitiveEqual( parameter_name.string(), "this" ) )
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

  constexpr bool expression =
      std::is_same<ParserContext, EscriptGrammar::EscriptParser::FunctionExpressionContext>::value;

  bool constructor_method = class_method && Clib::caseInsensitiveEqual( name, class_name );

  UserFunctionType type = !class_method        ? UserFunctionType::Static
                          : constructor_method ? UserFunctionType::Constructor
                                               : UserFunctionType::Method;

  in_constructor_function.push( type == UserFunctionType::Constructor );

  auto body =
      std::make_unique<FunctionBody>( location_for( *ctx ), block_statements( ctx->block() ) );

  in_constructor_function.pop();

  std::shared_ptr<ClassLink> class_link;
  if ( !class_name.empty() )
  {
    class_link = std::make_shared<ClassLink>( location_for( *ctx ), class_name );
    workspace.function_resolver.register_class_link( ScopeName( class_name ), class_link );
    auto cd = class_link->class_declaration();

    // Should never happen, since the only reason this user function can be
    // visited is because the class has been registered.
    if ( !cd )
      class_link->source_location.internal_error( "ClassLink has no ClassDeclaration" );
  }

  return std::make_unique<UserFunction>( location_for( *ctx ), exported, expression, type,
                                         class_name, std::move( name ), std::move( parameter_list ),
                                         std::move( body ), location_for( *end_token ),
                                         std::move( class_link ) );
}
}  // namespace Pol::Bscript::Compiler
