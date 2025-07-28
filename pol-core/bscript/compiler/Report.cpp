#include "Report.h"

#include "bscript/compiler/file/SourceLocation.h"
#include "clib/logfacility.h"

namespace Pol::Bscript::Compiler
{
Report::Report( bool display_warnings, bool display_errors, bool display_debugs )
    : display_warnings( display_warnings ),
      display_errors( display_errors ),
      display_debugs( display_debugs ),
      errors( 0 ),
      warnings( 0 )
{
}

void Report::report_error( const SourceLocation& source_location, const std::string& msg ) const
{
  try
  {
    ERROR_PRINTLN( "{}: error: {}", source_location, msg );
  }
  catch ( ... )
  {
  }
}

void Report::report_warning( const SourceLocation& source_location, const std::string& msg ) const
{
  try
  {
    ERROR_PRINTLN( "{}: warning: {}", source_location, msg );
  }
  catch ( ... )
  {
  }
}

void Report::report_debug( const SourceLocation& source_location, const std::string& msg ) const
{
  try
  {
    ERROR_PRINTLN( "{}: debug: {}", source_location, msg );
  }
  catch ( ... )
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

void Report::reset()
{
  errors = warnings = 0;
}


}  // namespace Pol::Bscript::Compiler
