#include "ArrayElementIndexes.h"

#include <format/format.h>

#include "Expression.h"
#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ArrayElementIndexes::ArrayElementIndexes( const SourceLocation& source_location,
                                          std::vector<std::unique_ptr<Expression>> indexes )
    : Node( source_location, std::move( indexes ) )
{
}

void ArrayElementIndexes::accept( NodeVisitor& visitor )
{
  visitor.visit_array_element_indexes( *this );
}

void ArrayElementIndexes::describe_to( fmt::Writer& w ) const
{
  w << "array-element-indexes";
}

}  // namespace Pol::Bscript::Compiler
