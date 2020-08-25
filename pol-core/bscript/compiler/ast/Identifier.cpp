#include "Identifier.h"

#include <format/format.h>

#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
Identifier::Identifier( const SourceLocation& source_location, std::string name )
    : Expression( source_location ), name( std::move( name ) )
{
}

void Identifier::accept( NodeVisitor& visitor )
{
  visitor.visit_identifier( *this );
}

void Identifier::describe_to( fmt::Writer& w ) const
{
  w << "identifier(" << name;
  w << ")";
}

}  // namespace Pol::Bscript::Compiler
