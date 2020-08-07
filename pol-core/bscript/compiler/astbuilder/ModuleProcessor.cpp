#include "ModuleProcessor.h"

#include "../clib/timer.h"

#include "BuilderWorkspace.h"
#include "DeclarationBuilder.h"
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

void ModuleProcessor::process_module( long long* micros_counted, SourceFile& sf )
{
  Pol::Tools::HighPerfTimer get_module_unit_timer;
  auto module_unit_context = sf.get_module_unit( report, source_file_identifier );
  long long parse_elapsed = get_module_unit_timer.ellapsed().count();
  profile.parse_em_micros += parse_elapsed;
  profile.parse_em_count++;

  Pol::Tools::HighPerfTimer load_module_unit_timer;
  module_unit_context->accept( this );
  long long ast_elapsed = load_module_unit_timer.ellapsed().count();
  profile.ast_em_micros.fetch_add( ast_elapsed );

  *micros_counted = parse_elapsed + ast_elapsed;
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
