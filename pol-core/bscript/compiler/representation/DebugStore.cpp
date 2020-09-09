#include "DebugStore.h"

#include "compiler/model/LocalVariableScopeInfo.h"
#include "compiler/model/Variable.h"
#include "compiler/representation/DebugBlock.h"

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
                                const LocalVariableScopeInfo& local_variable_scope_info )
{
  unsigned base_index = local_variable_scope_info.base_index;
  std::vector<std::string> local_variable_names;
  local_variable_names.reserve( local_variable_scope_info.variables.size() );
  for ( auto& var : local_variable_scope_info.variables )
  {
    local_variable_names.push_back( var->name );
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
