#include "FlowControlScope.h"

#include "bscript/compiler/analyzer/FlowControlScopes.h"
#include "bscript/compiler/analyzer/Variables.h"
#include "bscript/compiler/model/FlowControlLabel.h"

namespace Pol::Bscript::Compiler
{
FlowControlScope::FlowControlScope( FlowControlScopes& flow_control_scopes,
                                    const SourceLocation& source_location, std::string name,
                                    std::shared_ptr<FlowControlLabel> flow_control_label )
    : source_location( source_location ),
      name( std::move( name ) ),
      flow_control_label( std::move( flow_control_label ) ),
      local_variables_size( flow_control_scopes.local_variables.count() ),
      parent_scope( flow_control_scopes.current_unnamed_scope ),
      flow_control_scopes( flow_control_scopes )
{
  flow_control_scopes.push( this );
}

FlowControlScope::~FlowControlScope()
{
  flow_control_scopes.pop( this );
}

}  // namespace Pol::Bscript::Compiler
