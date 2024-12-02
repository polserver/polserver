#include "TypeNode.h"

namespace Pol::Bscript::Compiler
{
TypeNode::TypeNode( const SourceLocation& source_location ) : Node( source_location ) {}

TypeNode::TypeNode( const SourceLocation& source_location, NodeVector children )
    : Node( source_location, std::move( children ) )
{
}

TypeNode::TypeNode( const SourceLocation& source_location, std::unique_ptr<Node> child )
    : Node( source_location, std::move( child ) )
{
}

void TypeNode::accept( NodeVisitor& ) {}
}  // namespace Pol::Bscript::Compiler
