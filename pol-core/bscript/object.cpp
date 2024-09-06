/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2008/02/11 Turley:    ObjArray::unpack() will accept zero length Arrays and Erros from
 * Array-Elements
 * - 2009/09/05 Turley:    Added struct .? and .- as shortcut for .exists() and .erase()
 * - 2009/12/21 Turley:    ._method() call fix
 */

#include <assert.h>
#include <istream>
#include <limits>
#include <stddef.h>
#include <string>

#include "../clib/clib.h"
#include "../clib/fixalloc.h"
#include "../clib/logfacility.h"
#include "../clib/random.h"
#include "../clib/rawtypes.h"
#include "../clib/refptr.h"
#include "../clib/stlutil.h"
#include "bclassinstance.h"
#include "berror.h"
#include "bobject.h"
#include "bstruct.h"
#include "continueimp.h"
#include "dict.h"
#include "executor.h"
#include "executor.inl.h"
#include "impstr.h"
#include "object.h"
#include "objmembers.h"
#include "objmethods.h"

#if BOBJECTIMP_DEBUG
#include "escriptv.h"
#include <unordered_map>
#endif

namespace Pol
{
namespace Bscript
{
Clib::fixed_allocator<sizeof( BObject ), 256> bobject_alloc;
Clib::fixed_allocator<sizeof( UninitObject ), 256> uninit_alloc;
Clib::fixed_allocator<sizeof( BLong ), 256> blong_alloc;
Clib::fixed_allocator<sizeof( Double ), 256> double_alloc;

size_t BObjectRef::sizeEstimate() const
{
  if ( get() )
    return sizeof( BObjectRef ) + get()->sizeEstimate();
  else
    return sizeof( BObjectRef );
}
size_t BObject::sizeEstimate() const
{
  if ( objimp.get() )
    return sizeof( BObject ) + objimp.get()->sizeEstimate();
  else
    return sizeof( BObject );
}


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

BObject* BObject::clone() const
{
  return new BObject( objimp->copy() );
}

bool BObject::operator!=( const BObject& obj ) const
{
  return *objimp != *( obj.objimp );
}
bool BObject::operator==( const BObject& obj ) const
{
  return *objimp == *( obj.objimp );
}
bool BObject::operator<( const BObject& obj ) const
{
  return *objimp < *( obj.objimp );
}
bool BObject::operator<=( const BObject& obj ) const
{
  return *objimp <= *( obj.objimp );
}
bool BObject::operator>( const BObject& obj ) const
{
  return *objimp > *( obj.objimp );
}
bool BObject::operator>=( const BObject& obj ) const
{
  return *objimp >= *( obj.objimp );
}

////////////////////// BObjectImp //////////////////////
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
  OSTRINGSTREAM os;
  packonto( os );
  return OSTRINGSTREAM_STR( os );
}

void BObjectImp::packonto( std::ostream& os ) const
{
  os << "u";
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
  case OTApplicPtr:
    return "ApplicPtr";
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
  else
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
  else
  {
    BObjectRef ref = OperSubscript( *index );
    return ( *ref ).impptr()->OperMultiSubscript( indices );
  }
}

BObjectImp* BObjectImp::selfIsObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfIsObj( *this );
}

