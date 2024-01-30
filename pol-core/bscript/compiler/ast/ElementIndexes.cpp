#include "ElementIndexes.h"


#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ElementIndexes::ElementIndexes( const SourceLocation& source_location,
                                std::vector<std::unique_ptr<Expression>> indexes )
    : Node( source_location, std::move( indexes ) )
{
}

void ElementIndexes::accept( NodeVisitor& visitor )
{
  visitor.visit_element_indexes( *this );
}

void ElementIndexes::describe_to( std::string& w ) const
{
  w += type();
}

std::string ElementIndexes::type() const
{
  return "element-indexes";
}

}  // namespace Pol::Bscript::Compiler
