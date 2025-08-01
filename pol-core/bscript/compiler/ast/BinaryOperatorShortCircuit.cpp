#include "BinaryOperatorShortCircuit.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/model/FlowControlLabel.h"

#include <utility>

namespace Pol::Bscript::Compiler
{
BinaryOperatorShortCircuit::BinaryOperatorShortCircuit( const SourceLocation& source_location,
                                                        std::unique_ptr<Expression> lhs,
                                                        ShortCircuitOp op,
                                                        std::unique_ptr<Expression> rhs )
    : Expression( source_location ),
      oper( op ),
      end_label( std::make_shared<FlowControlLabel>() ),
      linked_jmp_label(),
      generate_logical_convert( true )
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
  fmt::format_to( std::back_inserter( w ), "binary-operator-short-circuit({})",
                  oper == ShortCircuitOp::OR ? "||" : "&&" );
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
