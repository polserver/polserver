#include "ElementAccess.h"

#include <format/format.h>

#include "bscript/compiler/ast/ElementIndexes.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ElementAccess::ElementAccess( const SourceLocation& source_location,
                              std::unique_ptr<Expression> lhs,
                              std::unique_ptr<ElementIndexes> indexes )
    : Expression( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( lhs ) );
  children.push_back( std::move( indexes ) );
}

void ElementAccess::accept( NodeVisitor& visitor )
{
  visitor.visit_element_access( *this );
}

void ElementAccess::describe_to( std::string& w ) const
{
  w += "element-access";
}

ElementIndexes& ElementAccess::indexes()
{
  return child<ElementIndexes>( 1 );
}

std::unique_ptr<Expression> ElementAccess::take_entity()
{
  return take_child<Expression>( 0 );
}

std::unique_ptr<ElementIndexes> ElementAccess::take_indexes()
{
  return take_child<ElementIndexes>( 1 );
}

}  // namespace Pol::Bscript::Compiler
