#include "Report.h"

#include "../clib/logfacility.h"

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
  ERROR_PRINT << source_location << ": error: " << msg;
}

void Report::report_warning( const SourceLocation& source_location, const char* msg )
{
  ++warnings;
  ERROR_PRINT << source_location << ": warning: " << msg;
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