BObjectImp* BObjectImp::selfIsObj( const BObjectImp& ) const
{
  return new BBoolean( false );
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

BObject BObjectImp::operator-( void ) const
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
bool BObjectImp::isTrue( void ) const
{
  return true;
}

BObjectImp* BObjectImp::call_method( const char* methodname, Executor& /*ex*/ )
{
  return new BError( std::string( "Method '" ) + methodname + "' not found" );
}
BObjectImp* BObjectImp::call_method_id( const int id, Executor& /*ex*/, bool /*forcebuiltin*/ )
{
  OSTRINGSTREAM os;
  os << "Method id '" << id << "' (" << getObjMethod( id )->code << ") not found";
  return new BError( std::string( OSTRINGSTREAM_STR( os ) ) );
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

UninitObject* UninitObject::SharedInstance;
ref_ptr<BObjectImp> UninitObject::SharedInstanceOwner;

UninitObject::UninitObject() : BObjectImp( OTUninit ) {}

BObjectImp* UninitObject::copy( void ) const
{
  return create();
}

size_t UninitObject::sizeEstimate() const
{
  return sizeof( UninitObject );
}

bool UninitObject::isTrue() const
{
  return false;
}

/**
 * An uninit is equal to any other error or uninit
 */
bool UninitObject::operator==( const BObjectImp& imp ) const
{
  return ( imp.isa( OTError ) || imp.isa( OTUninit ) );
}

bool UninitObject::operator<( const BObjectImp& imp ) const
{
  if ( imp.isa( OTError ) || imp.isa( OTUninit ) )
    return false;  // Because it's equal can't be lesser

  return true;
}


ObjArray::ObjArray() : BObjectImp( OTArray ), name_arr(), ref_arr() {}

ObjArray::ObjArray( BObjectType type ) : BObjectImp( type ), name_arr(), ref_arr() {}

ObjArray::ObjArray( const ObjArray& copyfrom )
    : BObjectImp( copyfrom.type() ), name_arr( copyfrom.name_arr ), ref_arr( copyfrom.ref_arr )
{
  deepcopy();
}

void ObjArray::deepcopy()
{
  for ( auto& elem : ref_arr )
  {
    if ( elem.get() )
    {
      /*
          NOTE: all BObjectRefs in an ObjArray reference BNamedObjects not BObjects
          HMMM, can this BNamedObject get destructed before we're done with it?
          No, we're making a copy, leaving the original be.
          (SO, bno's refcount should be >1 here)
          */

      BObject* bo = elem.get();
      elem.set( new BObject( bo->impptr()->copy() ) );
    }
  }
}

BObjectImp* ObjArray::copy( void ) const
{
  auto nobj = new ObjArray( *this );
  return nobj;
}

size_t ObjArray::sizeEstimate() const
{
  size_t size = sizeof( ObjArray );
  size += Clib::memsize( ref_arr );
  for ( const auto& elem : ref_arr )
  {
    size += elem.sizeEstimate();
  }
  size += Clib::memsize( name_arr );
  for ( const auto& elem : name_arr )
  {
    size += elem.capacity();
  }
  return size;
}

/**
 * Equality for arrays:
 * if the other guy is an array, check each element
 * otherwise not equal.
 * note that struct names aren't checked.
 *
 * @todo check structure names too?
 */
bool ObjArray::operator==( const BObjectImp& imp ) const
{
  if ( !imp.isa( OTArray ) )
    return false;

  const ObjArray& thatarr = static_cast<const ObjArray&>( imp );
  if ( thatarr.ref_arr.size() != ref_arr.size() )
    return false;

  for ( unsigned i = 0; i < ref_arr.size(); ++i )
  {
    const BObjectRef& thisref = ref_arr[i];
    const BObjectRef& thatref = thatarr.ref_arr[i];

    const BObject* thisobj = thisref.get();
    const BObject* thatobj = thatref.get();
    if ( thisobj != nullptr && thatobj != nullptr )
    {
      const BObjectImp& thisimp = thisobj->impref();
      const BObjectImp& thatimp = thatobj->impref();

      if ( thisimp == thatimp )
        continue;
      else
        return false;
    }
    else if ( thisobj == nullptr && thatobj == nullptr )
    {
      continue;
    }
    else
    {
      return false;
    }
  }
  return true;
}

const BObjectImp* ObjArray::imp_at( unsigned index /* 1-based */ ) const
{
  assert( index > 0 );
  if ( index > ref_arr.size() )
    return nullptr;
  const BObjectRef& ref = ref_arr[index - 1];
  if ( ref.get() == nullptr )
    return nullptr;
  return ref.get()->impptr();
}

BObjectImp* ObjArray::array_assign( BObjectImp* idx, BObjectImp* target, bool copy )
{
  if ( idx->isa( OTLong ) )
  {
    BLong& lng = (BLong&)*idx;

    unsigned index = (unsigned)lng.value();
    if ( index > ref_arr.size() )
      ref_arr.resize( index );
    else if ( index <= 0 )
      return new BError( "Array index out of bounds" );

    BObjectRef& ref = ref_arr[index - 1];
    BObject* refobj = ref.get();

    BObjectImp* new_target = copy ? target->copy() : target;

    if ( refobj != nullptr )
    {
      refobj->setimp( new_target );
    }
    else
    {
      ref.set( new BObject( new_target ) );
    }
    return ref->impptr();
  }
  else
  {
    return UninitObject::create();
  }
}

void ObjArray::operInsertInto( BObject& /*obj*/, const BObjectImp& objimp )
{
  ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}

BObjectImp* ObjArray::selfPlusObj( const BObjectImp& objimp ) const
{
  std::unique_ptr<ObjArray> result( new ObjArray( *this ) );
  result->ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
  return result.release();
}
BObjectImp* ObjArray::selfPlusObj( const BLong& objimp ) const
{
  std::unique_ptr<ObjArray> result( new ObjArray( *this ) );
  result->ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
  return result.release();
}
BObjectImp* ObjArray::selfPlusObj( const Double& objimp ) const
{
  std::unique_ptr<ObjArray> result( new ObjArray( *this ) );
  result->ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
  return result.release();
}
BObjectImp* ObjArray::selfPlusObj( const String& objimp ) const
{
  std::unique_ptr<ObjArray> result( new ObjArray( *this ) );
  result->ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
  return result.release();
}

BObjectImp* ObjArray::selfPlusObj( const ObjArray& objimp ) const
{
  std::unique_ptr<ObjArray> result( new ObjArray( *this ) );

  for ( const auto& elem : objimp.ref_arr )
  {
    if ( elem.get() )
    {
      /*
          NOTE: all BObjectRefs in an ObjArray reference BNamedObjects not BObjects
          HMMM, can this BNamedObject get destructed before we're done with it?
          No, we're making a copy, leaving the original be.
          (SO, bno's refcount should be >1 here)
          */
      BObject* bo = elem.get();

      result->ref_arr.push_back( BObjectRef( new BObject( ( *bo )->copy() ) ) );
    }
    else
    {
      result->ref_arr.push_back( BObjectRef() );
    }
  }
  return result.release();
}

BObjectImp* ObjArray::selfPlusObjImp( const BObjectImp& other ) const
{
  return other.selfPlusObj( *this );
}
void ObjArray::selfPlusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfPlusObj( *this, obj );
}
void ObjArray::selfPlusObj( BObjectImp& objimp, BObject& /*obj*/ )
{
  ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}
void ObjArray::selfPlusObj( BLong& objimp, BObject& /*obj*/ )
{
  ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}
void ObjArray::selfPlusObj( Double& objimp, BObject& /*obj*/ )
{
  ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}
void ObjArray::selfPlusObj( String& objimp, BObject& /*obj*/ )
{
  ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}
void ObjArray::selfPlusObj( ObjArray& objimp, BObject& /*obj*/ )
{
  for ( const_iterator itr = objimp.ref_arr.begin(), itrend = objimp.ref_arr.end(); itr != itrend;
        ++itr )
  {
    if ( itr->get() )
    {
      /*
          NOTE: all BObjectRefs in an ObjArray reference BNamedObjects not BObjects
          HMMM, can this BNamedObject get destructed before we're done with it?
          No, we're making a copy, leaving the original be.
          (SO, bno's refcount should be >1 here)
          */
      BObject* bo = itr->get();

      ref_arr.push_back( BObjectRef( new BObject( ( *bo )->copy() ) ) );
    }
    else
    {
      ref_arr.push_back( BObjectRef() );
    }
  }
}

BObjectRef ObjArray::OperMultiSubscript( std::stack<BObjectRef>& indices )
{
  BObjectRef start_ref = indices.top();
  indices.pop();
  BObjectRef length_ref = indices.top();
  indices.pop();

  BObject& length_obj = *length_ref;
  BObject& start_obj = *start_ref;

  BObjectImp& length = length_obj.impref();
  BObjectImp& start = start_obj.impref();

  // first deal with the start position.
  // return BObjectRef(new BError( "Subscript out of range" ));

  unsigned index;
  if ( start.isa( OTLong ) )
  {
    BLong& lng = (BLong&)start;
    index = (unsigned)lng.value();
    if ( index == 0 || index > ref_arr.size() )
      return BObjectRef( new BError( "Array start index out of bounds" ) );
  }
  else
    return BObjectRef( copy() );

  // now the end index

  unsigned end;
  if ( length.isa( OTLong ) )
  {
    BLong& lng = (BLong&)length;
    end = (unsigned)lng.value();
    if ( end == 0 || end > ref_arr.size() )
      return BObjectRef( new BError( "Array end index out of bounds" ) );
  }
  else
    return BObjectRef( copy() );

  auto str = new ObjArray();


  // std::unique_ptr<ObjArray> result (new ObjArray());
  unsigned i = 0;
  for ( const_iterator itr = ref_arr.begin(), itrend = ref_arr.end(); itr != itrend; ++itr )
  {
    if ( ++i < index )
      continue;
    if ( i > end )
      break;
    if ( itr->get() )
    {
      BObject* bo = itr->get();
      str->ref_arr.push_back( BObjectRef( new BObject( ( *bo )->copy() ) ) );
    }
    else
    {
      str->ref_arr.push_back( BObjectRef() );
    }
  }
  /*
  for (unsigned i = index; i < end; i++) {
  BObject *bo = ref_arr[i];
  if (bo != 0)
  str->ref_arr.push_back( BObjectRef( new BObject( (*bo)->copy() ) ) );
  else
  result->ref_arr.push_back( BObjectRef() );
  } */
  //  return result.release();
  return BObjectRef( str );
}

BObjectRef ObjArray::OperSubscript( const BObject& rightobj )
{
  const BObjectImp& right = rightobj.impref();
  if ( right.isa( OTLong ) )  // vector
  {
    BLong& lng = (BLong&)right;

    unsigned index = (unsigned)lng.value();
    if ( index > ref_arr.size() )
    {
      return BObjectRef( new BError( "Array index out of bounds" ) );
    }
    else if ( index <= 0 )
      return BObjectRef( new BError( "Array index out of bounds" ) );

    BObjectRef& ref = ref_arr[index - 1];
    if ( ref.get() == nullptr )
      ref.set( new BObject( UninitObject::create() ) );
    return ref;
  }
  else if ( right.isa( OTString ) )
  {
    // TODO: search for named variables (structure members)
    return BObjectRef( copy() );
  }
  else
  {
    // TODO: crap out
    return BObjectRef( copy() );
  }
}

BObjectRef ObjArray::get_member( const char* membername )
{
  int i = 0;
  for ( const_name_iterator itr = name_arr.begin(), end = name_arr.end(); itr != end; ++itr, ++i )
  {
    const std::string& name = ( *itr );
    if ( stricmp( name.c_str(), membername ) == 0 )
    {
      return ref_arr[i];
    }
  }

  return BObjectRef( UninitObject::create() );
}

BObjectRef ObjArray::set_member( const char* membername, BObjectImp* valueimp, bool copy )
{
  int i = 0;
  for ( const_name_iterator itr = name_arr.begin(), end = name_arr.end(); itr != end; ++itr, ++i )
  {
    const std::string& name = ( *itr );
    if ( stricmp( name.c_str(), membername ) == 0 )
    {
      BObjectImp* target = copy ? valueimp->copy() : valueimp;
      ref_arr[i].get()->setimp( target );
      return ref_arr[i];
    }
  }
  return BObjectRef( UninitObject::create() );
}

BObjectRef ObjArray::operDotPlus( const char* name )
{
  for ( auto& elem : name_arr )
  {
    if ( stricmp( name, elem.c_str() ) == 0 )
    {
      return BObjectRef( new BError( "Member already exists" ) );
    }
  }
  name_arr.push_back( name );
  auto pnewobj = new BObject( UninitObject::create() );
  ref_arr.push_back( BObjectRef( pnewobj ) );
  return BObjectRef( pnewobj );
}

void ObjArray::addElement( BObjectImp* imp )
{
  ref_arr.push_back( BObjectRef( new BObject( imp ) ) );
}

std::string ObjArray::getStringRep() const
{
  OSTRINGSTREAM os;
  os << "{ ";
  bool any = false;
  for ( const auto& elem : ref_arr )
  {
    if ( any )
      os << ", ";
    else
      any = true;

    BObject* bo = elem.get();

    if ( bo != nullptr )
    {
      std::string tmp = bo->impptr()->getStringRep();
      os << tmp;
    }
  }
  os << " }";

  return OSTRINGSTREAM_STR( os );
}

long ObjArray::contains( const BObjectImp& imp ) const
{
  for ( auto itr = ref_arr.begin(), end = ref_arr.end(); itr != end; ++itr )
  {
    if ( itr->get() )
    {
      BObject* bo = ( itr->get() );
      if ( bo == nullptr )
      {
        INFO_PRINTLN( "{} - '{} in array{{}}' check. Invalid data at index {}",
                      Clib::scripts_thread_script, imp, ( itr - ref_arr.begin() ) + 1 );
        continue;
      }
      else if ( *( bo->impptr() ) == imp )
      {
        return ( static_cast<long>( ( itr - ref_arr.begin() ) + 1 ) );
      }
    }
  }
  return 0;
}

class objref_cmp
{
public:
  bool operator()( const BObjectRef& x1, const BObjectRef& x2 ) const
  {
    const BObject* b1 = x1.get();
    const BObject* b2 = x2.get();
    if ( b1 == nullptr || b2 == nullptr )
      return ( &x1 < &x2 );

    const BObject& r1 = *b1;
    const BObject& r2 = *b2;
    return ( r1 < r2 );
  }
};

BObjectImp* ObjArray::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
{
  switch ( id )
  {
  case MTH_SIZE:
    if ( ex.numParams() == 0 )
      return new BLong( static_cast<int>( ref_arr.size() ) );
    else
      return new BError( "array.size() doesn't take parameters." );

  case MTH_ERASE:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 1 )
      {
        int idx;
        if ( ex.getParam( 0, idx, 1, static_cast<int>( ref_arr.size() ) ) )  // 1-based index
        {
          ref_arr.erase( ref_arr.begin() + idx - 1 );
          return new BLong( 1 );
        }
        else
        {
          return nullptr;
        }
      }
      else
        return new BError( "array.erase(index) requires a parameter." );
    }
    break;
  case MTH_EXISTS:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 1 )
      {
        int idx;
        if ( ex.getParam( 0, idx ) && idx >= 0 )
        {
          bool exists = ( idx <= (int)ref_arr.size() );
          return new BLong( exists ? 1 : 0 );
        }
        else
        {
          return new BError( "Invalid parameter type" );
        }
      }
      else
        return new BError( "array.exists(index) requires a parameter." );
    }
    break;
  case MTH_INSERT:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 2 )
      {
        int idx;
        BObjectImp* imp = ex.getParamImp( 1 );
        if ( ex.getParam( 0, idx, 1, static_cast<int>( ref_arr.size() + 1 ) ) &&
             imp != nullptr )  // 1-based index
        {
          --idx;
          BObjectRef tmp;
          ref_arr.insert( ref_arr.begin() + idx, tmp );
          BObjectRef& ref = ref_arr[idx];
          ref.set( new BObject( imp->copy() ) );
        }
        else
        {
          return new BError( "Invalid parameter type" );
        }
      }
      else
        return new BError( "array.insert(index,value) requires two parameters." );
    }
    break;
  case MTH_SHRINK:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 1 )
      {
        int idx;
        if ( ex.getParam( 0, idx, 0, static_cast<int>( ref_arr.size() ) ) )  // 1-based index
        {
          ref_arr.erase( ref_arr.begin() + idx, ref_arr.end() );
          return new BLong( 1 );
        }
        else
        {
          return new BError( "Invalid parameter type" );
        }
      }
      else
        return new BError( "array.shrink(nelems) requires a parameter." );
    }
    break;
  case MTH_APPEND:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 1 )
      {
        BObjectImp* imp = ex.getParamImp( 0 );
        if ( imp )
        {
          ref_arr.push_back( BObjectRef( new BObject( imp->copy() ) ) );

          return new BLong( 1 );
        }
        else
        {
          return new BError( "Invalid parameter type" );
        }
      }
      else
        return new BError( "array.append(value) requires a parameter." );
    }
    break;
  case MTH_REVERSE:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 0 )
      {
        reverse( ref_arr.begin(), ref_arr.end() );
        return new BLong( 1 );
      }
      else
        return new BError( "array.reverse() doesn't take parameters." );
    }
    break;
  case MTH_SORT:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 0 )
      {
        sort( ref_arr.begin(), ref_arr.end(), objref_cmp() );
        return new BLong( 1 );
      }
      else if ( ex.numParams() == 1 )
      {
        int sub_index;
        if ( !ex.getParam( 0, sub_index ) )
          return new BError( "Invalid parameter type" );
        if ( sub_index < 1 )
          return new BError( "Invalid sub_index value" );
        for ( const auto& ref : ref_arr )
        {
          if ( ref.get() == nullptr || !ref.get()->isa( OTArray ) )
            return new BError( "Invalid array" );
          auto sub_arr = ref.get()->impptr<ObjArray>();
          if ( sub_arr->ref_arr.size() < static_cast<size_t>( sub_index ) )
            return new BError( "Subindex to large" );
        }
        sort( ref_arr.begin(), ref_arr.end(),
              [=]( const BObjectRef& x1, const BObjectRef& x2 ) -> bool
              {
                auto sub_arr1 = x1.get()->impptr<ObjArray>();
                auto sub_arr2 = x2.get()->impptr<ObjArray>();
                auto sub1 = sub_arr1->ref_arr[sub_index - 1];
                auto sub2 = sub_arr2->ref_arr[sub_index - 1];
                const BObject* b1 = sub1.get();
                const BObject* b2 = sub2.get();
                if ( b1 == nullptr || b2 == nullptr )
                  return ( &x1 < &x2 );
                return ( *b1 < *b2 );
              } );
        return new BLong( 1 );
      }
      else
        return new BError( "array.sort(sub_index=0) takes at most one parameter." );
    }
    break;
  case MTH_RANDOMENTRY:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 0 )
      {
        if ( !ref_arr.empty() )
        {
          const BObjectRef& ref =
              ref_arr[Clib::random_int( static_cast<int>( ref_arr.size() ) - 1 )];
          if ( ref.get() == nullptr )
            return nullptr;
          return ref.get()->impptr();
        }
      }
      else
        return new BError( "array.randomentry() doesn't take parameters." );
    }
    break;
  case MTH_FILTER:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() < 1 )
        return new BError( "Invalid parameter type" );

      BObjectImp* param0 = ex.getParamImp( 0, BObjectType::OTFuncRef );

      if ( !param0 )
        return new BError( "Invalid parameter type" );

      // The filter callback allows optional arguments, so no need to check the
      // number of arguments for the passed function reference. Arguments passed
      // will be shrunk and expanded (with uninit) as needed.

      // If nothing to filter, return an empty array, since nothing to call the function with.
      if ( ref_arr.empty() )
        return new ObjArray();

      // Arguments for user function call.
      // - the element
      // - the index of the element
      // - the array itself
      BObjectRefVec args;
      args.push_back( ref_arr.front() );
      args.push_back( BObjectRef( new BLong( 1 ) ) );
      args.push_back( BObjectRef( this ) );

      // The ContinuationCallback receives three arguments:
      //
      // - `Executor&`
      // - `BContinuation* continuation`: The continuation, with methods to handle
      //   the continuation (call the function again; finalize)
      // - `BObjectRef result`: The result of the user function call specified in
      //   `makeContinuation`.
      //
      // We pass to the lambda a reference to the element in case the user
      // function modifies ref_arr.
      //
      // Returns a `BObjectImp`:
      // - Call the user function again by returning the same continuation via
      //   `ex.withContinuation`.
      // - Return something else (in this case, the filtered array) to provide
      //   that value back to the script.
      auto callback = [elementRef = args[0], processed = 1, thisArray = args[2],
                       filteredRef = BObjectRef( new ObjArray ),
                       initialSize = static_cast<int>( ref_arr.size() )](
                          Executor& ex, BContinuation* continuation,
                          BObjectRef result ) mutable -> BObjectImp*
      {
        auto filtered = filteredRef->impptr<ObjArray>();

        // Do something with result.
        // If the result is true, add it to the filtered array.
        if ( result->isTrue() )
        {
          filtered->ref_arr.push_back( BObjectRef( elementRef->impptr() ) );
        }

        // If thisArray was modified for some reason to no longer be an array,
        // return the filtered array.
        if ( !thisArray->isa( OTArray ) )
          return filtered;

        const auto& ref_arr = thisArray->impptr<ObjArray>()->ref_arr;

        // If the processed index is the last element, return the filtered
        // array. Also check if the processed index is greater than the initial
        // size of the array, as the user function may have modified the array.
        if ( processed >= initialSize || processed >= static_cast<int>( ref_arr.size() ) )
        {
          return filtered;
        }
        // Otherwise, increment the processed index and call the function again.
        else
        {
          // Increment the processed counter.
          ++processed;

          BObjectRefVec args;
          args.push_back( ref_arr[processed - 1] );
          args.push_back( BObjectRef( new BObject( new BLong( processed ) ) ) );
          args.push_back( thisArray );

          elementRef = args[0];

          // Return this continuation with the new arguments.
          return ex.withContinuation( continuation, std::move( args ) );
        }
      };

      // Create a new continuation for a user function call.
      return ex.makeContinuation( BObjectRef( new BObject( param0 ) ), callback,
                                  std::move( args ) );
    }
    break;

  case MTH_MAP:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() < 1 )
        return new BError( "Invalid parameter type" );

      BObjectImp* param0 = ex.getParamImp( 0, BObjectType::OTFuncRef );

      if ( !param0 )
        return new BError( "Invalid parameter type" );

      if ( ref_arr.empty() )
        return new ObjArray();

      // Arguments for user function call.
      // - the element
      // - the index of the element
      // - the array itself
      BObjectRefVec args;
      args.push_back( ref_arr.front() );
      args.push_back( BObjectRef( new BLong( 1 ) ) );
      args.push_back( BObjectRef( this ) );

      auto callback = [elementRef = args[0], processed = 1, thisArray = args[2],
                       mappedRef = BObjectRef( new ObjArray ),
                       initialSize = static_cast<int>( ref_arr.size() )](
                          Executor& ex, BContinuation* continuation,
                          BObjectRef result ) mutable -> BObjectImp*
      {
        auto mapped = mappedRef->impptr<ObjArray>();

        mapped->ref_arr.push_back( BObjectRef( result->impptr() ) );

        if ( !thisArray->isa( OTArray ) )
          return mapped;

        const auto& ref_arr = thisArray->impptr<ObjArray>()->ref_arr;

        if ( processed >= initialSize || processed >= static_cast<int>( ref_arr.size() ) )
        {
          return mapped;
        }
        else
        {
          // Increment the processed counter.
          ++processed;

          BObjectRefVec args;
          args.push_back( ref_arr[processed - 1] );
          args.push_back( BObjectRef( new BObject( new BLong( processed ) ) ) );
          args.push_back( thisArray );

          elementRef = args[0];

          return ex.withContinuation( continuation, std::move( args ) );
        }
      };

      return ex.makeContinuation( BObjectRef( new BObject( param0 ) ), callback,
                                  std::move( args ) );
    }
    break;

  case MTH_REDUCE:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() < 1 )
        return new BError( "Invalid parameter type" );

      BObjectImp* param0 = ex.getParamImp( 0, BObjectType::OTFuncRef );

      if ( !param0 )
        return new BError( "Invalid parameter type" );

      BObjectImp* accumulator;
      int processed;

      // If an initial accumulator value was passed in, use it. Otherwise, use
      // the first element of the array, erroring if the array is empty.
      if ( ex.numParams() > 1 )
      {
        accumulator = ex.getParamImp( 1 );
        processed = 1;
      }
      else if ( ref_arr.empty() )
      {
        return new BError( "Reduce of empty array with no initial value" );
      }
      else
      {
        accumulator = ref_arr[0]->impptr();
        processed = 2;
      }

      // Return the accumulator if there is no more to process, eg:
      // {}.reduce(@{}, "accum") or {"accum"}.reduce(@{})
      if ( processed > static_cast<int>( ref_arr.size() ) )
      {
        return accumulator;
      }

      // Arguments for user function call.
      // - accumulator
      // - current value
      // - current index
      // - the array itself
      BObjectRefVec args;
      args.push_back( BObjectRef( accumulator ) );
      args.push_back( BObjectRef( ref_arr[processed - 1] ) );
      args.push_back( BObjectRef( new BLong( processed ) ) );
      args.push_back( BObjectRef( this ) );

      auto callback = [thisArray = args[3], processed = processed,
                       initialSize = static_cast<int>( ref_arr.size() )](
                          Executor& ex, BContinuation* continuation,
                          BObjectRef result /* accumulator */ ) mutable -> BObjectImp*
      {
        if ( !thisArray->isa( OTArray ) )
          return result->impptr();

        const auto& ref_arr = thisArray->impptr<ObjArray>()->ref_arr;

        if ( processed >= initialSize || processed >= static_cast<int>( ref_arr.size() ) )
        {
          return result->impptr();
        }
        else
        {
          ++processed;

          BObjectRefVec args;
          args.push_back( result );
          args.push_back( ref_arr[processed - 1] );
          args.push_back( BObjectRef( new BObject( new BLong( processed ) ) ) );
          args.push_back( thisArray );

          return ex.withContinuation( continuation, std::move( args ) );
        }
      };

      return ex.makeContinuation( BObjectRef( new BObject( param0 ) ), callback,
                                  std::move( args ) );
    }
    break;

  case MTH_FIND:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() < 1 )
        return new BError( "Invalid parameter type" );

      BObjectImp* param0 = ex.getParamImp( 0, BObjectType::OTFuncRef );

      if ( !param0 )
        return new BError( "Invalid parameter type" );

      if ( ref_arr.empty() )
        return new ObjArray();

      // Arguments for user function call.
      // - the element
      // - the index of the element
      // - the array itself
      BObjectRefVec args;
      args.push_back( ref_arr.front() );
      args.push_back( BObjectRef( new BLong( 1 ) ) );
      args.push_back( BObjectRef( this ) );

      auto callback = [elementRef = args[0], processed = 1, thisArray = args[2],
                       initialSize = static_cast<int>( ref_arr.size() )](
                          Executor& ex, BContinuation* continuation,
                          BObjectRef result ) mutable -> BObjectImp*
      {
        if ( result->isTrue() )
        {
          return elementRef->impptr();
        }

        if ( !thisArray->isa( OTArray ) )
          return UninitObject::create();

        const auto& ref_arr = thisArray->impptr<ObjArray>()->ref_arr;

        if ( processed >= initialSize || processed >= static_cast<int>( ref_arr.size() ) )
        {
          return UninitObject::create();
        }
        else
        {
          ++processed;

          BObjectRefVec args;
          args.push_back( ref_arr[processed - 1] );
          args.push_back( BObjectRef( new BObject( new BLong( processed ) ) ) );
          args.push_back( thisArray );

          elementRef = args[0];

          return ex.withContinuation( continuation, std::move( args ) );
        }
      };

      return ex.makeContinuation( BObjectRef( new BObject( param0 ) ), callback,
                                  std::move( args ) );
    }
    break;

  case MTH_FINDINDEX:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() < 1 )
        return new BError( "Invalid parameter type" );

      BObjectImp* param0 = ex.getParamImp( 0, BObjectType::OTFuncRef );

      if ( !param0 )
        return new BError( "Invalid parameter type" );

      if ( ref_arr.empty() )
        return new BLong( 0 );

      // Arguments for user function call.
      // - the element
      // - the index of the element
      // - the array itself
      BObjectRefVec args;
      args.push_back( ref_arr.front() );
      args.push_back( BObjectRef( new BLong( 1 ) ) );
      args.push_back( BObjectRef( this ) );

      auto callback = [elementRef = args[0], processed = 1, thisArray = args[2],
                       initialSize = static_cast<int>( ref_arr.size() )](
                          Executor& ex, BContinuation* continuation,
                          BObjectRef result ) mutable -> BObjectImp*
      {
        if ( result->isTrue() )
        {
          return new BLong( processed );
        }

        if ( !thisArray->isa( OTArray ) )
          return new BLong( 0 );

        const auto& ref_arr = thisArray->impptr<ObjArray>()->ref_arr;

        if ( processed >= initialSize || processed >= static_cast<int>( ref_arr.size() ) )
        {
          return new BLong( 0 );
        }
        else
        {
          ++processed;

          BObjectRefVec args;
          args.push_back( ref_arr[processed - 1] );
          args.push_back( BObjectRef( new BObject( new BLong( processed ) ) ) );
          args.push_back( thisArray );

          elementRef->setimp( args[0]->impptr() );

          return ex.withContinuation( continuation, std::move( args ) );
        }
      };

      return ex.makeContinuation( BObjectRef( new BObject( param0 ) ), callback,
                                  std::move( args ) );
    }
    break;

  case MTH_CYCLE:
    if ( name_arr.empty() )
    {
      int shift_by;

      if ( ex.numParams() > 0 )
      {
        if ( !ex.getParam( 0, shift_by ) )
          return new BError( "Invalid parameter type" );
        if ( shift_by == 0 )
          return new BLong( 0 );
      }
      else
        shift_by = 1;

      if ( ref_arr.empty() || std::abs( shift_by ) > (int)ref_arr.size() )
        return new BLong( 0 );

      if ( shift_by > 0 )
        std::rotate( ref_arr.begin(), ref_arr.end() - shift_by, ref_arr.end() );
      else
        std::rotate( ref_arr.begin(), ref_arr.begin() - shift_by, ref_arr.end() );

      return new BLong( 1 );
    }
    break;

  case MTH_SORTEDINSERT:
  {
    if ( !name_arr.empty() )
      break;
    if ( ex.numParams() == 0 )
      return new BError(
          "array.sorted_insert(obj, sub_index:=0, reverse:=0) takes at least one parameter." );
    BObjectImp* imp = ex.getParamImp( 0 );
    if ( !imp )
      return new BError( "Invalid parameter type" );
    bool reverse = false;
    int sub_index = 0;
    if ( ex.numParams() >= 2 )
    {
      if ( !ex.getParam( 1, sub_index ) )
        return new BError( "Invalid parameter type" );
      if ( sub_index < 0 )
        return new BError( "Invalid sub_index value" );
    }
    if ( ex.numParams() >= 3 )
    {
      int reverseparam;
      if ( !ex.getParam( 2, reverseparam ) )
        return new BError( "Invalid parameter type" );
      reverse = reverseparam != 0;
    }
    BObjectRef item( new BObject( imp->copy() ) );
    if ( !sub_index )
    {
      if ( reverse )
      {
        ref_arr.insert( std::lower_bound( ref_arr.begin(), ref_arr.end(), item,
                                          []( const BObjectRef& x1, const BObjectRef& x2 ) -> bool
                                          {
                                            const BObject* b1 = x1.get();
                                            const BObject* b2 = x2.get();
                                            if ( b1 == nullptr || b2 == nullptr )
                                              return ( &x1 > &x2 );
                                            const BObject& r1 = *b1;
                                            const BObject& r2 = *b2;
                                            return ( r1 > r2 );
                                          } ),
                        item );
      }
      else
      {
        ref_arr.insert( std::upper_bound( ref_arr.begin(), ref_arr.end(), item, objref_cmp() ),
                        item );
      }
    }
    else
    {
      auto cmp_func = [=]( const BObjectRef& x1, const BObjectRef& x2 ) -> bool
      {
        if ( x1.get() == nullptr || !x1.get()->isa( OTArray ) )
          return false;
        if ( x2.get() == nullptr || !x2.get()->isa( OTArray ) )
          return false;
        auto sub_arr1 = x1.get()->impptr<ObjArray>();
        auto sub_arr2 = x2.get()->impptr<ObjArray>();
        if ( sub_arr1->ref_arr.size() < static_cast<size_t>( sub_index ) )
          return false;
        if ( sub_arr2->ref_arr.size() < static_cast<size_t>( sub_index ) )
          return false;
        auto sub1 = sub_arr1->ref_arr[sub_index - 1];
        auto sub2 = sub_arr2->ref_arr[sub_index - 1];
        const BObject* b1 = sub1.get();
        const BObject* b2 = sub2.get();
        if ( !reverse )
        {
          if ( b1 == nullptr || b2 == nullptr )
            return ( &x1 < &x2 );
          return ( *b1 < *b2 );
        }
        else
        {
          if ( b1 == nullptr || b2 == nullptr )
            return ( &x1 > &x2 );
          return ( *b1 > *b2 );
        }
      };
      if ( reverse )
      {
        ref_arr.insert( std::lower_bound( ref_arr.begin(), ref_arr.end(), item, cmp_func ), item );
      }
      else
      {
        ref_arr.insert( std::upper_bound( ref_arr.begin(), ref_arr.end(), item, cmp_func ), item );
      }
    }
    return new BLong( 1 );
    break;
  }
  default:
    return nullptr;
  }
  return nullptr;
}

