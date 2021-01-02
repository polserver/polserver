#include "Report.h"

namespace Pol::Bscript::Compiler
{
Report::Report() : errors( 0 ), warnings( 0 ) {}

unsigned Report::error_count() const
{
  return errors;
}

unsigned Report::warning_count() const
{
  return warnings;
}

}  // namespace Pol::Bscript::Compiler
