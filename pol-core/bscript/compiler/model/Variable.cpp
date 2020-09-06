#include "Variable.h"

#include <utility>

namespace Pol::Bscript::Compiler
{
Variable::Variable( VariableScope scope, std::string name, unsigned block_depth, size_t index,
                    WarnOn warn_on, const SourceLocation& source_location )
  : scope( scope ),
    name( std::move( name ) ),
    block_depth( block_depth ),
    index( index ),
    warn_on( warn_on ),
    source_location( source_location ),
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
