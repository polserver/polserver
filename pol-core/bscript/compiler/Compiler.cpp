#include "Compiler.h"

#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/timer.h"
#include "compiler/Profile.h"
#include "compiler/Report.h"
#include "compiler/analyzer/Disambiguator.h"
#include "compiler/analyzer/SemanticAnalyzer.h"
#include "compiler/astbuilder/CompilerWorkspaceBuilder.h"
#include "compiler/codegen/CodeGenerator.h"
#include "compiler/file/SourceFileCache.h"
#include "compiler/format/CompiledScriptSerializer.h"
#include "compiler/format/DebugStoreSerializer.h"
#include "compiler/format/ListingWriter.h"
#include "compiler/model/CompilerWorkspace.h"
#include "compiler/optimizer/Optimizer.h"
#include "compiler/representation/CompiledScript.h"
#include "compilercfg.h"


namespace Pol::Bscript::Compiler
{
Compiler::Compiler( SourceFileCache& em_cache, SourceFileCache& inc_cache, Profile& profile )
  : em_cache( em_cache ),
    inc_cache( inc_cache ),
    profile( profile )
{
}

Compiler::~Compiler() = default;

bool Compiler::compile_file( const std::string& filename )
{
  return compile_file( filename, nullptr );
}

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

    DebugStoreSerializer(*output).write( ofs, text_ofs.get() );
  }
}

void Compiler::write_included_filenames( const std::string& /*pathname*/ )
{
}

bool Compiler::compile_file( const std::string& filename,
                             const LegacyFunctionOrder* legacy_function_order )
{
  bool success;
  try
  {
    auto pathname = Clib::FullPath( filename.c_str() );

    Report report( compilercfg.DisplayWarnings || compilercfg.ErrorOnWarning );

    compile_file_steps( pathname, legacy_function_order, report );
    display_outcome( pathname, report );

    bool have_warning_as_error = report.warning_count() && compilercfg.ErrorOnWarning;
    success = !report.error_count() && !have_warning_as_error;
  }
  catch ( std::exception& ex )
  {
    ERROR_PRINT << ex.what() << '\n';
    success = false;
  }
  return success;
}

void Compiler::compile_file_steps( const std::string& pathname,
                                   const LegacyFunctionOrder* legacy_function_order,
                                   Report& report )
{
  std::unique_ptr<CompilerWorkspace> workspace =
      build_workspace( pathname, legacy_function_order, report );
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

  analyze( *workspace, report);
  if ( report.error_count() )
    return;

  output = generate( std::move( workspace ), legacy_function_order );
}

std::unique_ptr<CompilerWorkspace> Compiler::build_workspace(
    const std::string& pathname, const LegacyFunctionOrder* legacy_function_order,
    Report& report )
{
  Pol::Tools::HighPerfTimer timer;
  CompilerWorkspaceBuilder workspace_builder( em_cache, inc_cache, profile, report );
  auto workspace = workspace_builder.build( pathname, legacy_function_order );
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
  optimizer.optimize( workspace );
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

std::unique_ptr<CompiledScript> Compiler::generate(
    std::unique_ptr<CompilerWorkspace> workspace, const LegacyFunctionOrder* legacy_function_order )
{
  Pol::Tools::HighPerfTimer codegen_timer;
  auto compiled_script = CodeGenerator::generate( std::move( workspace ), legacy_function_order );
  profile.codegen_micros += codegen_timer.ellapsed().count();
  return compiled_script;
}

void Compiler::display_outcome( const std::string& filename, Report& report )
{
  INFO_PRINT << filename << ": " << report.error_count() << " errors";
  if ( compilercfg.DisplayWarnings || compilercfg.ErrorOnWarning )
    INFO_PRINT << ", " << report.warning_count() << " warnings";
  INFO_PRINT << ".\n";
}

}  // namespace Pol::Bscript::Compiler