BObjectImp* ObjArray::call_method( const char* methodname, Executor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_method_id( objmethod->id, ex );
  else
    return nullptr;
}

void ObjArray::packonto( std::ostream& os ) const
{
  os << "a" << ref_arr.size() << ":";
  for ( const auto& elem : ref_arr )
  {
    if ( elem.get() )
    {
      BObject* bo = elem.get();
      bo->impptr()->packonto( os );
    }
    else
    {
      os << "x";
    }
  }
}

BObjectImp* ObjArray::unpack( std::istream& is )
{
  unsigned arrsize;
  char colon;
  if ( !( is >> arrsize >> colon ) )
  {
    return new BError( "Unable to unpack array elemcount" );
  }
  if ( (int)arrsize < 0 )
  {
    return new BError( "Unable to unpack array elemcount. Invalid length!" );
  }
  if ( colon != ':' )
  {
    return new BError( "Unable to unpack array elemcount. Bad format. Colon not found!" );
  }
  std::unique_ptr<ObjArray> arr( new ObjArray );
  arr->ref_arr.resize( arrsize );
  for ( unsigned i = 0; i < arrsize; ++i )
  {
    BObjectImp* imp = BObjectImp::unpack( is );
    if ( imp != nullptr && !imp->isa( OTUninit ) )
    {
      arr->ref_arr[i].set( new BObject( imp ) );
    }
  }
  return arr.release();
}

