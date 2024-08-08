#include "Identifier.h"


#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
Identifier::Identifier( const SourceLocation& source_location, std::string scope, std::string name )
    : Expression( source_location ), scope( std::move( scope ) ), name( std::move( name ) )
{
}

void Identifier::accept( NodeVisitor& visitor )
{
  visitor.visit_identifier( *this );
}

void Identifier::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "identifier({}{})",
                  scope.empty() ? "" : scope + "::", name );
}

}  // namespace Pol::Bscript::Compiler
