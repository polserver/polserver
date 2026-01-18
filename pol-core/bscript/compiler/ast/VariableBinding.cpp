#include "VariableBinding.h"

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/model/ScopableName.h"

namespace Pol::Bscript::Compiler
{
VariableBinding::VariableBinding( const SourceLocation& loc, const std::string& scope,
                                  const std::string& name, bool rest )
    : Node( loc ), scoped_name( std::move( scope ), std::move( name ) ), rest( rest ), variable()
{
}

void VariableBinding::accept( NodeVisitor& visitor )
{
  visitor.visit_variable_binding( *this );
}

void VariableBinding::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "variable-binding({}{})", scoped_name.string(),
                  rest ? "..." : "" );
}

}  // namespace Pol::Bscript::Compiler
