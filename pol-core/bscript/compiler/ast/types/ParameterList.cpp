#include "ParameterList.h"

#include "bscript/compiler/ast/types/Parameter.h"
#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
ParameterList::ParameterList( const SourceLocation& source_location,
                              std::vector<std::unique_ptr<Parameter>> parameters )
    : Node( source_location, std::move( parameters ) )
{
}

ParameterList::ParameterList( const SourceLocation& source_location ) : Node( source_location ) {}

void ParameterList::accept( NodeVisitor& ) {}

void ParameterList::describe_to( std::string& w ) const
{
  w += "parameter-list";
}
}  // namespace Pol::Bscript::Compiler
