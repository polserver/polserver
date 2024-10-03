#include "Compiler.h"

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/analyzer/Disambiguator.h"
#include "bscript/compiler/analyzer/SemanticAnalyzer.h"
#include "bscript/compiler/astbuilder/CompilerWorkspaceBuilder.h"
#include "bscript/compiler/codegen/CodeGenerator.h"
#include "bscript/compiler/file/PrettifyBuilder.h"
#include "bscript/compiler/file/SourceFileCache.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/format/CompiledScriptSerializer.h"
#include "bscript/compiler/format/DebugStoreSerializer.h"
#include "bscript/compiler/format/ListingWriter.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "bscript/compiler/optimizer/Optimizer.h"
#include "bscript/compiler/representation/CompiledScript.h"
#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/timer.h"
#include "compilercfg.h"

namespace Pol::Bscript::Compiler
{
Compiler::Compiler( SourceFileCache& em_cache, SourceFileCache& inc_cache, Profile& profile )
    : em_cache( em_cache ), inc_cache( inc_cache ), profile( profile )
{
}

Compiler::~Compiler() = default;

bool Compiler::write_ecl( const std::string& pathname )
{
  if ( output )
  {
    CompiledScriptSerializer( *output ).write( pathname );
    return true;
  }
  else
  {
    return false;
  }
}

void Compiler::write_listing( const std::string& pathname )
{
  if ( output )
  {
    std::ofstream ofs( pathname );
    ListingWriter( *output ).write( ofs );
  }
}

void Compiler::write_dbg( const std::string& pathname, bool include_debug_text )
{
  if ( output )
  {
    std::ofstream ofs( pathname, std::ofstream::binary );
    auto text_ofs = include_debug_text ? std::make_unique<std::ofstream>( pathname + ".txt" )
                                       : std::unique_ptr<std::ofstream>();

    DebugStoreSerializer( *output ).write( ofs, text_ofs.get() );
  }
}

void Compiler::write_included_filenames( const std::string& pathname )
{
  if ( output )
  {
    std::ofstream ofs( pathname );
    for ( auto& r : output->source_file_identifiers )
    {
      ofs << r->pathname << "\n";
    }
  }
}

void Compiler::set_include_compile_mode()
{
  user_function_inclusion = UserFunctionInclusion::All;
}

bool Compiler::compile_file( const std::string& filename )
{
  bool success;
  try
  {
    auto pathname = Clib::FullPath( filename.c_str() );

    Report report( compilercfg.DisplayWarnings || compilercfg.ErrorOnWarning );

    compile_file_steps( pathname, report );
    display_outcome( pathname, report );

    bool have_warning_as_error = report.warning_count() && compilercfg.ErrorOnWarning;
    success = !report.error_count() && !have_warning_as_error;
  }
  catch ( std::exception& ex )
  {
    ERROR_PRINTLN( ex.what() );
    success = false;
  }
  return success;
}

void Compiler::compile_file_steps( const std::string& pathname, Report& report )
{
  std::unique_ptr<CompilerWorkspace> workspace = build_workspace( pathname, report );
  if ( report.error_count() )
    return;

  register_constants( *workspace, report );
  if ( report.error_count() )
    return;

  optimize( *workspace, report );
  if ( report.error_count() )
    return;

  disambiguate( *workspace, report );
  if ( report.error_count() )
    return;

  analyze( *workspace, report );
  if ( report.error_count() )
    return;

  output = generate( std::move( workspace ) );
}

bool Compiler::format_file( const std::string& filename, bool is_module, bool inplace )
{
  if ( !Clib::filesize( filename.c_str() ) )
    return true;

  Report report( false, true );
  PrettifyBuilder prettify_builder( profile, report );
  auto formatted = prettify_builder.build( filename, is_module );
  if ( report.error_count() )
    return false;
  if ( inplace )
  {
    std::ofstream filestream;
    filestream.open( filename, std::ios_base::out | std::ios_base::trunc | std::ios::binary );
    filestream << formatted;
    filestream.flush();
  }
  else
    INFO_PRINTLN( formatted );
  return true;
}

std::unique_ptr<CompilerWorkspace> Compiler::build_workspace( const std::string& pathname,
                                                              Report& report )
{
  Pol::Tools::HighPerfTimer timer;
  CompilerWorkspaceBuilder workspace_builder( em_cache, inc_cache, profile, report );
  auto workspace = workspace_builder.build( pathname, user_function_inclusion );
  profile.build_workspace_micros += timer.ellapsed().count();
  return workspace;
}

void Compiler::register_constants( CompilerWorkspace& workspace, Report& report )
{
  Pol::Tools::HighPerfTimer timer;
  SemanticAnalyzer::register_const_declarations( workspace, report );
  profile.register_const_declarations_micros += timer.ellapsed().count();
}

void Compiler::optimize( CompilerWorkspace& workspace, Report& report )
{
  Pol::Tools::HighPerfTimer timer;
  Optimizer optimizer( workspace.constants, report );
  optimizer.optimize( workspace, user_function_inclusion );
  profile.optimize_micros += timer.ellapsed().count();
}

void Compiler::disambiguate( CompilerWorkspace& workspace, Report& report )
{
  Pol::Tools::HighPerfTimer timer;
  Disambiguator disambiguator( workspace.constants, report );
  disambiguator.disambiguate( workspace );
  profile.disambiguate_micros += timer.ellapsed().count();
}

void Compiler::analyze( CompilerWorkspace& workspace, Report& report )
{
  Pol::Tools::HighPerfTimer timer;
  SemanticAnalyzer analyzer( workspace, report );
  analyzer.analyze();
  profile.analyze_micros += timer.ellapsed().count();
}

std::unique_ptr<CompiledScript> Compiler::generate( std::unique_ptr<CompilerWorkspace> workspace )
{
  Pol::Tools::HighPerfTimer codegen_timer;
  auto compiled_script = CodeGenerator::generate( std::move( workspace ) );
  profile.codegen_micros += codegen_timer.ellapsed().count();
  return compiled_script;
}

void Compiler::display_outcome( const std::string& filename, Report& report )
{
  auto msg = fmt::format( "{}: {} errors", filename, report.error_count() );
  if ( compilercfg.DisplayWarnings || compilercfg.ErrorOnWarning )
    msg += fmt::format( ", {} warnings", report.warning_count() );
  INFO_PRINTLN( msg + '.' );
}

}  // namespace Pol::Bscript::Compiler
