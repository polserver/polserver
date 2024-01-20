#include "BinaryOperatorWithBooleanOptimizer.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/BooleanValue.h"
#include "bscript/compiler/ast/IntegerValue.h"

namespace Pol::Bscript::Compiler
{
BinaryOperatorWithBooleanOptimizer::BinaryOperatorWithBooleanOptimizer( BooleanValue& lhs,
                                                                        BinaryOperator& op,
                                                                        Report& report )
    : lhs( lhs ), op( op ), report( report )
{
}

void BinaryOperatorWithBooleanOptimizer::visit_children( Node& ) {}

void BinaryOperatorWithBooleanOptimizer::visit_boolean_value( BooleanValue& rhs )
{
  bool bval = false;
  switch ( op.token_id )
  {
  case TOK_OR:
    bval = lhs.value | rhs.value;
    break;
  case TOK_AND:
    bval = lhs.value & rhs.value;
    break;
  default:
    return;
  }

  // Boolean logic operators return 1/0 as BLong, ie. `true || false` == `1`
  optimized_result = std::make_unique<IntegerValue>( op.source_location, bval );
}

}  // namespace Pol::Bscript::Compiler
