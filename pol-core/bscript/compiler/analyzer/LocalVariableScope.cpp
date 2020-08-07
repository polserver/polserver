#include "LocalVariableScope.h"

#include "Scopes.h"
#include "Variables.h"
#include "compiler/Report.h"
#include "compiler/file/SourceLocation.h"
#include "compiler/model/Variable.h"

namespace Pol::Bscript::Compiler
{
LocalVariableScope::LocalVariableScope( Scopes& scopes,
std::vector<std::shared_ptr<Variable>>& debug_variables )
    : scopes( scopes ),
      report( scopes.report ),
      block_depth( scopes.local_variable_scopes.size() ),
      prev_locals( scopes.local_variables.count() ),
      debug_variables( &debug_variables )
{
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

std::shared_ptr<Variable> LocalVariableScope::create( const std::string& name, bool warn_if_used,
                                                      bool warn_if_unused,
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
  auto local = scopes.local_variables.create( name, block_depth, warn_if_used, warn_if_unused,
                                              source_location );

  if ( debug_variables )
  {
    debug_variables->push_back( local );
  }

  return local;
}

unsigned LocalVariableScope::get_block_locals() const
{
  return scopes.local_variables.count() - prev_locals;
}

}  // namespace Pol::Bscript::Compiler
