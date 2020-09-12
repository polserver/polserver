#include "DebugStore.h"

#include "DebugBlock.h"

namespace Pol::Bscript::Compiler
{
DebugStore::DebugStore( std::vector<std::string> filenames ) : filenames( std::move( filenames ) )
{
  std::vector<std::string> empty;
  blocks.emplace_back( 0, 0, empty );
}

DebugStore::DebugStore( DebugStore&& ) noexcept = default;
DebugStore::~DebugStore() = default;

unsigned DebugStore::add_block( unsigned parent_block_id,
                                std::vector<std::string> local_variable_names )
{
  unsigned base_index = 0;
  if ( parent_block_id )
  {
    auto& parent_block = blocks.at( parent_block_id );
    base_index = parent_block.base_index + parent_block.local_variable_names.size();
  }
  blocks.emplace_back( parent_block_id, base_index, std::move( local_variable_names ) );
  return blocks.size() - 1;
}

void DebugStore::add_instruction( const DebugStore::InstructionInfo& info )
{
  instructions.push_back( info );
}

void DebugStore::add_user_function( UserFunctionInfo uf )
{
  user_functions.push_back( std::move( uf ) );
}

}  // namespace Pol::Bscript::Compiler