BApplicPtr::BApplicPtr( const BApplicObjType* pointer_type, void* ptr )
    : BObjectImp( OTApplicPtr ), ptr_( ptr ), pointer_type_( pointer_type )
{
}

BObjectImp* BApplicPtr::copy() const
{
  return new BApplicPtr( pointer_type_, ptr_ );
}

size_t BApplicPtr::sizeEstimate() const
{
  return sizeof( BApplicPtr );
}

const BApplicObjType* BApplicPtr::pointer_type() const
{
  return pointer_type_;
}

void* BApplicPtr::ptr() const
{
  return ptr_;
}

std::string BApplicPtr::getStringRep() const
{
  return "<appptr>";
}


void BApplicPtr::printOn( std::ostream& os ) const
{
  os << "<appptr>";
}

std::string BApplicObjBase::getStringRep() const
{
  return std::string( "<appobj:" ) + typeOf() + ">";
}

void BApplicObjBase::printOn( std::ostream& os ) const
{
  os << getStringRep();
}

#if BOBJECTIMP_DEBUG
BBoolean::BBoolean( bool bval ) : BObjectImp( OTBoolean ), bval_( bval ) {}
BBoolean::BBoolean( const BBoolean& B ) : BBoolean( B.bval_ ) {}
#endif

BObjectImp* BBoolean::unpack( std::istream& is )
{
  int lv;
  if ( is >> lv )
  {
    return new BBoolean( lv != 0 );
  }
  else
  {
    return new BError( "Error extracting Boolean value" );
  }
}

