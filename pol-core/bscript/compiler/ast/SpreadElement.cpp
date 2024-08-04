#include "SpreadElement.h"

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
SpreadElement::SpreadElement( const SourceLocation& source_location, std::unique_ptr<Node> child )
    : Expression( source_location, std::move( child ) )
{
}

void SpreadElement::accept( NodeVisitor& visitor )
{
  visitor.visit_spread_element( *this );
}

void SpreadElement::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "spread-element" );
}

}  // namespace Pol::Bscript::Compiler
