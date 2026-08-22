#include "bscript/bobjectimp.h"

#include <fmt/format.h>

#include "bscript/barray.h"
#include "bscript/bboolean.h"
#include "bscript/bcontiter.h"
#include "bscript/bdict.h"
#include "bscript/bdouble.h"
#include "bscript/berror.h"
#include "bscript/blong.h"
#include "bscript/bobject.h"
#include "bscript/bstring.h"
#include "bscript/bstruct.h"
#include "bscript/buninit.h"
#include "bscript/objmembers.h"
#include "bscript/objmethods.h"
#include "clib/stlutil.h"

#if BOBJECTIMP_DEBUG
#include "bscript/escriptv.h"
#include "clib/logfacility.h"
#include <unordered_map>
#endif

namespace Pol::Bscript
{

/**
 * Pack formats:
 * - sSTRING\0   string
 * - iINTEGER\0  integer
 * - rREAL\0     real
 * - u\0         uninitialized
 * - aNN:ELEMS   array
 * - SNN:STRING
 *
 * Examples:
 * - 57              i57
 * - 4.3             r4.3
 * - "hello world"   shello world
 * - { 5,3 }         a2:i5i3
 * - { 5, "hey" }    a2:i5S3:hey
 * - { 5, "hey", 7 } a3:i5S3:heyi7
 */
BObjectImp* BObjectImp::unpack( std::istream& is )
{
  char typech;
  if ( is >> typech )
  {
    switch ( typech )
    {
    case 's':
      return String::unpack( is );
    case 'S':
      return String::unpackWithLen( is );
    case 'i':
      return BLong::unpack( is );
    case 'r':
      return Double::unpack( is );
    case 'u':
      return UninitObject::create();
    case 'a':
      return ObjArray::unpack( is );
    case 'd':
      return BDictionary::unpack( is );
    case 't':
      return BStruct::unpack( is );
    case 'e':
      return BError::unpack( is );
    case 'x':
      return UninitObject::create();
    case 'b':
      return BBoolean::unpack( is );

    default:
      return new BError( "Unknown object type '" + std::string( 1, typech ) + "'" );
    }
  }
  else
  {
    return new BError( "Unable to extract type character" );
  }
}

BObjectImp* BObjectImp::unpack( const char* pstr )
{
  ISTRINGSTREAM is( pstr );
  return unpack( is );
}

#if BOBJECTIMP_DEBUG
typedef std::unordered_map<unsigned int, BObjectImp*> bobjectimps;

bobjectimps bobjectimp_instances;
int display_bobjectimp_instance( BObjectImp* imp )
{
  INFO_PRINTLN( "{}: {}", imp->instance(), imp->getStringRep() );
  return 0;
}
void display_bobjectimp_instances()
{
  INFO_PRINTLN( "bobjectimp instances: {}", bobjectimp_instances.size() );
  for ( bobjectimps::iterator itr = bobjectimp_instances.begin(); itr != bobjectimp_instances.end();
        ++itr )
  {
    display_bobjectimp_instance( ( *itr ).second );
  }
}
#endif

#if !INLINE_BOBJECTIMP_CTOR
unsigned int BObjectImp::instances_ = 0;
Clib::SpinLock BObjectImp::bobjectimp_lock;
BObjectImp::BObjectImp( BObjectType type ) : type_( type ), instance_( 0 )
{
  Clib::SpinLockGuard lock( bobjectimp_lock );
  instance_ = instances_++;
  ++eobject_imp_count;
  ++eobject_imp_constructions;
  bobjectimp_instances[instance_] = this;
}

BObjectImp::~BObjectImp()
{
  Clib::SpinLockGuard lock( bobjectimp_lock );
  bobjectimp_instances.erase( instance_ );
  --eobject_imp_count;
}
#endif

std::string BObjectImp::pack() const
{
  std::string str;
  packonto( str );
  return str;
}

void BObjectImp::packonto( std::string& str ) const
{
  str += "u";
}

std::string BObjectImp::getFormattedStringRep() const
{
  return getStringRep();
}

const char* BObjectImp::typestr( BObjectType typ )
{
  switch ( typ )
  {
  case OTUnknown:
    return "Unknown";
  case OTUninit:
    return "Uninit";
  case OTString:
    return "String";
  case OTLong:
    return "Integer";
  case OTDouble:
    return "Double";
  case OTArray:
    return "Array";
  case OTApplicObj:
    return "ApplicObj";
  case OTError:
    return "Error";
  case OTDictionary:
    return "Dictionary";
  case OTStruct:
    return "Struct";
  case OTPacket:
    return "Packet";
  case OTBinaryFile:
    return "BinaryFile";
  case OTBoolean:
    return "Boolean";
  case OTFuncRef:
    return "FunctionReference";
  default:
    return "Undefined";
  }
}

const char* BObjectImp::typeOf() const
{
  return typestr( type_ );
}

u8 BObjectImp::typeOfInt() const
{
  return type_;
}

/**
 * Can be overridden. By default objects are considered equal
 * only when having the same address in memory
 */
bool BObjectImp::operator==( const BObjectImp& objimp ) const
{
  return ( this == &objimp );
}
/**
 * Should be overridden. By default objects are lesser or greater
 * based on their type ID. Uninit and Error are always lesser than any other.
 * Same type object should have a custom comparison.
 *
 * @warning: do not forget to call base class when overriding
 */
bool BObjectImp::operator<( const BObjectImp& objimp ) const
{
  // Error an uninit are always lesser than any other type
  if ( ( objimp.type_ == OTError || objimp.type_ == OTUninit ) && type_ != OTError &&
       type_ != OTUninit )
    return false;

  if ( type_ == objimp.type_ )
  {
    // This is "undefined behavior" and should be avoided by implementing
    // comparison in child class
    return ( this < &objimp );
  }

  return type_ < objimp.type_;
}
/**
 * Can be overridden. By default uses == and <
 */
bool BObjectImp::operator<=( const BObjectImp& objimp ) const
{
  return *this == objimp || *this < objimp;
}
/**
 * Can be overridden. By default uses == and <
 */
bool BObjectImp::operator>( const BObjectImp& objimp ) const
{
  return !( *this == objimp || *this < objimp );
}
/**
 * Can be overridden. By default uses <
 */
bool BObjectImp::operator>=( const BObjectImp& objimp ) const
{
  return !( *this < objimp );
}
/**
 * Can be overridden. By default uses ==
 */
bool BObjectImp::operator!=( const BObjectImp& objimp ) const
{
  return !( *this == objimp );
}

BObjectImp* BObjectImp::array_assign( BObjectImp* /*idx*/, BObjectImp* /*target*/, bool /*copy*/ )
{
  return this;
}

BObjectRef BObjectImp::OperMultiSubscript( std::stack<BObjectRef>& /*indices*/ )
{
  // a[x,y] means a slice -- a start and a length -- and only Array and String have one to give.
  // Everything else used to read this as chained subscripting, so the same syntax meant two
  // different things depending on what it was applied to. Reach the members of a dictionary or a
  // struct with a[x][y] instead.
  return BObjectRef( new BError( "Multiple subscript not supported for this type" ) );
}

BObjectRef BObjectImp::OperMultiSubscriptAssign( std::stack<BObjectRef>& indices,
                                                 BObjectImp* target )
{
  BObjectRef index = indices.top();
  indices.pop();
  if ( indices.empty() )
  {
    BObjectImp* imp = array_assign( ( *index ).impptr(), target, false );
    return BObjectRef( imp );
  }

  // Only String assigns through a multi-subscript; every other type reaches this and has
  // nowhere to put the value.
  return BObjectRef( new BError( "Multiple subscript assignment not supported for this type" ) );
}

namespace
{
/// The operand as the number it stands for, or an independent copy of it if it is not a Boolean.
/// Copying the non-Boolean side keeps ownership uniform on a path that only ever runs once.
BObjectImp* as_number( BObjectImp& imp )
{
  if ( imp.isa( BObjectImp::OTBoolean ) )
    return new BLong( imp.isTrue() ? 1 : 0 );
  return imp.copy();
}

/// Applies one arithmetic or bitwise operator, exactly as the matching Executor::ins_* member
/// does -- note the dispatch through the *right* operand, which is what resolves the double
/// dispatch. Returns null when no type in the pair had a rule. Comparisons are absent on purpose:
/// they are total and never reach the fallback.
BObjectImp* apply_arithmetic( BTokenId token_id, BObjectImp& left, BObjectImp& right )
{
  switch ( token_id )
  {
  case TOK_ADD:
    return right.selfPlusObjImp( left );
  case TOK_SUBTRACT:
    return right.selfMinusObjImp( left );
  case TOK_MULT:
    return right.selfTimesObjImp( left );
  case TOK_DIV:
    return right.selfDividedByObjImp( left );
  case TOK_MODULUS:
    return right.selfModulusObjImp( left );
  case TOK_BSRIGHT:
    return right.selfBitShiftRightObjImp( left );
  case TOK_BSLEFT:
    return right.selfBitShiftLeftObjImp( left );
  case TOK_BITAND:
    return right.selfBitAndObjImp( left );
  case TOK_BITOR:
    return right.selfBitOrObjImp( left );
  case TOK_BITXOR:
    return right.selfBitXorObjImp( left );
  default:
    return nullptr;
  }
}

const char* operator_name( BTokenId token_id )
{
  switch ( token_id )
  {
  case TOK_ADD:
    return "+";
  case TOK_SUBTRACT:
    return "-";
  case TOK_MULT:
    return "*";
  case TOK_DIV:
    return "/";
  case TOK_MODULUS:
    return "%";
  case TOK_BSRIGHT:
    return ">>";
  case TOK_BSLEFT:
    return "<<";
  case TOK_BITAND:
    return "&";
  case TOK_BITOR:
    return "|";
  case TOK_BITXOR:
    return "^";
  default:
    return "?";
  }
}
}  // namespace

BObjectImp* apply_operator_fallback( BTokenId token_id, BObjectImp& left, BObjectImp& right,
                                     std::string* no_rule_message )
{
  no_rule_message->clear();

  // `+` with a String on either side concatenates. A String on the *left* already did this through
  // its own generic overload and never reaches here, so this is what makes the two orders agree --
  // and it is why the Boolean is spelled "true" here rather than coerced to 1 below.
  if ( token_id == TOK_ADD &&
       ( left.isa( BObjectImp::OTString ) || right.isa( BObjectImp::OTString ) ) )
    return new String( left.getStringRep() + right.getStringRep() );

  // A Boolean is 1 or 0 in arithmetic. Retry through the numeric rules rather than restating them
  // here, so this can never drift from what the same expression on Integers does.
  if ( left.isa( BObjectImp::OTBoolean ) || right.isa( BObjectImp::OTBoolean ) )
  {
    BObject as_left( as_number( left ) );
    BObject as_right( as_number( right ) );
    if ( BObjectImp* result = apply_arithmetic( token_id, as_left.impref(), as_right.impref() ) )
      return result;
  }

  *no_rule_message = fmt::format( "Operator {} not supported for {} and {}",
                                  operator_name( token_id ), left.typeOf(), right.typeOf() );
  return new BError( *no_rule_message );
}

BObjectImp* BObjectImp::selfIsObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfIsObj( *this );
}