void BBoolean::packonto( std::ostream& os ) const
{
  os << "b" << ( bval_ ? 1 : 0 );
}

std::string BBoolean::pack() const
{
  OSTRINGSTREAM os;
  os << "b" << ( bval_ ? 1 : 0 );
  return OSTRINGSTREAM_STR( os );
}

BObjectImp* BBoolean::copy() const
{
  return new BBoolean( *this );
}

size_t BBoolean::sizeEstimate() const
{
  return sizeof( BBoolean );
}

bool BBoolean::isTrue() const
{
  return bval_;
}

bool BBoolean::operator==( const BObjectImp& objimp ) const
{
  return bval_ == objimp.isTrue();
}

std::string BBoolean::getStringRep() const
{
  return bval_ ? "true" : "false";
}

BFunctionRef::BFunctionRef( ref_ptr<EScriptProgram> program, unsigned function_reference_index,
                            std::shared_ptr<ValueStackCont> globals, ValueStackCont&& captures )
    : BObjectImp( OTFuncRef ),
      prog_( std::move( program ) ),
      function_reference_index_( function_reference_index ),
      globals( std::move( globals ) ),
      captures( std::move( captures ) )
{
  passert( function_reference_index_ < prog_->function_references.size() );
}

BFunctionRef::BFunctionRef( const BFunctionRef& B )
    : BFunctionRef( B.prog_, B.function_reference_index_, B.globals, ValueStackCont( B.captures ) )
{
}

