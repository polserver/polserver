#include "bscript/compiler/optimizer/ConstantFolder.h"

#include "bscript/bboolean.h"
#include "bscript/bdouble.h"
#include "bscript/blong.h"
#include "bscript/bobject.h"
#include "bscript/bstring.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/BooleanValue.h"
#include "bscript/compiler/ast/FloatValue.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/UnaryOperator.h"

namespace Pol::Bscript::Compiler
{
namespace
{
/**
 * Builds the value a constant expression stands for. Leaves `imp` empty for anything else, which
 * is how a non-constant operand declines the fold.
 */
class ConstantValue final : public NodeVisitor
{
public:
  BObjectRef imp;

  explicit ConstantValue( Expression& expression ) { expression.accept( *this ); }

  bool empty() const { return imp.get() == nullptr; }
  BObject& object() const { return *imp.get(); }

  void visit_children( Node& ) override {}
  void visit_integer_value( IntegerValue& v ) override { imp.set( new BLong( v.value ) ); }
  void visit_float_value( FloatValue& v ) override { imp.set( new Double( v.value ) ); }
  void visit_string_value( StringValue& v ) override { imp.set( new String( v.value ) ); }
  void visit_boolean_value( BooleanValue& v ) override { imp.set( new BBoolean( v.value ) ); }
};

/**
 * Applies a binary operator the way the Executor's matching ins_* member does -- note that the
 * arithmetic ones dispatch through the *right* operand, which is what resolves the double
 * dispatch to the correct overload. Keep this in step with executor.cpp.
 */
BObjectImp* apply( BObject& left, BTokenId token_id, BObject& right )
{
  switch ( token_id )
  {
  case TOK_ADD:
    return right.impref().selfPlusObjImp( left.impref() );
  case TOK_SUBTRACT:
    return right.impref().selfMinusObjImp( left.impref() );
  case TOK_MULT:
    return right.impref().selfTimesObjImp( left.impref() );
  case TOK_DIV:
    return right.impref().selfDividedByObjImp( left.impref() );
  case TOK_MODULUS:
    return right.impref().selfModulusObjImp( left.impref() );

  case TOK_BSRIGHT:
    return right.impref().selfBitShiftRightObjImp( left.impref() );
  case TOK_BSLEFT:
    return right.impref().selfBitShiftLeftObjImp( left.impref() );
  case TOK_BITAND:
    return right.impref().selfBitAndObjImp( left.impref() );
  case TOK_BITOR:
    return right.impref().selfBitOrObjImp( left.impref() );
  case TOK_BITXOR:
    return right.impref().selfBitXorObjImp( left.impref() );

  case TOK_EQUAL:
    return new BLong( left == right );
  case TOK_NEQ:
    return new BLong( left != right );
  case TOK_LESSTHAN:
    return new BLong( left < right );
  case TOK_LESSEQ:
    return new BLong( left <= right );
  case TOK_GRTHAN:
    return new BLong( left > right );
  case TOK_GREQ:
    return new BLong( left >= right );

  case TOK_AND:
    return new BLong( left.isTrue() && right.isTrue() );
  case TOK_OR:
    return new BLong( left.isTrue() || right.isTrue() );

  default:
    return nullptr;
  }
}

/**
 * Applies a unary operator the way the Executor's matching ins_* member does.
 */
BObjectImp* apply( BTokenId token_id, BObject& operand )
{
  switch ( token_id )
  {
  case TOK_UNMINUS:
    return operand.impref().inverse();
  case TOK_LOG_NOT:
    return new BLong( !operand.isTrue() );
  case TOK_BITWISE_NOT:
    return operand.impref().bitnot();

  case TOK_UNPLUSPLUS:
  {
    BObjectImp* result = operand.impref().copy();
    result->selfPlusPlus();
    return result;
  }
  case TOK_UNMINUSMINUS:
  {
    BObjectImp* result = operand.impref().copy();
    result->selfMinusMinus();
    return result;
  }

  default:
    return nullptr;
  }
}

/**
 * Turns a result back into a literal. Anything without a literal form -- an error object, an
 * array, an uninitialized value -- declines the fold and is left to the VM.
 */
std::unique_ptr<Expression> to_expression( const SourceLocation& source_location,
                                           BObjectImp& result )
{
  switch ( result.type() )
  {
  case BObjectImp::OTLong:
    return std::make_unique<IntegerValue>( source_location, static_cast<BLong&>( result ).value() );
  case BObjectImp::OTDouble:
    return std::make_unique<FloatValue>( source_location, static_cast<Double&>( result ).value() );
  case BObjectImp::OTString:
    return std::make_unique<StringValue>( source_location, static_cast<String&>( result ).value() );
  case BObjectImp::OTBoolean:
    return std::make_unique<BooleanValue>( source_location,
                                           static_cast<BBoolean&>( result ).value() );
  default:
    return {};
  }
}

bool is_number( const BObject& obj )
{
  return obj.isa( BObjectImp::OTLong ) || obj.isa( BObjectImp::OTDouble );
}

bool is_zero( const BObject& obj )
{
  if ( obj.isa( BObjectImp::OTLong ) )
    return obj.impref<BLong>().value() == 0;
  if ( obj.isa( BObjectImp::OTDouble ) )
    return obj.impref<Double>().value() == 0.0;
  return false;
}
}  // namespace

std::unique_ptr<Expression> ConstantFolder::fold( BinaryOperator& op, Report& report )
{
  ConstantValue lhs( op.lhs() );
  if ( lhs.empty() )
    return {};

  ConstantValue rhs( op.rhs() );
  if ( rhs.empty() )
    return {};

  // Dividing by a literal zero is diagnosed rather than folded: the VM answers it with an error
  // object, which has no literal form, and a script that spells it out cannot have meant it.
  if ( ( op.token_id == TOK_DIV || op.token_id == TOK_MODULUS ) && is_number( lhs.object() ) &&
       is_zero( rhs.object() ) )
  {
    report.error( op, "Program would divide by zero" );
    return {};
  }

  BObjectImp* applied = apply( lhs.object(), op.token_id, rhs.object() );
  if ( !applied )
    return {};

  BObject result( applied );
  return to_expression( op.source_location, result.impref() );
}

std::unique_ptr<Expression> ConstantFolder::fold( UnaryOperator& op )
{
  ConstantValue operand( op.operand() );
  if ( operand.empty() )
    return {};

  BObjectImp* applied = apply( op.token_id, operand.object() );
  if ( !applied )
    return {};

  BObject result( applied );
  return to_expression( op.source_location, result.impref() );
}

}  // namespace Pol::Bscript::Compiler
