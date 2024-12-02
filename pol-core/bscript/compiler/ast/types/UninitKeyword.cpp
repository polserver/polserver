#include "UninitKeyword.h"

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
UninitKeyword::UninitKeyword( const SourceLocation& source_location ) : TypeNode( source_location )
{
}

void UninitKeyword::describe_to( std::string& w ) const
{
  w += "uninit-keyword";
}
}  // namespace Pol::Bscript::Compiler
