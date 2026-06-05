#include "bobjectimp.h"

#include "../clib/stlutil.h"
#include "barray.h"
#include "bboolean.h"
#include "bdict.h"
#include "bdouble.h"
#include "berror.h"
#include "blong.h"
#include "bstring.h"
#include "bstruct.h"
#include "buninit.h"
#include "objmembers.h"
#include "objmethods.h"

#if BOBJECTIMP_DEBUG
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

BObjectRef BObjectImp::OperMultiSubscript( std::stack<BObjectRef>& indices )
{
  BObjectRef index = indices.top();
  indices.pop();
  BObjectRef ref = OperSubscript( *index );
  if ( indices.empty() )
    return ref;
  return ( *ref ).impptr()->OperMultiSubscript( indices );
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

  BObjectRef ref = OperSubscript( *index );
  return ( *ref ).impptr()->OperMultiSubscript( indices );
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
  return copy();
}
BObjectImp* BObjectImp::selfPlusObj( const BLong& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfPlusObj( const Double& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfPlusObj( const String& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfPlusObj( const ObjArray& /*objimp*/ ) const
{
  return copy();
}
void BObjectImp::selfPlusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfPlusObj( *this, obj );
}
void BObjectImp::selfPlusObj( BObjectImp& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfPlusObj( BLong& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfPlusObj( Double& /*objimp*/, BObject& /*obj*/ )
{
  // obj.setimp( selfPlusObj(objimp) );
}
void BObjectImp::selfPlusObj( String& /*objimp*/, BObject& /*obj*/ )
{
  // obj.setimp( selfPlusObj(objimp) );
}
void BObjectImp::selfPlusObj( ObjArray& /*objimp*/, BObject& /*obj*/ )
{
  // obj.setimp( selfPlusObj(objimp) );
}

BObjectImp* BObjectImp::selfMinusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfMinusObj( *this );
}
BObjectImp* BObjectImp::selfMinusObj( const BObjectImp& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfMinusObj( const BLong& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfMinusObj( const Double& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfMinusObj( const String& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfMinusObj( const ObjArray& /*objimp*/ ) const
{
  return copy();
}
void BObjectImp::selfMinusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfMinusObj( *this, obj );
}
void BObjectImp::selfMinusObj( BObjectImp& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfMinusObj( BLong& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfMinusObj( Double& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfMinusObj( String& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfMinusObj( ObjArray& /*objimp*/, BObject& /*obj*/ )
{
  //
}

BObjectImp* BObjectImp::selfTimesObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfTimesObj( *this );
}
BObjectImp* BObjectImp::selfTimesObj( const BObjectImp& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfTimesObj( const BLong& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfTimesObj( const Double& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfTimesObj( const String& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfTimesObj( const ObjArray& /*objimp*/ ) const
{
  return copy();
}
void BObjectImp::selfTimesObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfTimesObj( *this, obj );
}
void BObjectImp::selfTimesObj( BObjectImp& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfTimesObj( BLong& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfTimesObj( Double& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfTimesObj( String& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfTimesObj( ObjArray& /*objimp*/, BObject& /*obj*/ )
{
  //
}

BObjectImp* BObjectImp::selfDividedByObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfDividedByObj( *this );
}
BObjectImp* BObjectImp::selfDividedByObj( const BObjectImp& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfDividedByObj( const BLong& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfDividedByObj( const Double& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfDividedByObj( const String& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfDividedByObj( const ObjArray& /*objimp*/ ) const
{
  return copy();
}
void BObjectImp::selfDividedByObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfDividedByObj( *this, obj );
}
void BObjectImp::selfDividedByObj( BObjectImp& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfDividedByObj( BLong& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfDividedByObj( Double& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfDividedByObj( String& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfDividedByObj( ObjArray& /*objimp*/, BObject& /*obj*/ )
{
  //
}

BObjectImp* BObjectImp::selfModulusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfModulusObj( *this );
}
BObjectImp* BObjectImp::selfModulusObj( const BObjectImp& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfModulusObj( const BLong& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfModulusObj( const Double& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfModulusObj( const String& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfModulusObj( const ObjArray& /*objimp*/ ) const
{
  return copy();
}
void BObjectImp::selfModulusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfModulusObj( *this, obj );
}
void BObjectImp::selfModulusObj( BObjectImp& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfModulusObj( BLong& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfModulusObj( Double& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfModulusObj( String& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfModulusObj( ObjArray& /*objimp*/, BObject& /*obj*/ )
{
  //
}

BObjectImp* BObjectImp::selfBitShiftRightObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitShiftRightObj( *this );
}
BObjectImp* BObjectImp::selfBitShiftRightObj( const BObjectImp& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitShiftRightObj( const BLong& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitShiftRightObj( const Double& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitShiftRightObj( const String& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitShiftRightObj( const ObjArray& /*objimp*/ ) const
{
  return copy();
}
void BObjectImp::selfBitShiftRightObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfBitShiftRightObj( *this, obj );
}
void BObjectImp::selfBitShiftRightObj( BObjectImp& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitShiftRightObj( BLong& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitShiftRightObj( Double& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitShiftRightObj( String& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitShiftRightObj( ObjArray& /*objimp*/, BObject& /*obj*/ )
{
  //
}

BObjectImp* BObjectImp::selfBitShiftLeftObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitShiftLeftObj( *this );
}
BObjectImp* BObjectImp::selfBitShiftLeftObj( const BObjectImp& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitShiftLeftObj( const BLong& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitShiftLeftObj( const Double& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitShiftLeftObj( const String& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitShiftLeftObj( const ObjArray& /*objimp*/ ) const
{
  return copy();
}
void BObjectImp::selfBitShiftLeftObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfBitShiftLeftObj( *this, obj );
}
void BObjectImp::selfBitShiftLeftObj( BObjectImp& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitShiftLeftObj( BLong& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitShiftLeftObj( Double& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitShiftLeftObj( String& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitShiftLeftObj( ObjArray& /*objimp*/, BObject& /*obj*/ )
{
  //
}

BObjectImp* BObjectImp::selfBitAndObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitAndObj( *this );
}
BObjectImp* BObjectImp::selfBitAndObj( const BObjectImp& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitAndObj( const BLong& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitAndObj( const Double& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitAndObj( const String& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitAndObj( const ObjArray& /*objimp*/ ) const
{
  return copy();
}
void BObjectImp::selfBitAndObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfBitAndObj( *this, obj );
}
void BObjectImp::selfBitAndObj( BObjectImp& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitAndObj( BLong& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitAndObj( Double& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitAndObj( String& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitAndObj( ObjArray& /*objimp*/, BObject& /*obj*/ )
{
  //
}

BObjectImp* BObjectImp::selfBitOrObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitOrObj( *this );
}
BObjectImp* BObjectImp::selfBitOrObj( const BObjectImp& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitOrObj( const BLong& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitOrObj( const Double& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitOrObj( const String& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitOrObj( const ObjArray& /*objimp*/ ) const
{
  return copy();
}
void BObjectImp::selfBitOrObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfBitOrObj( *this, obj );
}
void BObjectImp::selfBitOrObj( BObjectImp& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitOrObj( BLong& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitOrObj( Double& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitOrObj( String& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitOrObj( ObjArray& /*objimp*/, BObject& /*obj*/ )
{
  //
}

BObjectImp* BObjectImp::selfBitXorObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitXorObj( *this );
}
BObjectImp* BObjectImp::selfBitXorObj( const BObjectImp& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitXorObj( const BLong& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitXorObj( const Double& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitXorObj( const String& /*objimp*/ ) const
{
  return copy();
}
BObjectImp* BObjectImp::selfBitXorObj( const ObjArray& /*objimp*/ ) const
{
  return copy();
}
void BObjectImp::selfBitXorObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfBitXorObj( *this, obj );
}
void BObjectImp::selfBitXorObj( BObjectImp& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitXorObj( BLong& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitXorObj( Double& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitXorObj( String& /*objimp*/, BObject& /*obj*/ )
{
  //
}
void BObjectImp::selfBitXorObj( ObjArray& /*objimp*/, BObject& /*obj*/ )
{
  //
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

}  // namespace Pol::Bscript