BObjectImp* BObjectImp::selfIsObj( const BObjectImp& ) const
{
  return new BLong( 0 );
}

BObjectImp* BObjectImp::selfPlusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfPlusObj( *this );
}
BObjectImp* BObjectImp::selfPlusObj( const BObjectImp& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfPlusObj( const BLong& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfPlusObj( const Double& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfPlusObj( const String& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfPlusObj( const ObjArray& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
void BObjectImp::selfPlusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfPlusObj( *this, obj );
}
void BObjectImp::selfPlusObj( BObjectImp& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_ADD, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfPlusObj( BLong& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_ADD, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfPlusObj( Double& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_ADD, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfPlusObj( String& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_ADD, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfPlusObj( ObjArray& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_ADD, *this, objimp, &no_rule_message ) );
}

BObjectImp* BObjectImp::selfMinusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfMinusObj( *this );
}
BObjectImp* BObjectImp::selfMinusObj( const BObjectImp& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfMinusObj( const BLong& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfMinusObj( const Double& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfMinusObj( const String& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
void BObjectImp::selfMinusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfMinusObj( *this, obj );
}
void BObjectImp::selfMinusObj( BObjectImp& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_SUBTRACT, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfMinusObj( BLong& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_SUBTRACT, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfMinusObj( Double& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_SUBTRACT, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfMinusObj( String& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_SUBTRACT, *this, objimp, &no_rule_message ) );
}

BObjectImp* BObjectImp::selfTimesObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfTimesObj( *this );
}
BObjectImp* BObjectImp::selfTimesObj( const BObjectImp& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfTimesObj( const BLong& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfTimesObj( const Double& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
void BObjectImp::selfTimesObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfTimesObj( *this, obj );
}
void BObjectImp::selfTimesObj( BObjectImp& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_MULT, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfTimesObj( BLong& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_MULT, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfTimesObj( Double& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_MULT, *this, objimp, &no_rule_message ) );
}

BObjectImp* BObjectImp::selfDividedByObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfDividedByObj( *this );
}
BObjectImp* BObjectImp::selfDividedByObj( const BObjectImp& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfDividedByObj( const BLong& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfDividedByObj( const Double& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
void BObjectImp::selfDividedByObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfDividedByObj( *this, obj );
}
void BObjectImp::selfDividedByObj( BObjectImp& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_DIV, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfDividedByObj( BLong& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_DIV, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfDividedByObj( Double& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_DIV, *this, objimp, &no_rule_message ) );
}

BObjectImp* BObjectImp::selfModulusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfModulusObj( *this );
}
BObjectImp* BObjectImp::selfModulusObj( const BObjectImp& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfModulusObj( const BLong& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfModulusObj( const Double& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
void BObjectImp::selfModulusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfModulusObj( *this, obj );
}
void BObjectImp::selfModulusObj( BObjectImp& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_MODULUS, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfModulusObj( BLong& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_MODULUS, *this, objimp, &no_rule_message ) );
}
void BObjectImp::selfModulusObj( Double& objimp, BObject& obj )
{
  std::string no_rule_message;
  obj.setimp( apply_operator_fallback( TOK_MODULUS, *this, objimp, &no_rule_message ) );
}

BObjectImp* BObjectImp::selfBitShiftRightObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitShiftRightObj( *this );
}
BObjectImp* BObjectImp::selfBitShiftRightObj( const BObjectImp& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfBitShiftRightObj( const BLong& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfBitShiftLeftObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitShiftLeftObj( *this );
}
BObjectImp* BObjectImp::selfBitShiftLeftObj( const BObjectImp& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfBitShiftLeftObj( const BLong& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfBitAndObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitAndObj( *this );
}
BObjectImp* BObjectImp::selfBitAndObj( const BObjectImp& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfBitAndObj( const BLong& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfBitOrObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitOrObj( *this );
}
BObjectImp* BObjectImp::selfBitOrObj( const BObjectImp& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfBitOrObj( const BLong& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfBitXorObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitXorObj( *this );
}
BObjectImp* BObjectImp::selfBitXorObj( const BObjectImp& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::selfBitXorObj( const BLong& /*objimp*/ ) const
{
  return nullptr;  // no rule for this pair; see specs/escript/15
}
BObjectImp* BObjectImp::bitnot() const
{
  return copy();
}

void BObjectImp::operInsertInto( BObject& obj, const BObjectImp& /*objimp*/ )
{
  obj.setimp( new BError( "Object is not a 'container'" ) );
}

void BObjectImp::operPlusEqual( BObject& obj, BObjectImp& objimp )
{
  objimp.selfPlusObjImp( *this, obj );
  // obj.setimp( objimp.selfPlusObjImp( *this ) );
}

void BObjectImp::operMinusEqual( BObject& obj, BObjectImp& objimp )
{
  objimp.selfMinusObjImp( *this, obj );
  // obj.setimp( selfMinusObjImp( objimp ) );
}

void BObjectImp::operTimesEqual( BObject& obj, BObjectImp& objimp )
{
  objimp.selfTimesObjImp( *this, obj );
  // obj.setimp( selfTimesObjImp( objimp ) );
}

void BObjectImp::operDivideEqual( BObject& obj, BObjectImp& objimp )
{
  objimp.selfDividedByObjImp( *this, obj );
  // obj.setimp( selfDividedByObjImp( objimp ) );
}

void BObjectImp::operModulusEqual( BObject& obj, BObjectImp& objimp )
{
  objimp.selfModulusObjImp( *this, obj );
  // obj.setimp( selfModulusObjImp( objimp ) );
}

BObject BObjectImp::operator-() const
{
  BObjectImp* newobj = inverse();
  return BObject( newobj );
}

BObjectImp* BObjectImp::inverse() const
{
  return UninitObject::create();
}

void BObjectImp::selfPlusPlus() {}

void BObjectImp::selfMinusMinus() {}

BObjectRef BObjectImp::OperSubscript( const BObject& /*obj*/ )
{
  return BObjectRef( copy() );
}

/*
  "All Objects are inherently good."
  */
bool BObjectImp::isTrue() const
{
  return true;
}

BObjectImp* BObjectImp::call_method( const char* methodname, Executor& /*ex*/ )
{
  return new BError( std::string( "Method '" ) + methodname + "' not found" );
}
BObjectImp* BObjectImp::call_method_id( const int id, Executor& /*ex*/, bool /*forcebuiltin*/ )
{
  return new BError( fmt::format( "Method id '{}' ({}) not found", id, getObjMethod( id )->code ) );
}
BObjectRef BObjectImp::set_member( const char* membername, BObjectImp* /*valueimp*/, bool /*copy*/ )
{
  return BObjectRef( new BError( std::string( "Member '" ) + membername + "' not found" ) );
}
BObjectRef BObjectImp::get_member( const char* /*membername*/ )
{
  return BObjectRef( new BError( "Object does not support members" ) );
}
BObjectRef BObjectImp::get_member_id( const int id )
{
  ObjMember* memb = getObjMember( id );

  return get_member( memb->code );
}
BObjectRef BObjectImp::set_member_id( const int id, BObjectImp* valueimp, bool copy )
{
  ObjMember* memb = getObjMember( id );

  return set_member( memb->code, valueimp, copy );
}
long BObjectImp::contains( const BObjectImp& /*imp*/ ) const
{
  return 0;
}

BObjectRef BObjectImp::operDotPlus( const char* /*name*/ )
{
  return BObjectRef( new BError( "Operator .+ undefined" ) );
}

BObjectRef BObjectImp::operDotMinus( const char* /*name*/ )
{
  return BObjectRef( new BError( "Operator .- undefined" ) );
}

BObjectRef BObjectImp::operDotQMark( const char* /*name*/ )
{
  return BObjectRef( new BError( "Operator .? undefined" ) );
}

ContIterator* BObjectImp::createIterator( BObject* /*pIterVal*/ )
{
  return new ContIterator();
}
}  // namespace Pol::Bscript
