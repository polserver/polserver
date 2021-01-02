#ifndef POLSERVER_DIAGNOSTICREPORT_H
#define POLSERVER_DIAGNOSTICREPORT_H

#include <format/format.h>

#include "bscript/compiler/Report.h"

namespace Pol::Bscript::Compiler
{
class SourceLocation;

struct Diagnostic
{
  std::string file;
  unsigned line;
  unsigned character;
  enum class DiagnosticSeverity
  {
    Warning,
    Error
  } severity;
  std::string message;
};

class DiagnosticReport : public Report
{
public:
  DiagnosticReport();
  std::vector<Diagnostic> diagnostics;

private:
  void report_error( const SourceLocation&, const char* msg ) override;
  void report_warning( const SourceLocation&, const char* msg ) override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_REPORT_H
