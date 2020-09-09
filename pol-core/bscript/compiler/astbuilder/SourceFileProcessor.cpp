#include "SourceFileProcessor.h"

#include "clib/fileutil.h"
#include "clib/timer.h"
#include "compiler/Profile.h"
#include "compiler/Report.h"
#include "compiler/ast/ConstDeclaration.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/Statement.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/astbuilder/AvailableUserFunction.h"
#include "compiler/astbuilder/BuilderWorkspace.h"
#include "compiler/astbuilder/ModuleProcessor.h"
#include "compiler/file/SourceFile.h"
#include "compiler/file/SourceFileCache.h"
#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/model/CompilerWorkspace.h"
#include "compilercfg.h"

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

void SourceFileProcessor::use_module( const std::string& module_name,
                                      SourceLocation& including_location,
                                      long long* micros_counted )
{
  std::string pathname = compilercfg.ModuleDirectory + module_name + ".em";

  if ( workspace.source_files.find( pathname ) != workspace.source_files.end() )
    return;

  auto ident = std::make_unique<SourceFileIdentifier>(
      workspace.compiler_workspace.referenced_source_file_identifiers.size(), pathname );

  Pol::Tools::HighPerfTimer load_timer;
  auto sf = workspace.em_cache.load( *ident, report );
  long long load_elapsed = load_timer.ellapsed().count();
  profile.load_em_micros += load_elapsed;
  if ( !sf )
  {
    // This is fatal because if we keep going, we'll likely report a bunch of errors
    // that would just be noise, like missing module function declarations or constants.
    report.fatal( including_location, "Unable to use module '", module_name, "'.\n" );
  }
  workspace.source_files[ pathname ] = sf;

  ModuleProcessor module_processor( *ident, workspace, module_name );
  workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );

  Pol::Tools::HighPerfTimer get_module_unit_timer;
  auto module_unit_context = sf->get_module_unit( report, source_file_identifier );
  long long parse_elapsed = get_module_unit_timer.ellapsed().count();
  profile.parse_em_micros += parse_elapsed;
  profile.parse_em_count++;

  Pol::Tools::HighPerfTimer visit_module_unit_timer;
  module_unit_context->accept( &module_processor );
  long long ast_elapsed = visit_module_unit_timer.ellapsed().count();
  profile.ast_em_micros.fetch_add( ast_elapsed );

  if ( micros_counted )
    *micros_counted = load_elapsed + parse_elapsed + ast_elapsed;
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

void SourceFileProcessor::handle_use_declaration( EscriptParser::UseDeclarationContext* ctx,
                                                  long long* micros_used )
{
  EscriptParser::StringIdentifierContext* stringId = ctx->stringIdentifier();
  std::string modulename = stringId->STRING_LITERAL()
                               ? tree_builder.unquote( stringId->STRING_LITERAL() )
                               : tree_builder.text( stringId->IDENTIFIER() );
  auto source_location = location_for( *ctx );
  use_module( modulename, source_location, micros_used );
}

antlrcpp::Any SourceFileProcessor::visitFunctionDeclaration(
    EscriptParser::FunctionDeclarationContext* ctx )
{
  workspace.function_resolver.register_available_user_function( location_for( *ctx ), ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileProcessor::visitProgramDeclaration(
    EscriptParser::ProgramDeclarationContext* ctx )
{
  if ( workspace.compiler_workspace.program )
  {
    report.error( location_for( *ctx ), "Multiple program statements.\n",
                  "  Other declaration: ", workspace.compiler_workspace.program->source_location,
                  "\n" );
  }
  else
  {
    workspace.compiler_workspace.program = tree_builder.program( ctx );
  }
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileProcessor::visitStatement( EscriptParser::StatementContext* ctx )
{
  if ( auto constStatement = ctx->constStatement() )
  {
    workspace.compiler_workspace.const_declarations.push_back(
        tree_builder.const_declaration( constStatement ) );
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

antlrcpp::Any SourceFileProcessor::visitUseDeclaration( EscriptParser::UseDeclarationContext* ctx )
{
  long long elapsed_micros = 0;
  handle_use_declaration( ctx, &elapsed_micros );
  if ( is_src )
    profile.ast_src_micros.fetch_sub( elapsed_micros );
  else
    profile.ast_inc_micros.fetch_sub( elapsed_micros );
  return antlrcpp::Any();
}

SourceLocation SourceFileProcessor::location_for( antlr4::ParserRuleContext& ctx ) const
{
  return { &source_file_identifier, ctx };
}

}  // namespace Pol::Bscript::Compiler
