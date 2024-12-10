#include "TypeParameterList.h"

#include "bscript/compiler/ast/types/TypeParameter.h"
#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
TypeParameterList::TypeParameterList( const SourceLocation& source_location,
                                      std::vector<std::unique_ptr<TypeParameter>> type_parameters )
    : Node( source_location, std::move( type_parameters ) )
{
}

TypeParameterList::TypeParameterList( const SourceLocation& source_location )
    : Node( source_location )
{
}

void TypeParameterList::accept( NodeVisitor& ) {}

void TypeParameterList::describe_to( std::string& w ) const
{
  w += "type-parameter-list";
}
}  // namespace Pol::Bscript::Compiler
