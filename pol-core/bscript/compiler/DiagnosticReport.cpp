#include "DiagnosticReport.h"

#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "clib/logfacility.h"

namespace Pol::Bscript::Compiler
{
DiagnosticReport::DiagnosticReport() : Report() {}

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
