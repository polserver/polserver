#ifndef POLSERVER_DEBUGBLOCK_H
#define POLSERVER_DEBUGBLOCK_H

#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class DebugBlock
{
public:
  DebugBlock( unsigned parent_block_index, unsigned base_index,
              std::vector<std::string> local_variables );

  const unsigned parent_block_index;

  // this->local_variable_names[n] is overall local[n+base_index]
  const unsigned base_index;

  const std::vector<std::string> local_variable_names;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_DEBUGBLOCK_H
