#include "BinaryOperatorWithIntegerOptimizer.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/BooleanValue.h"
#include "bscript/compiler/ast/FloatValue.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/StringValue.h"

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
  case TOK_MODULUS:
    if ( rhs.value == 0 )
    {
      report.error( op, "Program would divide by zero" );
      return;
    }
    value = lhs.value % rhs.value;
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

void BinaryOperatorWithIntegerOptimizer::visit_float_value( FloatValue& rhs )
{
  auto setInt = [&]( int val )
  { optimized_result = std::make_unique<IntegerValue>( op.source_location, val ); };
  auto setDouble = [&]( double val )
  { optimized_result = std::make_unique<FloatValue>( op.source_location, val ); };
  auto eql = []( int a, double b ) { return fabs( a - b ) < 0.00000001; };
  switch ( op.token_id )
  {
  case TOK_ADD:
    setDouble( lhs.value + rhs.value );
    break;
  case TOK_SUBTRACT:
    setDouble( lhs.value - rhs.value );
    break;
  case TOK_MULT:
    setDouble( lhs.value * rhs.value );
    break;
  case TOK_DIV:
    if ( rhs.value == 0 )
    {
      report.error( op, "Program would divide by zero" );
      return;
    }
    setDouble( lhs.value / rhs.value );
    break;
  case TOK_MODULUS:
    if ( rhs.value == 0 )
    {
      report.error( op, "Program would divide by zero" );
      return;
    }
    setDouble( fmod( lhs.value, rhs.value ) );
    break;
  case TOK_EQUAL:
    setInt( eql( lhs.value, rhs.value ) );
    break;
  case TOK_NEQ:
    setInt( !eql( lhs.value, rhs.value ) );
    break;
  case TOK_LESSTHAN:
    setInt( lhs.value < rhs.value );
    break;
  case TOK_LESSEQ:
    setInt( eql( lhs.value, rhs.value ) || ( lhs.value < rhs.value ) );
    break;
  case TOK_GRTHAN:
    setInt( lhs.value > rhs.value );
    break;
  case TOK_GREQ:
    setInt( eql( lhs.value, rhs.value ) || ( lhs.value > rhs.value ) );
    break;
  case TOK_AND:
    setInt( lhs.value != 0 && rhs.value != 0.0 );
    break;
  case TOK_OR:
    setInt( lhs.value != 0 || rhs.value != 0.0 );
    break;
  default:
    break;
  }
}

void BinaryOperatorWithIntegerOptimizer::visit_string_value( StringValue& rhs )
{
  auto setString = [&]( std::string&& val )
  { optimized_result = std::make_unique<StringValue>( op.source_location, val ); };
  switch ( op.token_id )
  {
  case TOK_ADD:
    setString( fmt::format( "{}{}", lhs.value, rhs.value ) );
    break;
  default:
    break;
  }
}

void BinaryOperatorWithIntegerOptimizer::visit_boolean_value( BooleanValue& rhs )
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
}  // namespace Pol::Bscript::Compiler
