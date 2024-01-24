#include "SourceFileProcessor.h"

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/astbuilder/AvailableUserFunction.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/astbuilder/ModuleProcessor.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileCache.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/timer.h"
#include "compilercfg.h"
#include "plib/pkg.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
std::string getpathof( const std::string& fname );

SourceFileProcessor::SourceFileProcessor( const SourceFileIdentifier& source_file_identifier,
                                          BuilderWorkspace& workspace, bool is_src,
                                          UserFunctionInclusion user_function_inclusion )
    : profile( workspace.profile ),
      report( workspace.report ),
      source_file_identifier( source_file_identifier ),
      workspace( workspace ),
      tree_builder( source_file_identifier, workspace ),
      is_src( is_src ),
      user_function_inclusion( user_function_inclusion )
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
      static_cast<unsigned>(
          workspace.compiler_workspace.referenced_source_file_identifiers.size() ),
      pathname );

  Pol::Tools::HighPerfTimer load_timer;
  auto sf = workspace.em_cache.load( *ident, report );
  long long load_elapsed = load_timer.ellapsed().count();
  profile.load_em_micros += load_elapsed;
  if ( !sf )
  {
    // This is fatal because if we keep going, we'll likely report a bunch of errors
    // that would just be noise, like missing module function declarations or constants.
    // But in diagnostics mode, we want to continue, so...
    report.error( including_location, "Unable to use module '{}'.", module_name );
    // Move the failed-to-load SourceFileIdentifier to the compiler to properly
    // own diagnostic references.
    workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );
    return;
  }
  workspace.source_files[pathname] = sf;

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

  if ( workspace.continue_on_error || report.error_count() == 0 )
  {
    Pol::Tools::HighPerfTimer ast_timer;
    compilation_unit->accept( this );
    long long ast_us_elapsed = ast_timer.ellapsed().count();

    profile.ast_src_micros.fetch_add( ast_us_elapsed );
  }
}

void SourceFileProcessor::process_include( SourceFile& sf, long long* micros_counted )
{
  Pol::Tools::HighPerfTimer parse_timer;
  auto compilation_unit = sf.get_compilation_unit( report, source_file_identifier );
  profile.parse_inc_count++;
  long long parse_micros_elapsed = parse_timer.ellapsed().count();
  profile.parse_inc_micros.fetch_add( parse_micros_elapsed );
  *micros_counted += parse_micros_elapsed;

  if ( workspace.continue_on_error || report.error_count() == 0 )
  {
    Pol::Tools::HighPerfTimer ast_timer;
    compilation_unit->accept( this );
    long long ast_micros_elapsed = ast_timer.ellapsed().count();
    profile.ast_inc_micros.fetch_add( ast_micros_elapsed );
    *micros_counted += ast_micros_elapsed;
  }
}

void SourceFileProcessor::handle_include_declaration( EscriptParser::IncludeDeclarationContext* ctx,
                                                      long long* micros_counted )
{
  auto source_location = location_for( *ctx );

  std::string include_name;
  if ( auto string_literal = ctx->stringIdentifier()->STRING_LITERAL() )
    include_name = tree_builder.unquote( string_literal );
  else if ( auto identifier = ctx->stringIdentifier()->IDENTIFIER() )
    include_name = identifier->getSymbol()->getText();
  else if ( workspace.continue_on_error )
    return;
  else
    source_location.internal_error(
        "Unable to include module: expected a string literal or identifier.\n" );

  std::optional<std::string> maybe_canonical_include_pathname =
      locate_include_file( source_location, include_name );

  if ( !maybe_canonical_include_pathname )
  {
    report.error( source_location,
                  "Unable to include file '" + include_name + "': failed to locate." );
    return;
  }

  std::string canonical_include_pathname = maybe_canonical_include_pathname.value();

  if ( workspace.source_files.count( canonical_include_pathname ) == 0 )
  {
    auto ident = std::make_unique<SourceFileIdentifier>(
        static_cast<unsigned int>(
            workspace.compiler_workspace.referenced_source_file_identifiers.size() ),
        canonical_include_pathname );
    auto sf = workspace.inc_cache.load( *ident, report );
    if ( !sf )
    {
      report.error( source_location, "Unable to include file '{}': failed to load.",
                    canonical_include_pathname );
      workspace.compiler_workspace.referenced_source_file_identifiers.push_back(
          std::move( ident ) );
      return;
    }

    SourceFileProcessor include_processor( *ident, workspace, false, user_function_inclusion );
    workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );
    workspace.source_files[sf->pathname] = sf;

    include_processor.process_include( *sf, micros_counted );
  }
}

