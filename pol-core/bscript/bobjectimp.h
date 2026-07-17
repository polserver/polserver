#pragma once

#include "../clib/rawtypes.h"
#include "../clib/refptr.h"

#include <stack>
#include <string>
#include <type_traits>

#ifdef NDEBUG
#define BOBJECTIMP_DEBUG 0
#else
#define BOBJECTIMP_DEBUG 1
#endif

#if BOBJECTIMP_DEBUG
#define INLINE_BOBJECTIMP_CTOR 0
#else
#define INLINE_BOBJECTIMP_CTOR 1
#endif

#if BOBJECTIMP_DEBUG
#include "../clib/spinlock.h"
#endif
#if INLINE_BOBJECTIMP_CTOR
#include "escriptv.h"
#endif

namespace Pol::Bscript
{
class BLong;
class Double;
class String;
class ObjArray;
class UninitObject;
class BError;
class BStruct;
class BDictionary;
class BBoolean;
class BFunctionRef;
class BClassInstance;
class BClassInstanceRef;
class BContinuation;
class BSpread;
class BRegExp;
class BSpecialUserFuncJump;
class BApplicObjBase;

class BObject;
class BObjectRef;
class ContIterator;

class Executor;

class BObjectImp : public ref_counted
{
public:
  /**
   * Specify the object type for the child classes
   *
   * @warning This is directly returned by TypeOfInt(), so don't forget to
   *          keep constants inside basic.em in sync! It is better to always
   *          add new values at the end and use explicit int conversion to
   *          avoid breaking backward compatibility.
   */
  enum BObjectType : u8
  {
    OTUnknown = 0,
    OTUninit = 1,
    OTString = 2,
    OTLong = 3,
    OTDouble = 4,
    OTArray = 5,
    // unused
    OTApplicObj = 7,
    OTError = 8,
    OTDictionary = 9,
    OTStruct = 10,
    OTPacket = 11,
    OTBinaryFile = 12,
    OTXMLFile = 13,
    OTXMLNode = 14,
    OTXMLAttributes = 15,
    OTPolCoreRef = 16,

    // non direct used constants (for TypeOfInt) start
    OTAccountRef = 17,
    OTConfigFileRef = 18,
    OTConfigElemRef = 19,
    OTDataFileRef = 20,
    OTDataElemRef = 21,
    OTGuildRef = 22,
    OTPartyRef = 23,
    OTBoundingBox = 24,
    OTDebugContext = 25,
    OTScriptExRef = 26,
    OTPackage = 27,
    OTMenuRef = 28,
    OTMobileRef = 29,
    OTOfflineMobileRef = 30,
    OTItemRef = 31,
    OTBoatRef = 32,
    OTMultiRef = 33,
    OTClientRef = 34,
    // non direct used Constants (for TypeOfInt) end

    OTSQLConnection = 35,
    OTSQLResultSet = 36,
    OTSQLRow = 37,
    OTBoolean = 38,
    OTFuncRef = 39,
    OTExportScript = 40,
    OTStorageArea = 41,
    OTClassInstanceRef = 42,
    OTRegExp = 43,

    // Used internally only during executor runtime. Can be modified without
    // breaking compatibility.
    OTContinuation = 100,
    OTSpread = 101,
    OTClassInstance = 102,
    OTSpecialUserFuncJump = 103,
  };

#if INLINE_BOBJECTIMP_CTOR
  explicit BObjectImp( BObjectType type ) : type_( type )
  {
    ++eobject_imp_count;
    ++eobject_imp_constructions;
  }
  ~BObjectImp() override { --eobject_imp_count; }
#else
  explicit BObjectImp( BObjectType type );
  virtual ~BObjectImp();
#endif


  // Class Machinery:
  static BObjectImp* unpack( const char* pstr );
  static BObjectImp* unpack( std::istream& is );
  bool isa( BObjectType type ) const;
  BObjectType type() const;
#if BOBJECTIMP_DEBUG
  unsigned int instance() const { return instance_; }
#endif
  static const char* typestr( BObjectType typ );

  virtual BObjectImp* copy() const = 0;
  virtual std::string getStringRep() const = 0;
  virtual std::string getFormattedStringRep() const;

  virtual size_t sizeEstimate() const = 0;
  virtual const char* typeOf() const;
  virtual u8 typeOfInt() const;


  virtual std::string pack() const;
  virtual void packonto( std::string& os ) const;

