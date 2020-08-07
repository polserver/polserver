#include "SourceFileProcessor.h"

#include "../clib/fileutil.h"

#include "BuilderWorkspace.h"
#include "ModuleProcessor.h"
#include "compiler/Profile.h"
#include "compiler/Report.h"
#include "compiler/ast/Statement.h"
#include "compiler/ast/TopLevelStatements.h"
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

long long SourceFileProcessor::use_module( const std::string& module_name,
                                           SourceLocation& including_location )
{
  if ( workspace.used_modules.find( module_name ) != workspace.used_modules.end() )
    return 0;

  workspace.used_modules.insert( module_name );

  std::string pathname = compilercfg.ModuleDirectory + module_name + ".em";

  auto ident = std::make_unique<SourceFileIdentifier>(
      workspace.compiler_workspace.referenced_source_file_identifiers.size(), pathname );

  auto sf = workspace.em_cache.load( *ident, report );
  if ( !sf )
  {
    // This is fatal because if we keep going, we'll likely report a bunch of errors
    // that would just be noise, like missing module function declarations or constants.
    report.fatal( including_location, "Unable to use module '", module_name, "'.\n" );
  }

  ModuleProcessor module_processor( *ident, workspace, module_name );
  workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );

  long long micros_counted = 0;
  module_processor.process_module( &micros_counted, *sf );
  return micros_counted;
}

void SourceFileProcessor::process_source( SourceFile& sf )
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

antlrcpp::Any SourceFileProcessor::visitUnambiguousStatement(
    EscriptParser::UnambiguousStatementContext* ctx )
{
  if ( auto constStatement = ctx->unambiguousConstStatement() )
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
