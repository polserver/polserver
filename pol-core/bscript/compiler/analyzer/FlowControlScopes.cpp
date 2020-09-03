#include "FlowControlScopes.h"

#include "compiler/analyzer/FlowControlScope.h"
#include "compiler/Report.h"

namespace Pol::Bscript::Compiler
{
FlowControlScopes::FlowControlScopes( const Variables& local_variables, Report& report )
  : current_unnamed_scope( nullptr ),
    named_scopes(),
    local_variables( local_variables ),
    report( report )
{
}

const FlowControlScope* FlowControlScopes::find( const std::string& name )
{
  if ( name.empty() )
  {
    return current_unnamed_scope;
  }

  auto itr = named_scopes.find( name );
  if ( itr != named_scopes.end() )
  {
    return ( *itr ).second;
  }

  return nullptr;
}

bool FlowControlScopes::any() const
{
  return current_unnamed_scope != nullptr;
}

void FlowControlScopes::push( const FlowControlScope* scope )
{
  current_unnamed_scope = scope;

  if ( !scope->name.empty() )
  {
    auto ins = named_scopes.insert( { scope->name, scope } );
    if ( !ins.second )
    {
      report.error( scope->source_location, "A statement with label '", scope->name,
                    "' is already in scope.\n",
                    "  See also: ", ( *ins.first ).second->source_location, "\n" );
    }
  }
}

void FlowControlScopes::pop( const FlowControlScope* scope )
{
  if ( scope != current_unnamed_scope )
  {
    scope->source_location.internal_error(
        "current_unnamed_scope does not match scope being popped" );
  }

  current_unnamed_scope = scope->parent_scope;

  if ( !scope->name.empty() )
    named_scopes.erase( scope->name );
}

}  // namespace Pol::Bscript::Compiler
