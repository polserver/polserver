#include "Compiler.h"

#include "../clib/fileutil.h"
#include "../clib/logfacility.h"

#include "Report.h"
#include "compilercfg.h"
#include "format/CompiledScriptSerializer.h"
#include "representation/CompiledScript.h"

namespace Pol
{
namespace Bscript
{
namespace Compiler
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

void Compiler::compile_file_steps( const std::string& /*pathname*/,
                                   const LegacyFunctionOrder*,
                                   Report& )
{
}

void Compiler::display_outcome( const std::string& filename, Report& report )
{
  INFO_PRINT << filename << ": " << report.error_count() << " errors";
  if ( compilercfg.DisplayWarnings || compilercfg.ErrorOnWarning )
    INFO_PRINT << ", " << report.warning_count() << " warnings";
  INFO_PRINT << ".\n";
}

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol
