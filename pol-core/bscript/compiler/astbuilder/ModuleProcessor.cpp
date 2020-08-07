#include "ModuleProcessor.h"

#include "BuilderWorkspace.h"
#include "compiler/Profile.h"
#include "compiler/ast/ConstDeclaration.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
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
  else if ( auto constStatement = ctx->constStatement() )
  {
    workspace.compiler_workspace.const_declarations.push_back(
        tree_builder.const_declaration( constStatement ) );
  }
  return antlrcpp::Any();
}

antlrcpp::Any ModuleProcessor::visitUnambiguousModuleDeclarationStatement(
    EscriptGrammar::EscriptParser::UnambiguousModuleDeclarationStatementContext* ctx )
{
  if ( auto moduleFunctionDeclaration = ctx->unambiguousModuleFunctionDeclaration() )
  {
    auto ast =
        tree_builder.module_function_declaration( moduleFunctionDeclaration, modulename );
    workspace.function_resolver.register_module_function( ast.get() );
    workspace.compiler_workspace.module_function_declarations.push_back( std::move( ast ) );
  }
  else if ( auto constStatement = ctx->unambiguousConstStatement() )
  {
    workspace.compiler_workspace.const_declarations.push_back(
        tree_builder.const_declaration( constStatement ) );
  }
  return antlrcpp::Any();
}

}  // namespace Pol::Bscript::Compiler
