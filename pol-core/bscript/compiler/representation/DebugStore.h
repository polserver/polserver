#ifndef POLSERVER_DEBUGSTORE_H
#define POLSERVER_DEBUGSTORE_H

#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class DebugBlock;

class DebugStore
{
public:
  explicit DebugStore( std::vector<std::string> filenames );
  DebugStore( DebugStore&& ) noexcept;
  ~DebugStore();

  unsigned add_block( unsigned parent_block_index, std::vector<std::string> local_variable_names );

  struct InstructionInfo {
    unsigned file_index;
    unsigned line_number;
    unsigned block_index;
    bool statement_begin;
  };

  void add_instruction( const InstructionInfo& );

  struct UserFunctionInfo
  {
    std::string name;
    unsigned first_instruction;
    unsigned last_instruction;
  };

  void add_user_function( UserFunctionInfo );

private:
  friend class DebugStoreSerializer;
  std::vector<DebugBlock> blocks;
  std::vector<std::string> filenames;
  std::vector<InstructionInfo> instructions;
  std::vector<UserFunctionInfo> user_functions;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_DEBUGSTORE_H
