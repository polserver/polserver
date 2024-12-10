#include "TypeConstraint.h"

#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
TypeConstraint::TypeConstraint( const SourceLocation& source_location,
                                std::unique_ptr<TypeNode> base_type )
    : Node( source_location, std::move( base_type ) )
{
}

void TypeConstraint::describe_to( std::string& w ) const
{
  w += "type-constraint(extends)";
}

void TypeConstraint::accept( NodeVisitor& ) {}
}  // namespace Pol::Bscript::Compiler
