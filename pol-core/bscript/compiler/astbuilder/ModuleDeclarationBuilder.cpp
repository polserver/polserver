#include "ModuleDeclarationBuilder.h"

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/types/TypeNode.h"
#include "bscript/compiler/ast/types/TypeParameterList.h"
#include "bscript/compiler/model/ScopableName.h"

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
  auto type_params = type_parameter_list( location_for( *ctx ), ctx->typeParameters() );
  auto type_annotation = type_node( ctx->type() );

  if ( auto param_list = ctx->moduleFunctionParameterList() )
  {
    for ( auto param : param_list->moduleFunctionParameter() )
    {
      ScopableName parameter_name( ScopeName::None, text( param->IDENTIFIER() ) );
      std::unique_ptr<FunctionParameterDeclaration> parameter_declaration;
      auto param_type_annotation = type_node( param->typeAnnotation() );
      bool byref = false;
      bool unused = false;
      bool rest = false;

      if ( auto expr_ctx = param->expression() )
      {
        auto default_value = expression( expr_ctx );
        if ( param_type_annotation )
        {
          parameter_declaration = std::make_unique<FunctionParameterDeclaration>(
              location_for( *param ), std::move( parameter_name ), byref, unused, rest,
              std::move( default_value ), std::move( param_type_annotation ) );
        }
        else
        {
          parameter_declaration = std::make_unique<FunctionParameterDeclaration>(
              location_for( *param ), std::move( parameter_name ), byref, unused, rest,
              std::move( default_value ) );
        }
      }
      else
      {
        if ( param_type_annotation )
        {
          parameter_declaration = std::make_unique<FunctionParameterDeclaration>(
              location_for( *param ), std::move( parameter_name ), byref, unused, rest,
              std::move( param_type_annotation ) );
        }
        else
        {
          parameter_declaration = std::make_unique<FunctionParameterDeclaration>(
              location_for( *param ), std::move( parameter_name ), byref, unused, rest );
        }
      }
      parameters.push_back( std::move( parameter_declaration ) );
    }
  }

  auto source_location = location_for( *ctx );
  auto parameter_list =
      std::make_unique<FunctionParameterList>( source_location, std::move( parameters ) );
  return std::make_unique<ModuleFunctionDeclaration>(
      source_location, std::move( module_name ), std::move( name ), std::move( type_params ),
      std::move( parameter_list ), std::move( type_annotation ) );
}

}  // namespace Pol::Bscript::Compiler
