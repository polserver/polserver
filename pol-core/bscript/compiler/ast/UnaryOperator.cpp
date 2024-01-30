#include "UnaryOperator.h"


#include <utility>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
UnaryOperator::UnaryOperator( const SourceLocation& source_location, std::string op,
                              BTokenId token_id, std::unique_ptr<Expression> operand )
    : Expression( source_location, std::move( operand ) ),
      op( std::move( op ) ),
      token_id( token_id )
{
}

void UnaryOperator::accept( NodeVisitor& visitor )
{
  visitor.visit_unary_operator( *this );
}

void UnaryOperator::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "{}({})", type(), op );
}

std::string UnaryOperator::type() const
{
  return "unary-operator";
}

Expression& UnaryOperator::operand()
{
  return child<Expression>( 0 );
}

std::unique_ptr<Expression> UnaryOperator::take_operand()
{
  return take_child<Expression>( 0 );
}

}  // namespace Pol::Bscript::Compiler
