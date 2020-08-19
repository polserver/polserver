#include "SourceFileProcessor.h"

#include "clib/fileutil.h"
#include "clib/timer.h"
#include "compiler/astbuilder/BuilderWorkspace.h"
#include "compiler/Profile.h"
#include "compiler/Report.h"
#include "compiler/ast/Statement.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/file/SourceFile.h"
#include "compiler/file/SourceFileCache.h"
#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/model/CompilerWorkspace.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
SourceFileProcessor::SourceFileProcessor( const SourceFileIdentifier& source_file_identifier,
                                          BuilderWorkspace& workspace, bool is_src )
  : profile( workspace.profile ),
    report( workspace.report ),
    source_file_identifier( source_file_identifier ),
    workspace( workspace ),
    tree_builder( source_file_identifier, workspace ),
    is_src( is_src )
{
}

void SourceFileProcessor::process_source( SourceFile& sf )
{
  Pol::Tools::HighPerfTimer parse_timer;
  auto compilation_unit = sf.get_compilation_unit( report, source_file_identifier );
  long long parse_us_counted = parse_timer.ellapsed().count();

  profile.parse_src_micros.fetch_add( parse_us_counted );
  profile.parse_src_count++;

  if ( report.error_count() == 0 )
  {
    Pol::Tools::HighPerfTimer ast_timer;
    compilation_unit->accept( this );
    long long ast_us_elapsed = ast_timer.ellapsed().count();

    profile.ast_src_micros.fetch_add( ast_us_elapsed );
  }
}

antlrcpp::Any SourceFileProcessor::visitStatement( EscriptParser::StatementContext* ctx )
{
  if ( auto constStatement = ctx->constStatement() )
  {
  }
  else
  {
    std::vector<std::unique_ptr<Statement>> statements;
    tree_builder.add_statements( ctx, statements );
    for ( auto& statement : statements )
    {
      workspace.compiler_workspace.top_level_statements->children.push_back(
          std::move( statement ) );
    }
  }
  return antlrcpp::Any();
}

SourceLocation SourceFileProcessor::location_for( antlr4::ParserRuleContext& ctx ) const
{
  return { &source_file_identifier, ctx };
}

}  // namespace Pol::Bscript::Compiler
