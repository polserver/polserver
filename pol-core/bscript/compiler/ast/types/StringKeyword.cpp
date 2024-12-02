#include "StringKeyword.h"

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
StringKeyword::StringKeyword( const SourceLocation& source_location ) : TypeNode( source_location )
{
}

void StringKeyword::describe_to( std::string& w ) const
{
  w += "string-keyword";
}
}  // namespace Pol::Bscript::Compiler
