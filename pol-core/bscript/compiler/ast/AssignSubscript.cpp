#include "AssignSubscript.h"

#include <format/format.h>

#include "ArrayElementIndexes.h"
#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
AssignSubscript::AssignSubscript( const SourceLocation& source_location, bool consume,
                                  std::unique_ptr<Expression> entity,
                                  std::unique_ptr<ArrayElementIndexes> indexes,
                                  std::unique_ptr<Expression> rhs )
    : Expression( source_location ), consume( consume )
{
  children.reserve( 3 );
  children.push_back( std::move( entity ) );
  children.push_back( std::move( indexes ) );
  children.push_back( std::move( rhs ) );
}

void AssignSubscript::accept( NodeVisitor& visitor )
{
  visitor.visit_assign_subscript( *this );
}
void AssignSubscript::describe_to( fmt::Writer& w ) const
{
  w << "assign-subscript";
}

ArrayElementIndexes& AssignSubscript::indexes()
{
  return child<ArrayElementIndexes>( 1 );
}

std::unique_ptr<Expression> AssignSubscript::take_entity()
{
  return take_child<Expression>( 0 );
}

std::unique_ptr<ArrayElementIndexes> AssignSubscript::take_indexes()
{
  return take_child<ArrayElementIndexes>( 1 );
}

std::unique_ptr<Expression> AssignSubscript::take_rhs()
{
  return take_child<Expression>( 2 );
}

}  // namespace Pol::Bscript::Compiler