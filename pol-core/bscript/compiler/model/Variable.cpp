#include "Variable.h"

#include <utility>

namespace Pol::Bscript::Compiler
{
Variable::Variable( VariableScope scope, std::string name, FunctionDepth function_depth,
                    BlockDepth block_depth, VariableIndex index, WarnOn warn_on,
                    const SourceLocation& source_location )
    : scope( scope ),
      name( std::move( name ) ),
      function_depth( function_depth ),
      block_depth( block_depth ),
      index( index ),
      warn_on( warn_on ),
      source_location( source_location ),
      used( false )
{
}

Variable::Variable( const Variable& other )
    : scope( other.scope ),
      name( other.name ),
      function_depth( other.function_depth ),
      block_depth( other.block_depth ),
      index( other.index ),
      warn_on( other.warn_on ),
      source_location( other.source_location ),
      used( other.used )
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
