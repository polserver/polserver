#include "CompilerWorkspace.h"

#include "bscript/compiler/analyzer/Constants.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/model/FlowControlLabel.h"

namespace Pol::Bscript::Compiler
{
CompilerWorkspace::CompilerWorkspace( Report& report, SourceFileCache& em_cache,
                                      SourceFileCache& inc_cache, Profile& profile )
    : constants( report ),
      scope_tree( *this ),
      builder_workspace( *this, em_cache, inc_cache, profile, report )
{
}

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

  for ( auto& class_decl : class_declarations )
  {
    class_decl->accept( visitor );
  }
}

}  // namespace Pol::Bscript::Compiler
