#include "Report.h"

#include "clib/logfacility.h"
#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
Report::Report( bool display_warnings )
  : display_warnings( display_warnings ), errors( 0 ), warnings( 0 )
{
}

void Report::report_error( const SourceLocation& source_location, const char* msg )
{
  ++errors;
  try
  {
    ERROR_PRINT << source_location << ": error: " << msg;
  } catch (...)
  {
  }
}

void Report::report_warning( const SourceLocation& source_location, const char* msg )
{
  ++warnings;
  try
  {
    ERROR_PRINT << source_location << ": warning: " << msg;
  } catch (...)
  {
  }
}

unsigned Report::error_count() const
{
  return errors;
}

unsigned Report::warning_count() const
{
  return warnings;
}

void DiagnosticReport::report_error( const SourceLocation& loc, const char* msg )
{
  diagnostics.push_back( Diagnostic{ loc.source_file_identifier->pathname, loc.line_number,
                                     loc.character_column, Diagnostic::DiagnosticSeverity::Error,
                                     msg } );
  ERROR_PRINT << "report_error\n";
  errors++;
}

void DiagnosticReport::report_warning( const SourceLocation& loc, const char* msg )
{
  ERROR_PRINT << "report_warning\n";
  diagnostics.push_back( Diagnostic{ loc.source_file_identifier->pathname, loc.line_number,
                                     loc.character_column, Diagnostic::DiagnosticSeverity::Warning,
                                     msg } );
  warnings++;
}

}  // namespace Pol::Bscript::Compiler
