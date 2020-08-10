#include "Compiler.h"

#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/timer.h"

#include "Report.h"
#include "Profile.h"
#include "Report.h"
#include "analyzer/SemanticAnalyzer.h"
#include "astbuilder/CompilerWorkspaceBuilder.h"
#include "compilercfg.h"
#include "format/CompiledScriptSerializer.h"
#include "model/CompilerWorkspace.h"
#include "optimizer/Optimizer.h"
#include "representation/CompiledScript.h"

namespace Pol::Bscript::Compiler
{
Compiler::Compiler( Profile& profile )
  : profile( profile )
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

void Compiler::write_listing( const std::string& /*pathname*/ )
{
}

void Compiler::write_dbg( const std::string& /*pathname*/, bool /*include_debug_text*/ )
{
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

  register_constants( *workspace );
  if ( report.error_count() )
    return;

  optimize( *workspace, report );
  if ( report.error_count() )
    return;

  analyze( *workspace, report);
  if ( report.error_count() )
    return;
}

std::unique_ptr<CompilerWorkspace> Compiler::build_workspace(
    const std::string& pathname, const LegacyFunctionOrder* legacy_function_order,
    Report& report )
{
  Pol::Tools::HighPerfTimer timer;
  CompilerWorkspaceBuilder workspace_builder( profile, report );
  auto workspace = workspace_builder.build( pathname, legacy_function_order );
  profile.build_workspace_micros += timer.ellapsed().count();
  return workspace;
}

void Compiler::register_constants( CompilerWorkspace& workspace )
{
  Pol::Tools::HighPerfTimer timer;
  SemanticAnalyzer::register_const_declarations( workspace );
  profile.register_const_declarations_micros += timer.ellapsed().count();
}

void Compiler::optimize( CompilerWorkspace& workspace, Report& report )
{
  Pol::Tools::HighPerfTimer timer;
  Optimizer optimizer( report );
  optimizer.optimize( workspace );
  profile.optimize_micros += timer.ellapsed().count();
}

void Compiler::analyze( CompilerWorkspace& workspace, Report& report )
{
  Pol::Tools::HighPerfTimer timer;
  SemanticAnalyzer analyzer( report );
  analyzer.analyze( workspace );
  profile.analyze_micros += timer.ellapsed().count();
}

void Compiler::display_outcome( const std::string& filename, Report& report )
{
  INFO_PRINT << filename << ": " << report.error_count() << " errors";
  if ( compilercfg.DisplayWarnings || compilercfg.ErrorOnWarning )
    INFO_PRINT << ", " << report.warning_count() << " warnings";
  INFO_PRINT << ".\n";
}

}  // namespace Pol::Bscript::Compiler
