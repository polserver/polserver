#include "BinaryOperatorWithFloatOptimizer.h"

#include "bscript/bobject.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/BooleanValue.h"
#include "bscript/compiler/ast/FloatValue.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/StringValue.h"

namespace Pol::Bscript::Compiler
{
BinaryOperatorWithFloatOptimizer::BinaryOperatorWithFloatOptimizer( FloatValue& lhs,
                                                                    BinaryOperator& op,
                                                                    Report& report )
    : lhs( lhs ), op( op ), report( report )
{
}

void BinaryOperatorWithFloatOptimizer::visit_children( Node& ) {}

void BinaryOperatorWithFloatOptimizer::visit_float_value( FloatValue& rhs )
{
  auto setInt = [&]( int val )
  { optimized_result = std::make_unique<IntegerValue>( op.source_location, val ); };
  auto setDouble = [&]( double val )
  { optimized_result = std::make_unique<FloatValue>( op.source_location, val ); };
  auto eql = []( double a, double b ) { return fabs( a - b ) < 0.00000001; };

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
    if ( rhs.value == 0.0 )
    {
      report.error( op, "Expression would divide by zero." );
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
    setInt( lhs.value != 0.0 && rhs.value != 0.0 );
    break;
  case TOK_OR:
    setInt( lhs.value != 0.0 || rhs.value != 0.0 );
    break;
  default:
    return;
  }
}

void BinaryOperatorWithFloatOptimizer::visit_integer_value( IntegerValue& rhs )
{
  auto setInt = [&]( int val )
  { optimized_result = std::make_unique<IntegerValue>( op.source_location, val ); };
  auto setDouble = [&]( double val )
  { optimized_result = std::make_unique<FloatValue>( op.source_location, val ); };
  auto eql = []( double a, int b ) { return fabs( a - b ) < 0.00000001; };
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
    setInt( lhs.value != 0.0 && rhs.value != 0 );
    break;
  case TOK_OR:
    setInt( lhs.value != 0.0 || rhs.value != 0 );
    break;
  default:
    break;
  }
}

void BinaryOperatorWithFloatOptimizer::visit_string_value( StringValue& rhs )
{
  auto setString = [&]( std::string&& val )
  { optimized_result = std::make_unique<StringValue>( op.source_location, val ); };
  switch ( op.token_id )
  {
  case TOK_ADD:
    setString( fmt::format( "{}{}", Bscript::Double::double_to_string( lhs.value ), rhs.value ) );
    break;
  default:
    break;
  }
}

void BinaryOperatorWithFloatOptimizer::visit_boolean_value( BooleanValue& rhs )
{
  bool bval = false;
  switch ( op.token_id )
  {
  case TOK_EQUAL:
    bval = ( lhs.value == 0.0 ) == rhs.value;
    break;
  case TOK_NEQ:
    bval = ( lhs.value == 0.0 ) != rhs.value;
    break;
  case TOK_OR:
    bval = ( lhs.value == 0.0 ) || rhs.value;
    break;
  case TOK_AND:
    bval = ( lhs.value == 0.0 ) && rhs.value;
    break;
  default:
    return;
  }

  // Boolean logic operators return 1/0 as BLong, ie. `true || false` == `1`
  optimized_result = std::make_unique<IntegerValue>( op.source_location, bval );
}
}  // namespace Pol::Bscript::Compiler
