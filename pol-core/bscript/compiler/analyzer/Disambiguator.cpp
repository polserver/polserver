#include "Disambiguator.h"

#include "compiler/ast/TopLevelStatements.h"
#include "compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
{
Disambiguator::Disambiguator( Report& report )
  : report( report )
{
}

void Disambiguator::disambiguate( CompilerWorkspace& workspace )
{
  workspace.top_level_statements->accept( *this );
}

}  // namespace Pol::Bscript::Compiler
