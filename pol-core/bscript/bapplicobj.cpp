#include "bapplicobj.h"

#include <ostream>

namespace Pol::Bscript
{
std::string BApplicObjBase::getStringRep() const
{
  return std::string( "<appobj:" ) + typeOf() + ">";
}

void BApplicObjBase::printOn( std::ostream& os ) const
{
  os << getStringRep();
}
}  // namespace Pol::Bscript
