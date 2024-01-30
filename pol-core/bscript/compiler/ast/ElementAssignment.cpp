#include "ElementAssignment.h"


#include "bscript/compiler/ast/ElementIndexes.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ElementAssignment::ElementAssignment( const SourceLocation& source_location, bool consume,
                                      std::unique_ptr<Expression> entity,
                                      std::unique_ptr<ElementIndexes> indexes,
                                      std::unique_ptr<Expression> rhs )
    : Expression( source_location ), consume( consume )
{
  children.reserve( 3 );
  children.push_back( std::move( entity ) );
  children.push_back( std::move( indexes ) );
  children.push_back( std::move( rhs ) );
}

void ElementAssignment::accept( NodeVisitor& visitor )
{
  visitor.visit_element_assignment( *this );
}
void ElementAssignment::describe_to( std::string& w ) const
{
  w += type();
}

std::string ElementAssignment::type() const
{
  return "element-assignment";
}

ElementIndexes& ElementAssignment::indexes()
{
  return child<ElementIndexes>( 1 );
}

std::unique_ptr<Expression> ElementAssignment::take_entity()
{
  return take_child<Expression>( 0 );
}

std::unique_ptr<ElementIndexes> ElementAssignment::take_indexes()
{
  return take_child<ElementIndexes>( 1 );
}

std::unique_ptr<Expression> ElementAssignment::take_rhs()
{
  return take_child<Expression>( 2 );
}

}  // namespace Pol::Bscript::Compiler