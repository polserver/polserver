#include "ModuleProcessor.h"

#include "BuilderWorkspace.h"
#include "compiler/Profile.h"
#include "compiler/file/SourceFile.h"

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
  }
  else if ( auto constStatement = ctx->constStatement() )
  {
  }
  return antlrcpp::Any();
}

antlrcpp::Any ModuleProcessor::visitUnambiguousModuleDeclarationStatement(
    EscriptGrammar::EscriptParser::UnambiguousModuleDeclarationStatementContext* ctx )
{
  if ( auto moduleFunctionDeclaration = ctx->unambiguousModuleFunctionDeclaration() )
  {
  }
  else if ( auto constStatement = ctx->unambiguousConstStatement() )
  {
  }
  return antlrcpp::Any();
}

}  // namespace Pol::Bscript::Compiler
