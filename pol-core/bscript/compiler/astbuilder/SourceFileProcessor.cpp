#include "SourceFileProcessor.h"

#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/timer.h"
#include "../plib/pkg.h"

#include "AvailableUserFunction.h"
#include "BuilderWorkspace.h"
#include "ModuleProcessor.h"
#include "compiler/Profile.h"
#include "compiler/Report.h"
#include "compiler/ast/Statement.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/ast/ConstDeclaration.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/UserFunction.h"
#include "compiler/file/SourceFile.h"
#include "compiler/file/SourceFileCache.h"
#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/model/CompilerWorkspace.h"
#include "compilercfg.h"

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

void SourceFileProcessor::use_module( const std::string& module_name,
                                      SourceLocation& including_location,
                                      long long* micros_counted )
{
  if ( workspace.used_modules.find( module_name ) != workspace.used_modules.end() )
    return;

  workspace.used_modules.insert( module_name );

  std::string pathname = compilercfg.ModuleDirectory + module_name + ".em";

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

void SourceFileProcessor::process_include( long long* us_counted, SourceFile& sf )
{
  Pol::Tools::HighPerfTimer timer0;
  auto compilation_unit = sf.get_compilation_unit( report, source_file_identifier );
  // INFO_PRINT << "compilation unit:\n" << compilationUnit->toStringTree( true ) << "\n";
  long long parse_us_counted = timer0.ellapsed().count();
  profile.parse_inc_us.fetch_add( parse_us_counted );
  profile.parse_inc_count++;

  *us_counted += parse_us_counted;

  if ( report.error_count() == 0 )
  {
    Pol::Tools::HighPerfTimer timer;
    compilation_unit->accept( this );
    long long ast_us_elapsed = timer.ellapsed().count();
    profile.ast_inc_us.fetch_add( ast_us_elapsed );
    *us_counted += ast_us_elapsed;
  }
}

void SourceFileProcessor::handle_include_declaration( EscriptParser::IncludeDeclarationContext* ctx,
                                                      long long* us_counted )
{
  auto source_location = location_for( *ctx );

  std::string include_name;
  if ( auto string_literal = ctx->stringIdentifier()->STRING_LITERAL() )
    include_name = tree_builder.unquote( string_literal );
  else if ( auto identifier = ctx->stringIdentifier()->IDENTIFIER() )
    include_name = identifier->getSymbol()->getText();
  else
    source_location.internal_error(
        "Unable to include module: expected a string literal or identifier.\n" );

  boost::optional<std::string> maybe_canonical_include_pathname =
      locate_include_file( source_location, include_name );

  if ( !maybe_canonical_include_pathname )
  {
    report.error( source_location,
                  "Unable to include file '" + include_name + "': failed to locate.\n" );
    return;
  }

  std::string canonical_include_pathname = maybe_canonical_include_pathname.value();

  if ( workspace.source_files.count( canonical_include_pathname ) == 0 )
  {
    auto ident = std::make_unique<SourceFileIdentifier>(
        workspace.compiler_workspace.referenced_source_file_identifiers.size(),
        canonical_include_pathname );
    auto sf = workspace.inc_cache.load( *ident, report );
    if ( !sf )
    {
      report.error( source_location, "Unable to include file '", canonical_include_pathname,
                    "': failed to load.\n" );
      return;
    }

    SourceFileProcessor include_processor( *ident, workspace, false );
    workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );
    workspace.source_files[sf->pathname] = sf;

    include_processor.process_include( us_counted, *sf );
  }
}

