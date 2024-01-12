#include "ArrayInitializer.h"

#include <format/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ArrayInitializer::ArrayInitializer( const SourceLocation& source_location,
                                    std::vector<std::unique_ptr<Expression>> children )
    : Expression( source_location, std::move( children ) )
{
}

void ArrayInitializer::accept( NodeVisitor& visitor )
{
  visitor.visit_array_initializer( *this );
}

void ArrayInitializer::describe_to( std::string& w ) const
{
  w += "array-initializer";
}

}  // namespace Pol::Bscript::Compiler