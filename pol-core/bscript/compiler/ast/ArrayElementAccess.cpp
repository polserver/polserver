#include "ArrayElementAccess.h"

#include <format/format.h>

#include "ArrayElementIndexes.h"
#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ArrayElementAccess::ArrayElementAccess( const SourceLocation& source_location,
                                        std::unique_ptr<Expression> lhs,
                                        std::unique_ptr<ArrayElementIndexes> indexes )
    : Expression( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( lhs ) );
  children.push_back( std::move( indexes ) );
}

void ArrayElementAccess::accept( NodeVisitor& visitor )
{
  visitor.visit_array_element_access( *this );
}

void ArrayElementAccess::describe_to( fmt::Writer& w ) const
{
  w << "array-element-access";
}

ArrayElementIndexes& ArrayElementAccess::indexes()
{
  return child<ArrayElementIndexes>( 1 );
}

std::unique_ptr<Expression> ArrayElementAccess::take_entity()
{
  return take_child<Expression>( 0 );
}

std::unique_ptr<ArrayElementIndexes> ArrayElementAccess::take_indexes()
{
  return take_child<ArrayElementIndexes>( 1 );
}

}  // namespace Pol::Bscript::Compiler
