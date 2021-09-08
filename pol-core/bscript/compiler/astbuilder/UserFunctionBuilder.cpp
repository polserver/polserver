#include "UserFunctionBuilder.h"

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/UserFunction.h"

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
  if ( auto function_name_identifier = ctx->IDENTIFIER() )
  {
    std::string name = text( function_name_identifier );
    std::vector<std::unique_ptr<FunctionParameterDeclaration>> parameters;
    if ( auto function_parameters = ctx->functionParameters() )
    {
      if ( auto param_list = function_parameters->functionParameterList() )
      {
        for ( auto param : param_list->functionParameter() )
        {
          if ( auto param_identifier = param->IDENTIFIER() )
          {
            std::string parameter_name = text( param_identifier );
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
    }

    auto parameter_list =
        std::make_unique<FunctionParameterList>( location_for( *ctx ), std::move( parameters ) );
    auto body = std::make_unique<FunctionBody>(
        location_for( *ctx ),
        ctx->block() ? block_statements( ctx->block() )
                     : std::vector<std::unique_ptr<Pol::Bscript::Compiler::Statement>>() );

    bool exported = ctx->EXPORTED() != nullptr;
    return std::make_unique<UserFunction>( location_for( *ctx ), exported, std::move( name ),
                                           std::move( parameter_list ), std::move( body ),
                                           location_for( *ctx->ENDFUNCTION() ) );
  }

  // suppose it's okay to return nullptr since program is already optional for sources
  return {};
}

}  // namespace Pol::Bscript::Compiler
