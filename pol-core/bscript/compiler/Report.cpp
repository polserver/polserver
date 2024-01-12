#include "Report.h"

#include "bscript/compiler/file/SourceLocation.h"
#include "clib/logfacility.h"

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
    ERROR_PRINTLN( "{}: error: {}", source_location, msg );
  }
  catch ( ... )
  {
  }
}

void Report::report_warning( const SourceLocation& source_location, const char* msg )
{
  ++warnings;
  try
  {
    ERROR_PRINTLN( "{}: warning: {}", source_location, msg );
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

}  // namespace Pol::Bscript::Compiler
