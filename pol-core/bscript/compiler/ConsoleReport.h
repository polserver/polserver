#ifndef POLSERVER_CONSOLEREPORT_H
#define POLSERVER_CONSOLEREPORT_H

#include "bscript/compiler/Report.h"
#include <format/format.h>


namespace Pol::Bscript::Compiler
{
class SourceLocation;

class ConsoleReport : public Report
{
public:
  explicit ConsoleReport( bool display_warnings );

private:
  const bool display_warnings;

  void report_error( const SourceLocation&, const char* msg );
  void report_warning( const SourceLocation&, const char* msg );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CONSOLEREPORT_H
