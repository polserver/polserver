#include "ConsoleReport.h"

#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "clib/logfacility.h"

namespace Pol::Bscript::Compiler
{
ConsoleReport::ConsoleReport( bool display_warnings ) : display_warnings( display_warnings ) {}

void ConsoleReport::report_error( const SourceLocation& source_location, const char* msg )
{
  ++errors;
  try
  {
    ERROR_PRINT << source_location << ": error: " << msg;
  }
  catch ( ... )
  {
  }
}

void ConsoleReport::report_warning( const SourceLocation& source_location, const char* msg )
{
  ++warnings;
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

}  // namespace Pol::Bscript::Compiler
