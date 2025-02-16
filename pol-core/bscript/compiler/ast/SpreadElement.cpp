#include "SpreadElement.h"

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
SpreadElement::SpreadElement( const SourceLocation& source_location, std::unique_ptr<Node> child,
                              bool spread_into )
    : Expression( source_location, std::move( child ) ), spread_into( spread_into )
{
}

void SpreadElement::accept( NodeVisitor& visitor )
{
  visitor.visit_spread_element( *this );
}

void SpreadElement::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "spread-{}", spread_into ? "into" : "element" );
}

}  // namespace Pol::Bscript::Compiler