std::optional<std::string> SourceFileProcessor::locate_include_file(
    const SourceLocation& source_location, const std::string& include_name )
{
  std::string filename_part = include_name + ".inc";

  std::string current_file_path = getpathof( source_location.source_file_identifier->pathname );
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
          INFO_PRINTLN( "Searching for {}", filename_full );

        if ( !Clib::FileExists( filename_full.c_str() ) )
        {
          if ( compilercfg.VerbosityLevel >= 10 )
            INFO_PRINTLN( "Searching for {}", try_filename_full );
          if ( Clib::FileExists( try_filename_full.c_str() ) )
          {
            if ( compilercfg.VerbosityLevel >= 10 )
              INFO_PRINTLN( "Found {}", try_filename_full );

            filename_full = try_filename_full;
          }
        }
        else
        {
          if ( compilercfg.VerbosityLevel >= 10 )
            INFO_PRINTLN( "Found {}", filename_full );

          if ( Clib::FileExists( try_filename_full.c_str() ) )
            report.warning( source_location, "Found '{}' and '{}'! Will use first file!",
                            filename_full, try_filename_full );
        }
      }
      else
      {
        filename_full = compilercfg.PolScriptRoot + path;

        if ( compilercfg.VerbosityLevel >= 10 )
        {
          INFO_PRINTLN( "Searching for {}", filename_full );
          if ( Clib::FileExists( filename_full.c_str() ) )
            INFO_PRINTLN( "Found {}", filename_full );
        }
      }
    }
    else
    {
      // This is fatal because if we keep going, we'll likely report a bunch of errors
      // that would just be noise, like missing functions or constants.
      report.fatal( source_location, "Unable to read include file '{}'", include_name );
    }
  }
  else
  {
    if ( compilercfg.VerbosityLevel >= 10 )
      INFO_PRINTLN( "Searching for {}", filename_full );

    if ( !Clib::FileExists( filename_full.c_str() ) )
    {
      std::string try_filename_full = compilercfg.IncludeDirectory + filename_part;
      if ( compilercfg.VerbosityLevel >= 10 )
        INFO_PRINTLN( "Searching for {}", try_filename_full );
      if ( Clib::FileExists( try_filename_full.c_str() ) )
      {
        if ( compilercfg.VerbosityLevel >= 10 )
          INFO_PRINTLN( "Found {}", try_filename_full );

        // cout << "Using " << try_filename << endl;
        filename_full = try_filename_full;
      }
    }
    else
    {
      if ( compilercfg.VerbosityLevel >= 10 )
        INFO_PRINTLN( "Found {}", filename_full );
    }
  }
  if ( SourceFile::enforced_case_sensitivity_mismatch( source_location, filename_full, report ) )
  {
    return {};
  }

  filename_full = Clib::FullPath( filename_full.c_str() );

  if ( !filename_full.empty() )
    return std::optional<std::string>( filename_full );
  else
    return {};
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
  if ( auto identifier = ctx->IDENTIFIER() )
  {
    auto loc = location_for( *ctx );
    workspace.function_resolver.register_available_user_function( loc, ctx );
    const std::string& function_name = tree_builder.text( identifier );
    workspace.compiler_workspace.all_function_locations.emplace( function_name, loc );
    if ( user_function_inclusion == UserFunctionInclusion::All )
      workspace.function_resolver.force_reference( function_name, loc );
  }
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileProcessor::visitIncludeDeclaration(
    EscriptParser::IncludeDeclarationContext* ctx )
{
  long long micros_counted = 0;
  handle_include_declaration( ctx, &micros_counted );
  if ( is_src )
    profile.ast_src_micros.fetch_sub( micros_counted );
  else
    profile.ast_inc_micros.fetch_sub( micros_counted );
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileProcessor::visitProgramDeclaration(
    EscriptParser::ProgramDeclarationContext* ctx )
{
  if ( workspace.compiler_workspace.program )
  {
    report.error( location_for( *ctx ),
                  "Multiple program statements.\n"
                  "  Other declaration: {}",
                  workspace.compiler_workspace.program->source_location );
  }
  else if ( ctx->IDENTIFIER() )
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

std::string getpathof( const std::string& fname )
{
  std::string::size_type pos = fname.find_last_of( "\\/" );
  if ( pos == std::string::npos )
    return "./";
  else
    return fname.substr( 0, pos + 1 );
}
}  // namespace Pol::Bscript::Compiler