  virtual bool operator==( const BObjectImp& objimp ) const;
  virtual bool operator<( const BObjectImp& objimp ) const;
  virtual bool operator<=( const BObjectImp& objimp ) const;
  virtual bool operator>( const BObjectImp& objimp ) const;
  virtual bool operator>=( const BObjectImp& objimp ) const;
  virtual bool operator!=( const BObjectImp& objimp ) const;

  virtual BObjectImp* selfIsObjImp( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfIsObj( const BObjectImp& objimp ) const;

  virtual BObjectImp* selfPlusObjImp( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfPlusObj( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfPlusObj( const BLong& objimp ) const;
  virtual BObjectImp* selfPlusObj( const Double& objimp ) const;
  virtual BObjectImp* selfPlusObj( const String& objimp ) const;
  virtual BObjectImp* selfPlusObj( const ObjArray& objimp ) const;
  virtual void selfPlusObjImp( BObjectImp& objimp, BObject& obj );
  virtual void selfPlusObj( BObjectImp& objimp, BObject& obj );
  virtual void selfPlusObj( BLong& objimp, BObject& obj );
  virtual void selfPlusObj( Double& objimp, BObject& obj );
  virtual void selfPlusObj( String& objimp, BObject& obj );
  virtual void selfPlusObj( ObjArray& objimp, BObject& obj );

  virtual BObjectImp* selfMinusObjImp( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfMinusObj( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfMinusObj( const BLong& objimp ) const;
  virtual BObjectImp* selfMinusObj( const Double& objimp ) const;
  virtual BObjectImp* selfMinusObj( const String& objimp ) const;
  virtual BObjectImp* selfMinusObj( const ObjArray& objimp ) const;
  virtual void selfMinusObjImp( BObjectImp& objimp, BObject& obj );
  virtual void selfMinusObj( BObjectImp& objimp, BObject& obj );
  virtual void selfMinusObj( BLong& objimp, BObject& obj );
  virtual void selfMinusObj( Double& objimp, BObject& obj );
  virtual void selfMinusObj( String& objimp, BObject& obj );
  virtual void selfMinusObj( ObjArray& objimp, BObject& obj );

  virtual BObjectImp* selfTimesObjImp( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfTimesObj( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfTimesObj( const BLong& objimp ) const;
  virtual BObjectImp* selfTimesObj( const Double& objimp ) const;
  virtual BObjectImp* selfTimesObj( const String& objimp ) const;
  virtual BObjectImp* selfTimesObj( const ObjArray& objimp ) const;
  virtual void selfTimesObjImp( BObjectImp& objimp, BObject& obj );
  virtual void selfTimesObj( BObjectImp& objimp, BObject& obj );
  virtual void selfTimesObj( BLong& objimp, BObject& obj );
  virtual void selfTimesObj( Double& objimp, BObject& obj );
  virtual void selfTimesObj( String& objimp, BObject& obj );
  virtual void selfTimesObj( ObjArray& objimp, BObject& obj );

  virtual BObjectImp* selfDividedByObjImp( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfDividedByObj( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfDividedByObj( const BLong& objimp ) const;
  virtual BObjectImp* selfDividedByObj( const Double& objimp ) const;
  virtual BObjectImp* selfDividedByObj( const String& objimp ) const;
  virtual BObjectImp* selfDividedByObj( const ObjArray& objimp ) const;
  virtual void selfDividedByObjImp( BObjectImp& objimp, BObject& obj );
  virtual void selfDividedByObj( BObjectImp& objimp, BObject& obj );
  virtual void selfDividedByObj( BLong& objimp, BObject& obj );
  virtual void selfDividedByObj( Double& objimp, BObject& obj );
  virtual void selfDividedByObj( String& objimp, BObject& obj );
  virtual void selfDividedByObj( ObjArray& objimp, BObject& obj );

  virtual BObjectImp* selfModulusObjImp( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfModulusObj( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfModulusObj( const BLong& objimp ) const;
  virtual BObjectImp* selfModulusObj( const Double& objimp ) const;
  virtual BObjectImp* selfModulusObj( const String& objimp ) const;
  virtual BObjectImp* selfModulusObj( const ObjArray& objimp ) const;
  virtual void selfModulusObjImp( BObjectImp& objimp, BObject& obj );
  virtual void selfModulusObj( BObjectImp& objimp, BObject& obj );
  virtual void selfModulusObj( BLong& objimp, BObject& obj );
  virtual void selfModulusObj( Double& objimp, BObject& obj );
  virtual void selfModulusObj( String& objimp, BObject& obj );
  virtual void selfModulusObj( ObjArray& objimp, BObject& obj );

  virtual BObjectImp* selfBitShiftRightObjImp( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfBitShiftRightObj( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfBitShiftRightObj( const BLong& objimp ) const;
  virtual BObjectImp* selfBitShiftRightObj( const Double& objimp ) const;
  virtual BObjectImp* selfBitShiftRightObj( const String& objimp ) const;
  virtual BObjectImp* selfBitShiftRightObj( const ObjArray& objimp ) const;
  virtual void selfBitShiftRightObjImp( BObjectImp& objimp, BObject& obj );
  virtual void selfBitShiftRightObj( BObjectImp& objimp, BObject& obj );
  virtual void selfBitShiftRightObj( BLong& objimp, BObject& obj );
  virtual void selfBitShiftRightObj( Double& objimp, BObject& obj );
  virtual void selfBitShiftRightObj( String& objimp, BObject& obj );
  virtual void selfBitShiftRightObj( ObjArray& objimp, BObject& obj );

  virtual BObjectImp* selfBitShiftLeftObjImp( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfBitShiftLeftObj( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfBitShiftLeftObj( const BLong& objimp ) const;
  virtual BObjectImp* selfBitShiftLeftObj( const Double& objimp ) const;
  virtual BObjectImp* selfBitShiftLeftObj( const String& objimp ) const;
  virtual BObjectImp* selfBitShiftLeftObj( const ObjArray& objimp ) const;
  virtual void selfBitShiftLeftObjImp( BObjectImp& objimp, BObject& obj );
  virtual void selfBitShiftLeftObj( BObjectImp& objimp, BObject& obj );
  virtual void selfBitShiftLeftObj( BLong& objimp, BObject& obj );
  virtual void selfBitShiftLeftObj( Double& objimp, BObject& obj );
  virtual void selfBitShiftLeftObj( String& objimp, BObject& obj );
  virtual void selfBitShiftLeftObj( ObjArray& objimp, BObject& obj );

  virtual BObjectImp* selfBitAndObjImp( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfBitAndObj( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfBitAndObj( const BLong& objimp ) const;
  virtual BObjectImp* selfBitAndObj( const Double& objimp ) const;
  virtual BObjectImp* selfBitAndObj( const String& objimp ) const;
  virtual BObjectImp* selfBitAndObj( const ObjArray& objimp ) const;
  virtual void selfBitAndObjImp( BObjectImp& objimp, BObject& obj );
  virtual void selfBitAndObj( BObjectImp& objimp, BObject& obj );
  virtual void selfBitAndObj( BLong& objimp, BObject& obj );
  virtual void selfBitAndObj( Double& objimp, BObject& obj );
  virtual void selfBitAndObj( String& objimp, BObject& obj );
  virtual void selfBitAndObj( ObjArray& objimp, BObject& obj );

  virtual BObjectImp* selfBitXorObjImp( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfBitXorObj( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfBitXorObj( const BLong& objimp ) const;
  virtual BObjectImp* selfBitXorObj( const Double& objimp ) const;
  virtual BObjectImp* selfBitXorObj( const String& objimp ) const;
  virtual BObjectImp* selfBitXorObj( const ObjArray& objimp ) const;
  virtual void selfBitXorObjImp( BObjectImp& objimp, BObject& obj );
  virtual void selfBitXorObj( BObjectImp& objimp, BObject& obj );
  virtual void selfBitXorObj( BLong& objimp, BObject& obj );
  virtual void selfBitXorObj( Double& objimp, BObject& obj );
  virtual void selfBitXorObj( String& objimp, BObject& obj );
  virtual void selfBitXorObj( ObjArray& objimp, BObject& obj );

  virtual BObjectImp* selfBitOrObjImp( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfBitOrObj( const BObjectImp& objimp ) const;
  virtual BObjectImp* selfBitOrObj( const BLong& objimp ) const;
  virtual BObjectImp* selfBitOrObj( const Double& objimp ) const;
  virtual BObjectImp* selfBitOrObj( const String& objimp ) const;
  virtual BObjectImp* selfBitOrObj( const ObjArray& objimp ) const;
  virtual void selfBitOrObjImp( BObjectImp& objimp, BObject& obj );
  virtual void selfBitOrObj( BObjectImp& objimp, BObject& obj );
  virtual void selfBitOrObj( BLong& objimp, BObject& obj );
  virtual void selfBitOrObj( Double& objimp, BObject& obj );
  virtual void selfBitOrObj( String& objimp, BObject& obj );
  virtual void selfBitOrObj( ObjArray& objimp, BObject& obj );

  virtual BObjectImp* bitnot() const;

  virtual BObjectRef operDotPlus( const char* name );
  virtual BObjectRef operDotMinus( const char* name );
  virtual BObjectRef operDotQMark( const char* name );

  virtual void operInsertInto( BObject& obj, const BObjectImp& objimp );

  virtual void operPlusEqual( BObject& obj, BObjectImp& objimp );
  virtual void operMinusEqual( BObject& obj, BObjectImp& objimp );
  virtual void operTimesEqual( BObject& obj, BObjectImp& objimp );
  virtual void operDivideEqual( BObject& obj, BObjectImp& objimp );
  virtual void operModulusEqual( BObject& obj, BObjectImp& objimp );

  virtual BObject operator-() const;

  virtual char /*BObjectRef */ member( const BObject& obj ) const;
  virtual char /*BObjectImp* */ str_member( const std::string& membername ) const;

  virtual BObjectImp* call_method( const char* methodname, Executor& ex );
  virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin = false );
  virtual BObjectRef set_member( const char* membername, BObjectImp* valueimp, bool copy );
  virtual BObjectRef get_member( const char* membername );
  virtual BObjectRef get_member_id( const int id );                                   // test id
  virtual BObjectRef set_member_id( const int id, BObjectImp* valueimp, bool copy );  // test id

  virtual BObjectRef OperSubscript( const BObject& obj );
  virtual BObjectRef OperMultiSubscript( std::stack<BObjectRef>& indices );
  virtual BObjectRef OperMultiSubscriptAssign( std::stack<BObjectRef>& indices,
                                               BObjectImp* target );

  virtual bool isTrue() const;
  virtual long contains( const BObjectImp& objimp ) const;

  virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy );

  virtual BObjectImp* inverse() const;

  virtual void selfPlusPlus();
  virtual void selfMinusMinus();

  virtual ContIterator* createIterator( BObject* pIterVal );

private:
  BObjectType type_;
#if BOBJECTIMP_DEBUG
  unsigned int instance_;
  static unsigned int instances_;
  static Clib::SpinLock bobjectimp_lock;
#endif
};

inline char /*BObjectRef */ BObjectImp::member( const BObject& ) const
{
  return 0;
}
inline char /*BObjectImp* */ BObjectImp::str_member( const std::string& ) const
{
  return 0;
}


inline bool BObjectImp::isa( BObjectType type ) const
{
  return ( type == type_ );
}

inline BObjectImp::BObjectType BObjectImp::type() const
{
  return type_;
}

namespace
{
template <typename T>  // static_assert(false) is not valid, need a typedependent false
struct always_false : std::false_type
{
};
}  // namespace
// accepts const/non-const BObjectImp pointers
template <typename T, typename I>
  requires std::is_base_of_v<BObjectImp, std::remove_cv_t<std::remove_pointer_t<I>>>
T* impptrIf( I* objimp )
{
  if ( !objimp )
    return nullptr;
#define impif_test( type ) std::is_same_v<std::remove_const_t<T>, type>
#define impif_return( ot ) return objimp->isa( ot ) ? static_cast<T*>( objimp ) : nullptr
#define impif_i( ot, type )           \
  if constexpr ( impif_test( type ) ) \
  impif_return( ot )
#define impif_e( ot, type ) else if constexpr ( impif_test( type ) ) impif_return( ot )

  impif_i( BObjectImp::OTUninit, UninitObject );
  impif_e( BObjectImp::OTString, String );
  impif_e( BObjectImp::OTLong, BLong );
  impif_e( BObjectImp::OTDouble, Double );
  impif_e( BObjectImp::OTArray, ObjArray );
  impif_e( BObjectImp::OTError, BError );
  impif_e( BObjectImp::OTDictionary, BDictionary );
  impif_e( BObjectImp::OTStruct, BStruct );
  impif_e( BObjectImp::OTBoolean, BBoolean );
  impif_e( BObjectImp::OTFuncRef, BFunctionRef );
  // BClassInstance objects are never given to scripts directly, so no need for
  // inclusion here (so far...)
  impif_e( BObjectImp::OTClassInstanceRef, BClassInstanceRef );
  impif_e( BObjectImp::OTContinuation, BContinuation );
  impif_e( BObjectImp::OTSpread, BSpread );
  impif_e( BObjectImp::OTRegExp, BRegExp );
  impif_e( BObjectImp::OTSpecialUserFuncJump, BSpecialUserFuncJump );
  impif_e( BObjectImp::OTApplicObj, BApplicObjBase );
  else static_assert( always_false<T>::value, "unsupported type" );
#undef impif_i
#undef impif_e
#undef impif_return
#undef impif_test
}

}  // namespace Pol::Bscript
