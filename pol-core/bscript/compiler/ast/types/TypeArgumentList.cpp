#include "TypeArgumentList.h"

#include "bscript/compiler/ast/types/TypeNode.h"
#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
TypeArgumentList::TypeArgumentList( const SourceLocation& source_location,
                                    std::vector<std::unique_ptr<TypeNode>> type_arguments )
    : Node( source_location, std::move( type_arguments ) )
{
}

TypeArgumentList::TypeArgumentList( const SourceLocation& source_location )
    : Node( source_location )
{
}

void TypeArgumentList::accept( NodeVisitor& ) {}

void TypeArgumentList::describe_to( std::string& w ) const
{
  w += "type-argument-list";
}
}  // namespace Pol::Bscript::Compiler
