#include "bapplicobj.h"

#include <fmt/format.h>

namespace Pol::Bscript
{
std::string BApplicObjBase::getStringRep() const
{
  return fmt::format( "<appobj:{}>", typeOf() );
}
}  // namespace Pol::Bscript
