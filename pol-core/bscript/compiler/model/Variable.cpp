#include "Variable.h"

#include <utility>

namespace Pol::Bscript::Compiler
{
Variable::Variable( VariableScope scope, std::string name, BlockDepth block_depth,
                    VariableIndex index, WarnOn warn_on, std::shared_ptr<Variable> capturing,
                    const SourceLocation& source_location, const SourceLocation& var_decl_location )
    : scope( scope ),
      name( std::move( name ) ),
      block_depth( block_depth ),
      index( index ),
      warn_on( warn_on ),
      source_location( source_location ),
      capturing( std::move( capturing ) ),
      var_decl_location( var_decl_location ),
      used( false )
{
}

void Variable::mark_used()
{
  used = true;
}

bool Variable::was_used() const
{
  return used;
}

}  // namespace Pol::Bscript::Compiler
