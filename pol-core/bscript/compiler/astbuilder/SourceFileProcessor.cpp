#include "SourceFileProcessor.h"

#include "../clib/fileutil.h"

#include "BuilderWorkspace.h"
#include "compiler/Profile.h"
#include "compiler/Report.h"
#include "compiler/file/SourceFile.h"
#include "compiler/file/SourceFileCache.h"
#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/model/CompilerWorkspace.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Legacy
{
std::string getpathof( const std::string& fname );
}
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

void SourceFileProcessor::process_source( SourceFile& )
{
  // INFO_PRINT << "Builder::process_source '" << sourcefile.pathname << "'\n";

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
    // INFO_PRINT << "top-level declaration: statement";
    tree_builder.add_statements( ctx, workspace.compiler_workspace.top_level_statements );
  }
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileProcessor::visitUnambiguousStatement(
    EscriptParser::UnambiguousStatementContext* ctx )
{
  if ( auto constStatement = ctx->unambiguousConstStatement() )
  {
  }
  else
  {
    // INFO_PRINT << "top-level declaration: statement";
    tree_builder.add_statements( ctx, workspace.top_level_statements );
  }
  return antlrcpp::Any();
}

SourceLocation SourceFileProcessor::location_for( antlr4::ParserRuleContext& ctx ) const
{
  return { &source_file_identifier, ctx };
}

}  // namespace Pol::Bscript::Compiler
