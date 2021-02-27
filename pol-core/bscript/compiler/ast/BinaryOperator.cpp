#include "BinaryOperator.h"

#include <format/format.h>
#include <utility>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
BinaryOperator::BinaryOperator( const SourceLocation& source_location,
                                std::unique_ptr<Expression> lhs, std::string op, BTokenId token_id,
                                std::unique_ptr<Expression> rhs )
    : Expression( source_location ), op( std::move( op ) ), token_id( token_id )
{
  children.reserve( 2 );
  children.push_back( std::move( lhs ) );
  children.push_back( std::move( rhs ) );
}

void BinaryOperator::accept( NodeVisitor& visitor )
{
  visitor.visit_binary_operator( *this );
}

void BinaryOperator::describe_to( fmt::Writer& w ) const
{
  w << "binary-operator(" << op << ")";
}

Expression& BinaryOperator::lhs()
{
  return child<Expression>( 0 );
}

Expression& BinaryOperator::rhs()
{
  return child<Expression>( 1 );
}

std::unique_ptr<Expression> BinaryOperator::take_lhs()
{
  return take_child<Expression>( 0 );
}

std::unique_ptr<Expression> BinaryOperator::take_rhs()
{
  return take_child<Expression>( 1 );
}

}  // namespace Pol::Bscript::Compiler
