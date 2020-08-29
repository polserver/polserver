#include "Disambiguator.h"

#include "compiler/ast/Program.h"
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
  if ( auto program = workspace.program.get() )
  {
    program->accept( *this );
  }
}

}  // namespace Pol::Bscript::Compiler
