#include "LocalVariableScope.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/analyzer/LocalVariableScopes.h"
#include "bscript/compiler/analyzer/Variables.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/LocalVariableScopeInfo.h"
#include "bscript/compiler/model/Variable.h"

namespace Pol::Bscript::Compiler
{
LocalVariableScope::LocalVariableScope( LocalVariableScopes& scopes,
                                        LocalVariableScopeInfo& local_variable_scope_info )
    : scopes( scopes ),
      report( scopes.report ),
      block_depth( scopes.local_variable_scopes.size() ),
      prev_locals( scopes.local_variables.count() ),
      local_variable_scope_info( local_variable_scope_info )
{
  local_variable_scope_info.base_index = prev_locals;
  scopes.local_variable_scopes.push_back( this );
}

LocalVariableScope::~LocalVariableScope()
{
  scopes.local_variables.remove_all_but( prev_locals );

  for ( auto& variable : shadowing )
  {
    scopes.local_variables.restore_shadowed( variable );
  }

  scopes.local_variable_scopes.pop_back();
}

std::shared_ptr<Variable> LocalVariableScope::create( const std::string& name, WarnOn warn_on,
                                                      const SourceLocation& source_location )
{
  if ( auto existing = scopes.local_variables.find( name ) )
  {
    if ( existing->block_depth == block_depth )
    {
      report.error( source_location,
                    "Variable '{}' is already in scope.\n"
                    "  See previous definition at: {}",
                    name, existing->source_location );
      return existing;
    }
    shadowing.push_back( existing );
  }
  auto local = scopes.local_variables.create( name, block_depth, warn_on, source_location );

  local_variable_scope_info.variables.push_back( local );

  return local;
}

std::shared_ptr<Variable> LocalVariableScope::capture( std::shared_ptr<Variable>& other )
{
  auto local = scopes.local_variables.capture( other );

  other->mark_used();

  local_variable_scope_info.variables.push_back( local );

  return local;
}

}  // namespace Pol::Bscript::Compiler
