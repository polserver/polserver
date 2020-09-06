#include "CompilerWorkspace.h"

#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/analyzer/Constants.h"
#include "compiler/ast/ConstDeclaration.h"
#include "compiler/ast/UserFunction.h"
#include "compiler/file/SourceFileIdentifier.h"

namespace Pol::Bscript::Compiler
{
CompilerWorkspace::CompilerWorkspace( Report& report ) : constants( report ) {}

CompilerWorkspace::~CompilerWorkspace() = default;

/*
 * This method is the main argument for making this class into an ast/Node.
 * However, it has only one caller: the Optimizer.
 * Other classes visit parts of the tree or do things in between visiting parts.
 */
void CompilerWorkspace::accept( NodeVisitor& visitor )
{
  for ( auto& constant : const_declarations )
  {
    constant->accept( visitor );
  }

  for ( auto& module_function : module_function_declarations )
  {
    module_function->accept( visitor );
  }

  top_level_statements->accept( visitor );

  if ( program )
  {
    program->accept( visitor );
  }

  for ( auto& user_function : user_functions )
  {
    user_function->accept( visitor );
  }
}

}  // namespace Pol::Bscript::Compiler
