#include "Report.h"

#include "clib/logfacility.h"
#include "bscript/compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
ConsoleReporter::ConsoleReporter( bool display_warnings )
    : ErrorReporter(), display_warnings( display_warnings )
{
}

void ConsoleReporter::report_error( const SourceLocation& source_location, const char* msg )
{
  try
  {
    ERROR_PRINT << source_location << ": error: " << msg;
  }
  catch ( ... )
  {
  }
}

void ConsoleReporter::report_warning( const SourceLocation& source_location, const char* msg )
{
  if ( display_warnings )
  {
    try
    {
      ERROR_PRINT << source_location << ": warning: " << msg;
    }
    catch ( ... )
    {
    }
  }
}

void DiagnosticReporter::report_error( const SourceLocation& source_location, const char* msg )
{
  auto len = strlen( msg );
  std::string message( msg, len > 0 ? len - 1 : len );
  diagnostics.push_back(
      Diagnostic{ Diagnostic::Severity::Error, source_location, std::move( message ) } );
}

void DiagnosticReporter::report_warning( const SourceLocation& source_location, const char* msg )
{
  auto len = strlen( msg );
  std::string message( msg, len > 0 ? len - 1 : len );
  diagnostics.push_back(
      Diagnostic{ Diagnostic::Severity::Warning, source_location, std::move( message ) } );
}

Report::Report( ErrorReporter& reporter ) : reporter( reporter ), errors( 0 ), warnings( 0 ) {}

void Report::report_error( const SourceLocation& source_location, const char* msg )
{
  ++errors;
  reporter.report_error( source_location, msg );
}

void Report::report_warning( const SourceLocation& source_location, const char* msg )
{
  ++warnings;
  reporter.report_warning( source_location, msg );
}

unsigned Report::error_count() const
{
  return errors;
}

unsigned Report::warning_count() const
{
  return warnings;
}

}  // namespace Pol::Bscript::Compiler
