#include "ModuleProcessor.h"

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
{
ModuleProcessor::ModuleProcessor( const SourceFileIdentifier& source_file_identifier,
                                  BuilderWorkspace& workspace, std::string modulename )
  : workspace( workspace ),
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
    workspace.compiler_workspace.all_function_locations.emplace(ast->name, ast->source_location);
    workspace.compiler_workspace.module_function_declarations.push_back( std::move( ast ) );
  }
  else if ( auto constStatement = ctx->constStatement() )
  {
    workspace.compiler_workspace.const_declarations.push_back(
        tree_builder.const_declaration( constStatement ) );
  }
  return antlrcpp::Any();
}

}  // namespace Pol::Bscript::Compiler
