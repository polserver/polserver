#include "BinaryOperatorShortCircuit.h"


#include <utility>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
BinaryOperatorShortCircuit::BinaryOperatorShortCircuit( const SourceLocation& source_location,
                                                        std::unique_ptr<Expression> lhs,
                                                        std::string op, BTokenId token_id,
                                                        std::unique_ptr<Expression> rhs )
    : Expression( source_location ), op( std::move( op ) ), token_id( token_id )
{
  children.reserve( 2 );
  children.push_back( std::move( lhs ) );
  children.push_back( std::move( rhs ) );
}

void BinaryOperatorShortCircuit::accept( NodeVisitor& visitor )
{
  visitor.visit_binary_operator_short_circuit( *this );
}

void BinaryOperatorShortCircuit::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "binary-operator-short-circuit({})", op );
}

Expression& BinaryOperatorShortCircuit::lhs()
{
  return child<Expression>( 0 );
}

Expression& BinaryOperatorShortCircuit::rhs()
{
  return child<Expression>( 1 );
}

}  // namespace Pol::Bscript::Compiler
