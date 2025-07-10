#include "ElvisOperator.h"


#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ElvisOperator::ElvisOperator( const SourceLocation& source_location,
                              std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs )
    : Expression( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( lhs ) );
  children.push_back( std::move( rhs ) );
}

void ElvisOperator::accept( NodeVisitor& visitor )
{
  visitor.visit_elvis_operator( *this );
}

void ElvisOperator::describe_to( std::string& w ) const
{
  w += "elvis-operator";
}

Expression& ElvisOperator::lhs()
{
  return child<Expression>( 0 );
}

Expression& ElvisOperator::rhs()
{
  return child<Expression>( 1 );
}

std::unique_ptr<Expression> ElvisOperator::take_lhs()
{
  return take_child<Expression>( 0 );
}
std::unique_ptr<Expression> ElvisOperator::take_rhs()
{
  return take_child<Expression>( 1 );
}

}  // namespace Pol::Bscript::Compiler
