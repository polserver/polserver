#include "Disambiguator.h"

#include "compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
{
Disambiguator::Disambiguator( Report& report )
  : report( report )
{
}

void Disambiguator::disambiguate( CompilerWorkspace& )
{
}

}  // namespace Pol::Bscript::Compiler
