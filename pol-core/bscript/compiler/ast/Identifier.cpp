#include "Identifier.h"


#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
Identifier::Identifier( const SourceLocation& loc, const std::string& calling_scope,
                        const ScopableName& scoped_name )
    : Expression( loc ), calling_scope( calling_scope ), scoped_name( scoped_name )

{
}
Identifier::Identifier( const SourceLocation& loc, const std::string& calling_scope,
                        const std::string& name )
    : Identifier( loc, calling_scope, ScopableName( ScopeName::None, name ) )
{
}

void Identifier::accept( NodeVisitor& visitor )
{
  visitor.visit_identifier( *this );
}

void Identifier::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "identifier({}{})", scoped_name.string(),
                  calling_scope.empty() ? "" : ( " in " + calling_scope ) );
}

std::string Identifier::maybe_scoped_name() const
{
  return scoped_name.string();
}

const std::string& Identifier::name() const
{
  return scoped_name.name;
}

}  // namespace Pol::Bscript::Compiler
