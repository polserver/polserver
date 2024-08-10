#include "Identifier.h"


#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
Identifier::Identifier( const SourceLocation& source_location, std::string calling_scope,
                        std::string name )
    : Expression( source_location ),
      calling_scope( std::move( calling_scope ) ),
      name( std::move( name ) )
{
}

void Identifier::accept( NodeVisitor& visitor )
{
  visitor.visit_identifier( *this );
}

void Identifier::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "identifier({}{})", name,
                  calling_scope.empty() ? "" : ( " in " + calling_scope ) );
}

std::string Identifier::maybe_scoped_name() const
{
  if ( !calling_scope.empty() )
  {
    return fmt::format( "{}::{}", calling_scope, name );
  }

  return name;
}

}  // namespace Pol::Bscript::Compiler