BObjectImp* BFunctionRef::copy() const
{
  return new BFunctionRef( *this );
}

size_t BFunctionRef::sizeEstimate() const
{
  return sizeof( BFunctionRef ) + Clib::memsize( captures ) + prog_->sizeEstimate();
}

bool BFunctionRef::isTrue() const
{
  return false;
}

bool BFunctionRef::operator==( const BObjectImp& /*objimp*/ ) const
{
  return false;
}

BObjectImp* BFunctionRef::selfIsObjImp( const BObjectImp& other ) const
{
  auto classinstref = dynamic_cast<const BClassInstanceRef*>( &other );
  if ( !classinstref )
    return new BLong( 0 );

  auto classinst = classinstref->instance();

  // Class index could be maxint if the function reference is not a class
  // method.
  if ( class_index() >= prog_->class_descriptors.size() )
    return new BLong( 0 );

  const auto& my_constructors = prog_->class_descriptors.at( class_index() ).constructors;
  passert( !my_constructors.empty() );
  const auto& my_descriptor = my_constructors.front();

  const auto& other_descriptors =
      classinst->prog()->class_descriptors.at( classinst->index() ).constructors;

  // An optimization for same program: since strings are never duplicated inside
  // the data section, we can just check for offset equality.
  if ( prog_ == classinst->prog() )
  {
    for ( const auto& other_descriptor : other_descriptors )
    {
      if ( my_descriptor.type_tag_offset == other_descriptor.type_tag_offset )
        return new BLong( 1 );
    }
  }
  // For different programs, we must check for string equality.
  else
  {
    auto type_tag = prog_->symbols.array() + my_descriptor.type_tag_offset;
    for ( const auto& other_descriptor : other_descriptors )
    {
      auto other_type_tag = classinst->prog()->symbols.array() + other_descriptor.type_tag_offset;
      if ( strcmp( type_tag, other_type_tag ) == 0 )
        return new BLong( 1 );
    }
  }

  return new BLong( 0 );
}

