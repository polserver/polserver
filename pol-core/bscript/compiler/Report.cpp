#include "Report.h"

#include "bscript/compiler/file/SourceLocation.h"
#include "clib/logfacility.h"

namespace Pol::Bscript::Compiler
{
ConsoleReporter::ConsoleReporter( bool display_warnings, bool display_errors, bool display_debugs )
    : ErrorReporter(),
      display_warnings( display_warnings ),
      display_errors( display_errors ),
      display_debugs( display_debugs )
{
}

void ConsoleReporter::report_error( const SourceLocation& source_location, const std::string& msg )
{
  if ( display_errors )
  {
    try
    {
      ERROR_PRINTLN( "{}: error: {}", source_location, msg );
    }
    catch ( ... )
    {
    }
  }
}

void ConsoleReporter::report_warning( const SourceLocation& source_location,
                                      const std::string& msg )
{
  if ( display_warnings )
  {
    try
    {
      ERROR_PRINTLN( "{}: warning: {}", source_location, msg );
    }
    catch ( ... )
    {
    }
  }
}

void ConsoleReporter::report_debug( const SourceLocation& source_location, const std::string& msg )
{
  if ( display_debugs )
  {
    try
    {
      ERROR_PRINTLN( "{}: debug: {}", source_location, msg );
    }
    catch ( ... )
    {
    }
  }
}

void ConsoleReporter::clear() {}

void DiagnosticReporter::report_error( const SourceLocation& source_location,
                                       const std::string& msg )
{
  diagnostics.push_back( Diagnostic{ Diagnostic::Severity::Error, source_location, msg } );
}

void DiagnosticReporter::report_warning( const SourceLocation& source_location,
                                         const std::string& msg )
{
  diagnostics.push_back( Diagnostic{ Diagnostic::Severity::Warning, source_location, msg } );
}

void DiagnosticReporter::report_debug( const SourceLocation&, const std::string& ) {}

void DiagnosticReporter::clear()
{
  diagnostics.clear();
}

Report::Report( ErrorReporter& reporter ) : reporter( reporter ), errors( 0 ), warnings( 0 ) {}

void Report::report_error( const SourceLocation& source_location, const std::string& msg ) const
{
  reporter.report_error( source_location, msg );
}

void Report::report_warning( const SourceLocation& source_location, const std::string& msg ) const
{
  reporter.report_warning( source_location, msg );
}

void Report::clear()
{
  warnings = errors = 0;
  reporter.clear();
}

void Report::report_debug( const SourceLocation& source_location, const std::string& msg ) const
{
  reporter.report_debug( source_location, msg );
}

unsigned Report::error_count() const
{
  return errors;
}

unsigned Report::warning_count() const
{
  return warnings;
}

void Report::reset()
{
  errors = warnings = 0;
}


}  // namespace Pol::Bscript::Compiler
