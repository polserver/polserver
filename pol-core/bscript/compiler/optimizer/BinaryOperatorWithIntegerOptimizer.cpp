#include "BinaryOperatorWithIntegerOptimizer.h"

#include "compiler/ast/BinaryOperator.h"
#include "compiler/ast/IntegerValue.h"
#include "compiler/Report.h"

namespace Pol::Bscript::Compiler
{
BinaryOperatorWithIntegerOptimizer::BinaryOperatorWithIntegerOptimizer( IntegerValue& lhs,
                                                                        BinaryOperator& op,
                                                                        Report& report )
    : lhs( lhs ), op( op ), report( report )
{
}

void BinaryOperatorWithIntegerOptimizer::visit_children( Node& ) {}

void BinaryOperatorWithIntegerOptimizer::visit_integer_value( IntegerValue& rhs )
{
  int value = 0;
  switch ( op.token_id )
  {
  case TOK_ADD:
    value = lhs.value + rhs.value;
    break;
  case TOK_SUBTRACT:
    value = lhs.value - rhs.value;
    break;
  case TOK_MULT:
    value = lhs.value * rhs.value;
    break;
  case TOK_DIV:
    if ( rhs.value == 0 )
    {
      report.error( op, "Program would divide by zero" );
      return;
    }
    value = lhs.value / rhs.value;
    break;

  case TOK_EQUAL:
    value = ( lhs.value == rhs.value );
    break;
  case TOK_NEQ:
    value = ( lhs.value != rhs.value );
    break;
  case TOK_LESSTHAN:
    value = ( lhs.value < rhs.value );
    break;
  case TOK_LESSEQ:
    value = ( lhs.value <= rhs.value );
    break;
  case TOK_GRTHAN:
    value = ( lhs.value > rhs.value );
    break;
  case TOK_GREQ:
    value = ( lhs.value >= rhs.value );
    break;

  case TOK_AND:
    value = ( lhs.value && rhs.value );
    break;
  case TOK_OR:
    value = ( lhs.value || rhs.value );
    break;

  case TOK_BSRIGHT:
    value = ( lhs.value >> rhs.value );
    break;
  case TOK_BSLEFT:
    value = ( lhs.value << rhs.value );
    break;
  case TOK_BITAND:
    value = ( lhs.value & rhs.value );
    break;
  case TOK_BITOR:
    value = ( lhs.value | rhs.value );
    break;
  case TOK_BITXOR:
    value = ( lhs.value ^ rhs.value );
    break;

  default:
    return;
  }

  optimized_result = std::make_unique<IntegerValue>( op.source_location, value );
}

}  // namespace Pol::Bscript::Compiler
