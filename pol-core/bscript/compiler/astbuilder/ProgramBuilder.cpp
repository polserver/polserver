#include "ProgramBuilder.h"

#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/ProgramParameterDeclaration.h"
#include "bscript/compiler/ast/ProgramParameterList.h"
#include "bscript/compiler/ast/Statement.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
ProgramBuilder::ProgramBuilder( const SourceFileIdentifier& source_file_identifier,
                                BuilderWorkspace& workspace )
  : CompoundStatementBuilder( source_file_identifier, workspace )
{
}

std::unique_ptr<Program> ProgramBuilder::program( EscriptParser::ProgramDeclarationContext* ctx )
{
  std::vector<std::unique_ptr<ProgramParameterDeclaration>> parameter_declarations;

  if ( auto params = ctx->programParameters() )
  {
    if ( auto param_list = params->programParameterList() )
    {
      for ( auto param : param_list->programParameter() )
      {
        auto name = text( param->IDENTIFIER() );
        bool unused = param->UNUSED() != nullptr;
        parameter_declarations.push_back( std::make_unique<ProgramParameterDeclaration>(
            location_for( *param ), std::move( name ), unused ) );
      }
    }
  }
  auto parameter_list = std::make_unique<ProgramParameterList>(
      location_for( *ctx->programParameters() ), std::move( parameter_declarations ) );

  auto body =
      std::make_unique<FunctionBody>( location_for( *ctx ), block_statements( ctx->block() ) );

  return std::make_unique<Program>( location_for( *ctx ), std::move( parameter_list ),
                                    std::move( body ) );
}

}  // namespace Pol::Bscript::Compiler
