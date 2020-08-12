#include "ModuleDeclarationBuilder.h"

#include "compiler/ast/Expression.h"
#include "compiler/ast/FunctionParameterDeclaration.h"
#include "compiler/ast/FunctionParameterList.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
ModuleDeclarationBuilder::ModuleDeclarationBuilder(
    const SourceFileIdentifier& source_file_identifier, BuilderWorkspace& workspace )
    : SimpleStatementBuilder( source_file_identifier, workspace )
{
}

std::unique_ptr<ModuleFunctionDeclaration> ModuleDeclarationBuilder::module_function_declaration(
    EscriptParser::ModuleFunctionDeclarationContext* ctx, std::string module_name )
{
  std::string name = text( ctx->IDENTIFIER() );
  std::vector<std::unique_ptr<FunctionParameterDeclaration>> parameters;

  if ( auto param_list = ctx->moduleFunctionParameterList() )
  {
    for ( auto param : param_list->moduleFunctionParameter() )
    {
      std::string parameter_name = text( param->IDENTIFIER() );
      std::unique_ptr<FunctionParameterDeclaration> parameter_declaration;
      bool byref = false;
      bool unused = false;

//      if ( auto expr_ctx = param->expression() )
//      {
//        auto default_value = expression( expr_ctx );
//        parameter_declaration = std::make_unique<FunctionParameterDeclaration>(
//            location_for( *param ), std::move( parameter_name ), byref, unused,
//            std::move( default_value ) );
//      }
//      else
//      {
      parameter_declaration = std::make_unique<FunctionParameterDeclaration>(
          location_for( *param ), std::move( parameter_name ), byref, unused );
//      }
      parameters.push_back( std::move( parameter_declaration ) );
    }
  }

  auto source_location = location_for( *ctx );
  auto parameter_list =
      std::make_unique<FunctionParameterList>( source_location, std::move( parameters ) );
  return std::make_unique<ModuleFunctionDeclaration>(
      source_location, std::move( module_name ), std::move( name ), std::move( parameter_list ) );
}

std::unique_ptr<ModuleFunctionDeclaration> ModuleDeclarationBuilder::module_function_declaration(
    EscriptParser::UnambiguousModuleFunctionDeclarationContext* ctx, std::string module_name )
{
  std::string name = text( ctx->IDENTIFIER() );
  std::vector<std::unique_ptr<FunctionParameterDeclaration>> parameters;

  if ( auto param_list = ctx->unambiguousModuleFunctionParameterList() )
  {
    for ( auto param : param_list->unambiguousModuleFunctionParameter() )
    {
      std::string parameter_name = text( param->IDENTIFIER() );
      std::unique_ptr<FunctionParameterDeclaration> parameter_declaration;
      bool byref = false;
      bool unused = false;

//      if ( auto expr_ctx = param->unambiguousExpression() )
//      {
//        auto default_value = expression( expr_ctx );
//        parameter_declaration = std::make_unique<FunctionParameterDeclaration>(
//            location_for( *param ), std::move( parameter_name ), byref, unused,
//            std::move( default_value ) );
//      }
//      else
//      {
      parameter_declaration = std::make_unique<FunctionParameterDeclaration>(
          location_for( *param ), std::move( parameter_name ), byref, unused );
//      }
      parameters.push_back( std::move( parameter_declaration ) );
    }
  }

  auto loc = location_for( *ctx );
  auto parameter_list = std::make_unique<FunctionParameterList>( loc, std::move( parameters ) );
  return std::make_unique<ModuleFunctionDeclaration>(
      loc, std::move( module_name ), std::move( name ), std::move( parameter_list ) );
}

}  // namespace Pol::Bscript::Compiler
