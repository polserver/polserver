#include "ModuleProcessor.h"

#include "compiler/Profile.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/astbuilder/BuilderWorkspace.h"
#include "compiler/file/SourceFile.h"
#include "compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
{
ModuleProcessor::ModuleProcessor( const SourceFileIdentifier& source_file_identifier,
                                  BuilderWorkspace& workspace, std::string modulename )
  : profile( workspace.profile ),
    report( workspace.report ),
    source_file_identifier( source_file_identifier ),
    workspace( workspace ),
    tree_builder( source_file_identifier, workspace ),
    modulename( std::move( modulename ) )
{
}

antlrcpp::Any ModuleProcessor::visitModuleDeclarationStatement(
    EscriptGrammar::EscriptParser::ModuleDeclarationStatementContext* ctx )
{
  if ( auto moduleFunctionDeclaration = ctx->moduleFunctionDeclaration() )
  {
    auto ast =
        tree_builder.module_function_declaration( moduleFunctionDeclaration, modulename );

    workspace.function_resolver.register_module_function( ast.get() );

    workspace.compiler_workspace.module_function_declarations.push_back( std::move( ast ) );
  }
  return antlrcpp::Any();
}

}  // namespace Pol::Bscript::Compiler
