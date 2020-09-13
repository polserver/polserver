#include "TernaryOperator.h"

#include <format/format.h>

#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
TernaryOperator::TernaryOperator( const SourceLocation& source_location,
                                  std::unique_ptr<Expression> predicate,
                                  std::unique_ptr<Expression> consequent,
                                  std::unique_ptr<Expression> alternative )
    : Expression( source_location )
{
  children.reserve( 3 );
  children.push_back( std::move( predicate ) );
  children.push_back( std::move( consequent ) );
  children.push_back( std::move( alternative ) );
}

void TernaryOperator::accept( NodeVisitor& visitor )
{
  visitor.visit_ternary_operator( *this );
}

void TernaryOperator::describe_to( fmt::Writer& w ) const
{
  w << "ternary-operator";
}

Expression& TernaryOperator::predicate()
{
  return child<Expression>( 0 );
}

Expression& TernaryOperator::consequent()
{
  return child<Expression>( 1 );
}

Expression& TernaryOperator::alternative()
{
  return child<Expression>( 2 );
}

};  // namespace Pol::Bscript::Compiler
