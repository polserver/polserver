#include "DebugBlock.h"

namespace Pol::Bscript::Compiler
{
/*
 * A DebugBlock names the local variables in a block.
 *
 * Suppose local_variables_base_index is 4 and local_variables == { "x, y, z" }
 * local #0: look up using parent_block_index
 * local #1: "
 * local #2: "
 * local #3: "
 * local #4: local named x
 * local #5: local named y
 * local #6: local named z
 */
DebugBlock::DebugBlock( unsigned parent_block_index, unsigned base_index,
                        std::vector<std::string> local_variable_names )
  : parent_block_index( parent_block_index ),
    base_index( base_index ),
    local_variable_names( std::move( local_variable_names ) )
{
}

}  // namespace Pol::Bscript::Compiler