std::string BFunctionRef::getStringRep() const
{
  return "FunctionObject";
}

BObjectImp* BFunctionRef::call_method( const char* methodname, Executor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return call_method_id( objmethod->id, ex );
  return nullptr;
}

bool BFunctionRef::validCall( const int id, Executor& ex, Instruction* inst ) const
{
  auto passed_args = static_cast<int>( ex.numParams() );

  auto [expected_min_args, expected_max_args] = expected_args();

  if ( id != MTH_CALL && id != MTH_NEW && id != MTH_CALL_METHOD )
    return false;

  if ( passed_args < expected_min_args || ( passed_args > expected_max_args && !variadic() ) )
    return false;

  inst->func = &Executor::ins_nop;

  if ( passed_args >= expected_max_args )
  {
    inst->token.lval = pc();
  }
  else
  {
    auto default_parameter_address_index = expected_max_args - passed_args - 1;
    passert( default_parameter_address_index >= 0 &&
             default_parameter_address_index <
                 static_cast<int>( default_parameter_addresses().size() ) );
    inst->token.lval = default_parameter_addresses().at( default_parameter_address_index );
  }

  return true;
}

bool BFunctionRef::validCall( const char* methodname, Executor& ex, Instruction* inst ) const
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod == nullptr )
    return false;
  return validCall( objmethod->id, ex, inst );
}

