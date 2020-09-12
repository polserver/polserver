#include "LocalVariableScope.h"

#include "compiler/Report.h"
#include "compiler/analyzer/LocalVariableScopes.h"
#include "compiler/analyzer/Variables.h"
#include "compiler/file/SourceLocation.h"
#include "compiler/model/LocalVariableScopeInfo.h"
#include "compiler/model/Variable.h"

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
      report.error( source_location, "Variable '", name, "' is already in scope.\n",
                    "  See previous definition at: ",
                    existing->source_location, "\n" );
      return existing;
    }
    shadowing.push_back( existing );
  }
  auto local = scopes.local_variables.create( name, block_depth, warn_on,
                                              source_location );

  local_variable_scope_info.variables.push_back( local );

  return local;
}

}  // namespace Pol::Bscript::Compiler