boost::optional<std::string> SourceFileProcessor::locate_include_file(
    const SourceLocation& source_location, const std::string& include_name )
{
  std::string filename_part = include_name + ".inc";

  // ERROR_PRINT << "handle_include_declaration modulename=" << include_name << "\n";

  std::string current_file_path =
      Pol::Bscript::Legacy::getpathof( source_location.source_file_identifier->pathname );
  std::string filename_full = current_file_path + filename_part;

  if ( filename_part[0] == ':' )
  {
    const Plib::Package* pkg = nullptr;
    std::string path;
    if ( Plib::pkgdef_split( filename_part, nullptr, &pkg, &path ) )
    {
      if ( pkg != nullptr )
      {
        filename_full = pkg->dir() + path;
        std::string try_filename_full = pkg->dir() + "include/" + path;

        if ( compilercfg.VerbosityLevel >= 10 )
          INFO_PRINT << "Searching for " << filename_full << "\n";

        if ( !Clib::FileExists( filename_full.c_str() ) )
        {
          if ( compilercfg.VerbosityLevel >= 10 )
            INFO_PRINT << "Searching for " << try_filename_full << "\n";
          if ( Clib::FileExists( try_filename_full.c_str() ) )
          {
            if ( compilercfg.VerbosityLevel >= 10 )
              INFO_PRINT << "Found " << try_filename_full << "\n";

            filename_full = try_filename_full;
          }
        }
        else
        {
          if ( compilercfg.VerbosityLevel >= 10 )
            INFO_PRINT << "Found " << filename_full << "\n";

          if ( Clib::FileExists( try_filename_full.c_str() ) )
            report.warning( source_location, "Found '", filename_full, "' and '", try_filename_full,
                            "'! Will use first file!\n" );
        }
      }
      else
      {
        filename_full = compilercfg.PolScriptRoot + path;

        if ( compilercfg.VerbosityLevel >= 10 )
        {
          INFO_PRINT << "Searching for " << filename_full << "\n";
          if ( Clib::FileExists( filename_full.c_str() ) )
            INFO_PRINT << "Found " << filename_full << "\n";
        }
      }
    }
    else
    {
      // This is fatal because if we keep going, we'll likely report a bunch of errors
      // that would just be noise, like missing functions or constants.
      report.fatal( source_location, "Unable to read include file '" + include_name + "'\n" );
    }
  }
  else
  {
    if ( compilercfg.VerbosityLevel >= 10 )
      INFO_PRINT << "Searching for " << filename_full << "\n";

    if ( !Clib::FileExists( filename_full.c_str() ) )
    {
      std::string try_filename_full = compilercfg.IncludeDirectory + filename_part;
      if ( compilercfg.VerbosityLevel >= 10 )
        INFO_PRINT << "Searching for " << try_filename_full << "\n";
      if ( Clib::FileExists( try_filename_full.c_str() ) )
      {
        if ( compilercfg.VerbosityLevel >= 10 )
          INFO_PRINT << "Found " << try_filename_full << "\n";

        // cout << "Using " << try_filename << endl;
        filename_full = try_filename_full;
      }
    }
    else
    {
      if ( compilercfg.VerbosityLevel >= 10 )
        INFO_PRINT << "Found " << filename_full << "\n";
    }
  }
  // ERROR_PRINT << "filename_full is (1) " << filename_full << "\n";

  if ( SourceFile::enforced_case_sensitivity_mismatch( source_location, filename_full, report ) )
  {
    return {};
  }

  filename_full = Clib::FullPath( filename_full.c_str() );
  // ERROR_PRINT << "filename_full is (2) " << filename_full << "\n";

  return boost::optional<std::string>( !filename_full.empty(), filename_full );
}

long long SourceFileProcessor::handle_use_declaration( EscriptParser::UseDeclarationContext* ctx )
{
  std::string modulename =
      ( ctx->stringIdentifier()->STRING_LITERAL() ? ctx->stringIdentifier()->STRING_LITERAL()
                                                  : ctx->stringIdentifier()->IDENTIFIER() )
          ->getSymbol()
          ->getText();
  if ( modulename[0] == '\"' )
    modulename = modulename.substr( 1, modulename.length() - 2 );
  // INFO_PRINT << "  - " << modulename << "\n";
  auto source_location = location_for( *ctx );
  long long micros_used = 0;
  use_module( modulename, source_location, &micros_used );
  return micros_used;
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

antlrcpp::Any SourceFileProcessor::visitUnambiguousStatement(
    EscriptParser::UnambiguousStatementContext* ctx )
{
  if ( auto constStatement = ctx->unambiguousConstStatement() )
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

antlrcpp::Any SourceFileProcessor::visitProgramDeclaration(
    EscriptParser::ProgramDeclarationContext* ctx )
{
  //  << "top-level declaration: program\n";
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

antlrcpp::Any SourceFileProcessor::visitUnambiguousProgramDeclaration(
    EscriptParser::UnambiguousProgramDeclarationContext* ctx )
{
  //  << "top-level declaration: program\n";
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

antlrcpp::Any SourceFileProcessor::visitIncludeDeclaration(
    EscriptParser::IncludeDeclarationContext* ctx )
{
  long long us_counted = 0;
  handle_include_declaration( ctx, &us_counted );
  if ( is_src )
    profile.ast_src_micros.fetch_sub( us_counted );
  else
    profile.ast_inc_us.fetch_sub( us_counted );
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileProcessor::visitUseDeclaration( EscriptParser::UseDeclarationContext* ctx )
{
  long long elapsed = handle_use_declaration( ctx );
  if ( is_src )
    profile.ast_src_micros.fetch_sub( elapsed );
  else
    profile.ast_inc_us.fetch_sub( elapsed );
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileProcessor::visitFunctionDeclaration(
    EscriptParser::FunctionDeclarationContext* ctx )
{
  workspace.function_resolver.register_available_user_function( location_for( *ctx ), ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileProcessor::visitUnambiguousFunctionDeclaration(
    EscriptParser::UnambiguousFunctionDeclarationContext* ctx )
{
  workspace.function_resolver.register_available_user_function( location_for( *ctx ), ctx );
  return antlrcpp::Any();
}

SourceLocation SourceFileProcessor::location_for( antlr4::ParserRuleContext& ctx ) const
{
  return { &source_file_identifier, ctx };
}

}  // namespace Pol::Bscript::Compiler