int BFunctionRef::numParams() const
{
  return prog_->function_references[function_reference_index_].parameter_count;
}

unsigned BFunctionRef::pc() const
{
  return prog_->function_references[function_reference_index_].address;
}

bool BFunctionRef::variadic() const
{
  return prog_->function_references[function_reference_index_].is_variadic;
}

ref_ptr<EScriptProgram> BFunctionRef::prog() const
{
  return prog_;
}

unsigned BFunctionRef::class_index() const
{
  return prog_->function_references[function_reference_index_].class_index;
}

bool BFunctionRef::constructor() const
{
  return prog_->function_references[function_reference_index_].is_constructor;
}

bool BFunctionRef::class_method() const
{
  return prog_->function_references[function_reference_index_].class_index <
         std::numeric_limits<unsigned>::max();
}

const std::vector<unsigned>& BFunctionRef::default_parameter_addresses() const
{
  return prog_->function_references[function_reference_index_].default_parameter_addresses;
}

int BFunctionRef::default_parameter_count() const
{
  return static_cast<int>( default_parameter_addresses().size() );
}

std::pair<int, int> BFunctionRef::expected_args() const
{
  auto expected_min_args = numParams() - default_parameter_count();  // remove default parameters

  return { expected_min_args, expected_min_args + default_parameter_count() };
}

BObjectImp* BFunctionRef::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
{
  bool adjust_for_this = false;

  // These are only entered if `ins_call_method_id` did _not_ do the call jump.
  switch ( id )
  {
  case MTH_NEW:
    if ( !constructor() )
      return new BError( "Function is not a constructor" );
    // intentional fallthrough
  case MTH_CALL_METHOD:
    adjust_for_this = true;
    // intentional fallthrough
  case MTH_CALL:
  {
    auto [expected_min_args, expected_max_args] = expected_args();
    auto param_count = ex.numParams();

    if ( adjust_for_this )
    {
      --expected_min_args;
      --expected_max_args;
      --param_count;
    }

    auto expected_arg_string = variadic() ? fmt::format( "{}+", expected_min_args )
                               : ( expected_min_args == expected_max_args )
                                   ? std::to_string( expected_min_args )
                                   : fmt::format( "{}-{}", expected_min_args, expected_max_args );

    return new BError( fmt::format( "Invalid argument count: expected {}, got {}",
                                    expected_arg_string, param_count ) );
  }
  default:
    return nullptr;
  }
}

BSpread::BSpread( BObjectRef obj ) : BObjectImp( OTSpread ), object( obj ) {}

BSpread::BSpread( const BSpread& B ) : BObjectImp( OTSpread ), object( B.object ) {}

size_t BSpread::sizeEstimate() const
{
  return sizeof( BSpread ) + object.sizeEstimate();
}

BObjectImp* BSpread::copy() const
{
  return new BSpread( *this );
}

bool BSpread::isTrue() const
{
  return object->isTrue();
}

std::string BSpread::getStringRep() const
{
  return "Spread";
}

}  // namespace Bscript
}  // namespace Pol
