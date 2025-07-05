#include "BinaryOperatorWithBooleanOptimizer.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/BooleanValue.h"
#include "bscript/compiler/ast/FloatValue.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/StringValue.h"

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
  case TOK_EQUAL:
    bval = lhs.value == rhs.value;
    break;
  case TOK_NEQ:
    bval = lhs.value != rhs.value;
    break;
  case TOK_OR:
    bval = lhs.value || rhs.value;
    break;
  case TOK_AND:
    bval = lhs.value && rhs.value;
    break;
  default:
    return;
  }

  // Boolean logic operators return 1/0 as BLong, ie. `true || false` == `1`
  optimized_result = std::make_unique<IntegerValue>( op.source_location, bval );
}

void BinaryOperatorWithBooleanOptimizer::visit_integer_value( IntegerValue& rhs )
{
  bool bval = false;
  switch ( op.token_id )
  {
  case TOK_EQUAL:
    bval = lhs.value == rhs.value;
    break;
  case TOK_NEQ:
    bval = lhs.value != rhs.value;
    break;
  case TOK_OR:
    bval = lhs.value || rhs.value;
    break;
  case TOK_AND:
    bval = lhs.value && rhs.value;
    break;
  default:
    return;
  }

  // Boolean logic operators return 1/0 as BLong, ie. `true || false` == `1`
  optimized_result = std::make_unique<IntegerValue>( op.source_location, bval );
}

void BinaryOperatorWithBooleanOptimizer::visit_float_value( FloatValue& rhs )
{
  bool bval = false;
  switch ( op.token_id )
  {
  case TOK_EQUAL:
    bval = lhs.value == ( rhs.value != 0.0 );
    break;
  case TOK_NEQ:
    bval = lhs.value != ( rhs.value != 0.0 );
    break;
  case TOK_OR:
    bval = lhs.value || ( rhs.value != 0.0 );
    break;
  case TOK_AND:
    bval = lhs.value && ( rhs.value != 0.0 );
    break;
  default:
    return;
  }

  // Boolean logic operators return 1/0 as BLong, ie. `true || false` == `1`
  optimized_result = std::make_unique<IntegerValue>( op.source_location, bval );
}

void BinaryOperatorWithBooleanOptimizer::visit_string_value( StringValue& rhs )
{
  auto setString = [&]( std::string&& val )
  { optimized_result = std::make_unique<StringValue>( op.source_location, val ); };
  auto setInt = [&]( int val )
  { optimized_result = std::make_unique<IntegerValue>( op.source_location, val ); };
  switch ( op.token_id )
  {
  case TOK_ADD:
    setString( fmt::format( "{}{}", lhs.value, rhs.value ) );
    break;
  case TOK_EQUAL:
    setInt( lhs.value == !rhs.value.empty() );
    break;
  case TOK_NEQ:
    setInt( lhs.value != !rhs.value.empty() );
    break;
  case TOK_OR:
    setInt( lhs.value || !rhs.value.empty() );
    break;
  case TOK_AND:
    setInt( lhs.value && !rhs.value.empty() );
    break;
  default:
    return;
  }
}

}  // namespace Pol::Bscript::Compiler
