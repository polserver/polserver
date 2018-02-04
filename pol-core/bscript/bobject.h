/** @file
 *
 * @par History
 * - 2006/10/28 Shinigami: GCC 4.1.1 fix - invalid use of constructor as a template
 * - 2009/09/05 Turley:    Added struct .? and .- as shortcut for .exists() and .erase()
 * - 2009/12/21 Turley:    ._method() call fix
 */

#ifndef BSCRIPT_BOBJECT_H
#define BSCRIPT_BOBJECT_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

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

#include <format/format.h>

#include "../clib/fixalloc.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../clib/refptr.h"
#include "../clib/spinlock.h"

#if INLINE_BOBJECTIMP_CTOR
#include "escriptv.h"
#endif

#include <iosfwd>
#include <stack>
#include <vector>

namespace Pol
{
/**
 * Bscript namespace is for escript stuff, like compiler and basic modules
 */
namespace Bscript
{
class BObjectImp;
class BObject;
class BObjectRef;
class ContIterator;
class Executor;
class Instruction;

class BLong;
class Double;
class String;
class ObjArray;

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
    OTApplicPtr = 6,
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
  };

#if INLINE_BOBJECTIMP_CTOR
  explicit BObjectImp( BObjectType type ) : type_( type )
  {
    ++eobject_imp_count;
    ++eobject_imp_constructions;
  }
  virtual ~BObjectImp() { --eobject_imp_count; }
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
  virtual void packonto( std::ostream& os ) const;
  virtual void printOn( std::ostream& ) const;

  virtual bool operator==( const BObjectImp& objimp ) const;
  virtual bool operator<( const BObjectImp& objimp ) const;
  virtual bool operator<=( const BObjectImp& objimp ) const;
  virtual bool operator>( const BObjectImp& objimp ) const;
  virtual bool operator>=( const BObjectImp& objimp ) const;
  virtual bool operator!=( const BObjectImp& objimp ) const;

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

  virtual ContIterator* createIterator( BObject* pIterVal );

  friend std::ostream& operator<<( std::ostream&, const BObjectImp& );
  friend fmt::Writer& operator<<( fmt::Writer&, const BObjectImp& );

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

class BObject : public ref_counted
{
public:
  explicit BObject( BObjectImp* objimp ) : ref_counted(), objimp( objimp ) { passert( objimp ); }
  BObject( const BObject& obj ) : ref_counted(), objimp( obj.objimp ) {}
  ~BObject();
  // NOTE: BObject should not be derived from!

  size_t sizeEstimate() const;

  void* operator new( std::size_t len );
  void operator delete( void* );

  bool operator!=( const BObject& obj ) const;
  bool operator==( const BObject& obj ) const;
  bool operator<( const BObject& obj ) const;
  bool operator<=( const BObject& obj ) const;
  bool operator>( const BObject& obj ) const;
  bool operator>=( const BObject& obj ) const;

  BObjectImp* operator->() const { return objimp.get(); }
  bool isTrue() const { return objimp->isTrue(); }
  void assign( const BObjectImp& objimp );

  BObject* clone() const;

  bool isa( BObjectImp::BObjectType type ) const;

  //   friend StreamWriter& operator << (StreamWriter&, const BObject& );
  friend std::ostream& operator<<( std::ostream&, const BObject& );
  friend fmt::Writer& operator<<( fmt::Writer&, const BObject& );
  void printOn( std::ostream& ) const;

  BObjectImp* impptr();
  const BObjectImp* impptr() const;
  BObjectImp& impref();
  const BObjectImp& impref() const;

  void setimp( BObjectImp* imp );

private:
  ref_ptr<BObjectImp> objimp;

  BObject& operator=( const BObject& obj );
};

typedef std::vector<ref_ptr<BObjectImp>> BObjectImpRefVec;

extern Clib::fixed_allocator<sizeof( BObject ), 256> bobject_alloc;

inline void* BObject::operator new( std::size_t /*len*/ )
{
  return bobject_alloc.allocate();
}

inline void BObject::operator delete( void* p )
{
  bobject_alloc.deallocate( p );
}

inline bool BObject::isa( BObjectImp::BObjectType type ) const
{
  return objimp->isa( type );
}

inline BObjectImp* BObject::impptr()
{
  return objimp.get();
}

inline const BObjectImp* BObject::impptr() const
{
  return objimp.get();
}

inline BObjectImp& BObject::impref()
{
  return *objimp;
}
inline const BObjectImp& BObject::impref() const
{
  return *objimp;
}
inline void BObject::setimp( BObjectImp* imp )
{
  objimp.set( imp );
}

class BObjectRef : public ref_ptr<BObject>
{
public:
  explicit BObjectRef( BObject* pobj = NULL ) : ref_ptr<BObject>( pobj ) {}
  explicit BObjectRef( BObjectImp* pimp ) : ref_ptr<BObject>( new BObject( pimp ) ) {}
  void set( BObject* obj ) { ref_ptr<BObject>::set( obj ); }
  size_t sizeEstimate() const;
};


class UninitObject : public BObjectImp
{
public:
  UninitObject();
  UninitObject( const UninitObject& i );

  static UninitObject* SharedInstance;
  static ref_ptr<BObjectImp> SharedInstanceOwner;

  virtual BObjectImp* copy() const POL_OVERRIDE;
  virtual size_t sizeEstimate() const POL_OVERRIDE;
  virtual std::string getStringRep() const POL_OVERRIDE { return "<uninitialized object>"; }
  virtual void printOn( std::ostream& os ) const POL_OVERRIDE;

  virtual bool isTrue() const POL_OVERRIDE;
  virtual bool operator==( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual bool operator<( const BObjectImp& objimp ) const POL_OVERRIDE;

  void* operator new( std::size_t len );
  void operator delete( void* );

  static UninitObject* create() { return SharedInstance; }
  static void ReleaseSharedInstance()
  {
    SharedInstanceOwner.clear();
    SharedInstance = NULL;
  }
};
extern Clib::fixed_allocator<sizeof( UninitObject ), 256> uninit_alloc;

inline void* UninitObject::operator new( std::size_t /*len*/ )
{
  return uninit_alloc.allocate();
}

inline void UninitObject::operator delete( void* p )
{
  uninit_alloc.deallocate( p );
}
class ObjArray : public BObjectImp
{
public:
  typedef std::vector<std::string> NameCont;
  typedef NameCont::iterator name_iterator;
  typedef NameCont::const_iterator const_name_iterator;

  typedef std::vector<BObjectRef> Cont;
  typedef Cont::iterator iterator;
  typedef Cont::const_iterator const_iterator;

  NameCont name_arr;
  Cont ref_arr;

  ObjArray();
  ObjArray( const ObjArray& i );  // copy constructor

  virtual void packonto( std::ostream& os ) const POL_OVERRIDE;
  static BObjectImp* unpack( std::istream& is );
  virtual size_t sizeEstimate() const POL_OVERRIDE;
  virtual BObjectImp* copy() const POL_OVERRIDE;

  virtual BObjectRef operDotPlus( const char* name ) POL_OVERRIDE;
  virtual long contains( const BObjectImp& imp ) const POL_OVERRIDE;
  virtual void operInsertInto( BObject& obj, const BObjectImp& objimp ) POL_OVERRIDE;
  virtual BObjectImp* selfPlusObjImp( const BObjectImp& other ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const Double& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const String& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const ObjArray& objimp ) const POL_OVERRIDE;
  virtual void selfPlusObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( Double& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( String& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( ObjArray& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) POL_OVERRIDE;
  virtual BObjectImp* call_method_id( const int id, Executor& ex,
                                      bool forcebuiltin = false ) POL_OVERRIDE;
  virtual BObjectRef set_member( const char* membername, BObjectImp* value,
                                 bool copy ) POL_OVERRIDE;
  virtual BObjectRef get_member( const char* membername ) POL_OVERRIDE;

  void addElement( BObjectImp* imp );

  const BObjectImp* imp_at( unsigned index ) const;  // 1-based

  virtual std::string getStringRep() const POL_OVERRIDE;
  virtual void printOn( std::ostream& os ) const POL_OVERRIDE;

  virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy ) POL_OVERRIDE;
  virtual BObjectRef OperSubscript( const BObject& obj ) POL_OVERRIDE;
  virtual BObjectRef OperMultiSubscript( std::stack<BObjectRef>& indices ) POL_OVERRIDE;

  virtual bool operator==( const BObjectImp& objimp ) const POL_OVERRIDE;

  virtual ContIterator* createIterator( BObject* pIterVal ) POL_OVERRIDE;

protected:
  explicit ObjArray( BObjectType type );
  void deepcopy();
};

class BLong : public BObjectImp
{
  typedef BObjectImp base;

public:
#if BOBJECTIMP_DEBUG
  explicit BLong( int lval = 0L );
  BLong( const BLong& L );
#else
  explicit BLong( int lval = 0L ) : BObjectImp( OTLong ), lval_( static_cast<int>( lval ) ) {}
  BLong( const BLong& L ) : BObjectImp( OTLong ), lval_( L.lval_ ) {}
#endif
private:
  ~BLong() {}

public:
  void* operator new( std::size_t len );
  void operator delete( void* );
  void operator delete( void*, size_t );

  static BObjectImp* unpack( const char* pstr );
  static BObjectImp* unpack( std::istream& is );
  virtual std::string pack() const POL_OVERRIDE;
  static std::string pack( int val );
  virtual void packonto( std::ostream& os ) const POL_OVERRIDE;
  virtual size_t sizeEstimate() const POL_OVERRIDE;

  int value() const { return lval_; }
  int increment() { return ++lval_; }

public:  // Class Machinery
  virtual BObjectImp* copy() const POL_OVERRIDE;
  virtual BObjectImp* inverse() const POL_OVERRIDE { return new BLong( -lval_ ); }
  void copyvalue( const BLong& ni ) { lval_ = ni.lval_; }
  virtual bool isTrue() const POL_OVERRIDE;
  virtual bool operator==( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual bool operator<( const BObjectImp& objimp ) const POL_OVERRIDE;

  virtual BObjectImp* selfPlusObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const Double& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const String& objimp ) const POL_OVERRIDE;
  virtual void selfPlusObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( Double& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( String& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfMinusObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfMinusObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfMinusObj( const Double& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfMinusObj( const String& objimp ) const POL_OVERRIDE;
  virtual void selfMinusObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfMinusObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfMinusObj( Double& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfMinusObj( String& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfTimesObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfTimesObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfTimesObj( const Double& objimp ) const POL_OVERRIDE;
  virtual void selfTimesObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfTimesObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfTimesObj( Double& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfDividedByObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfDividedByObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfDividedByObj( const Double& objimp ) const POL_OVERRIDE;
  virtual void selfDividedByObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfDividedByObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfDividedByObj( Double& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfModulusObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfModulusObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfModulusObj( const Double& objimp ) const POL_OVERRIDE;
  virtual void selfModulusObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfModulusObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfModulusObj( Double& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfBitShiftRightObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfBitShiftRightObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual void selfBitShiftRightObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfBitShiftRightObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfBitShiftLeftObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfBitShiftLeftObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual void selfBitShiftLeftObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfBitShiftLeftObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfBitAndObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfBitAndObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual void selfBitAndObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfBitAndObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfBitXorObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfBitXorObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual void selfBitXorObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfBitXorObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfBitOrObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfBitOrObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual void selfBitOrObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfBitOrObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* bitnot() const POL_OVERRIDE;

  virtual std::string getStringRep() const POL_OVERRIDE;
  virtual void printOn( std::ostream& ) const POL_OVERRIDE;

protected:
  int lval_;
};

extern Clib::fixed_allocator<sizeof( BLong ), 256> blong_alloc;
inline void* BLong::operator new( std::size_t len )
{
  (void)len;
  passert_paranoid( len == sizeof( BLong ) );
  return blong_alloc.allocate();
}
inline void BLong::operator delete( void* p )
{
  blong_alloc.deallocate( p );
}
inline void BLong::operator delete( void* p, size_t /*len*/ )
{
  blong_alloc.deallocate( p );
}

class Double : public BObjectImp
{
  typedef BObjectImp base;

public:
  explicit Double( double dval = 0.0 ) : BObjectImp( OTDouble ), dval_( dval ) {}
  Double( const Double& dbl ) : BObjectImp( OTDouble ), dval_( dbl.dval_ ) {}

protected:
  ~Double() {}

public:
  void* operator new( std::size_t len );
  void operator delete( void* );

  static BObjectImp* unpack( const char* pstr );
  static BObjectImp* unpack( std::istream& is );
  virtual std::string pack() const POL_OVERRIDE;
  virtual void packonto( std::ostream& os ) const POL_OVERRIDE;
  virtual size_t sizeEstimate() const POL_OVERRIDE;

  double value() const { return dval_; }
  void copyvalue( const Double& dbl ) { dval_ = dbl.dval_; }
  double increment() { return ++dval_; }

public:  // Class Machinery
  virtual bool isTrue() const POL_OVERRIDE { return ( dval_ != 0.0 ); }
  virtual BObjectImp* copy() const POL_OVERRIDE { return new Double( *this ); }
  virtual bool operator==( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual bool operator<( const BObjectImp& objimp ) const POL_OVERRIDE;

  virtual BObjectImp* selfPlusObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const Double& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfPlusObj( const String& objimp ) const POL_OVERRIDE;
  virtual void selfPlusObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( Double& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfPlusObj( String& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfMinusObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfMinusObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfMinusObj( const Double& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfMinusObj( const String& objimp ) const POL_OVERRIDE;
  virtual void selfMinusObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfMinusObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfMinusObj( Double& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfMinusObj( String& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfTimesObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfTimesObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfTimesObj( const Double& objimp ) const POL_OVERRIDE;
  virtual void selfTimesObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfTimesObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfTimesObj( Double& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* selfDividedByObjImp( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfDividedByObj( const BLong& objimp ) const POL_OVERRIDE;
  virtual BObjectImp* selfDividedByObj( const Double& objimp ) const POL_OVERRIDE;
  virtual void selfDividedByObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfDividedByObj( BLong& objimp, BObject& obj ) POL_OVERRIDE;
  virtual void selfDividedByObj( Double& objimp, BObject& obj ) POL_OVERRIDE;

  virtual BObjectImp* inverse() const POL_OVERRIDE { return new Double( -dval_ ); }
  virtual std::string getStringRep() const POL_OVERRIDE;
  virtual void printOn( std::ostream& ) const POL_OVERRIDE;

private:
  double dval_;
};

extern Clib::fixed_allocator<sizeof( Double ), 256> double_alloc;
inline void* Double::operator new( std::size_t len )
{
  (void)len;
  passert_paranoid( len == sizeof( Double ) );
  return double_alloc.allocate();
}
inline void Double::operator delete( void* p )
{
  double_alloc.deallocate( p );
}

class BBoolean : public BObjectImp
{
  typedef BObjectImp base;

public:
#if BOBJECTIMP_DEBUG
  explicit BBoolean( bool bval = false );
  BBoolean( const BBoolean& B );
#else
  explicit BBoolean( bool bval = false ) : BObjectImp( OTBoolean ), bval_( bval ) {}
  BBoolean( const BBoolean& B ) : BBoolean( B.bval_ ) {}
#endif
private:
  ~BBoolean() {}

public:
  static BObjectImp* unpack( std::istream& is );
  virtual std::string pack() const POL_OVERRIDE;
  virtual void packonto( std::ostream& os ) const POL_OVERRIDE;
  virtual size_t sizeEstimate() const POL_OVERRIDE;

  bool value() const { return bval_; }

public:  // Class Machinery
  virtual BObjectImp* copy() const POL_OVERRIDE;
  virtual bool isTrue() const POL_OVERRIDE;
  virtual bool operator==( const BObjectImp& objimp ) const POL_OVERRIDE;

  virtual std::string getStringRep() const POL_OVERRIDE;
  virtual void printOn( std::ostream& ) const POL_OVERRIDE;

private:
  bool bval_;
};

class BFunctionRef : public BObjectImp
{
  typedef BObjectImp base;

public:
  explicit BFunctionRef( int progcounter, int param_count );
  BFunctionRef( const BFunctionRef& B );

private:
  ~BFunctionRef() {}

public:
  virtual size_t sizeEstimate() const POL_OVERRIDE;
  bool validCall( const int id, Executor& ex, Instruction* inst ) const;
  bool validCall( const char* methodname, Executor& ex, Instruction* inst ) const;

public:  // Class Machinery
  virtual BObjectImp* copy() const POL_OVERRIDE;
  virtual bool isTrue() const POL_OVERRIDE;
  virtual bool operator==( const BObjectImp& objimp ) const POL_OVERRIDE;

  virtual std::string getStringRep() const POL_OVERRIDE;

  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) POL_OVERRIDE;
  virtual BObjectImp* call_method_id( const int id, Executor& ex,
                                      bool forcebuiltin = false ) POL_OVERRIDE;

private:
  unsigned int pc_;
  int num_params_;
};
class BApplicObjType
{
};

/**
 * Application Pointer object.  Meant to point to some application data.
 *
 * The script can't modify the value of the pointer, so the application can
 * hand this off to the script and trust that when it gets it back, it's
 * what was passed in.  pointer_type should be, typically, a class name.
 */
class BApplicPtr : public BObjectImp
{
public:
  BApplicPtr( const BApplicObjType* pointer_type, void* ptr );

  const BApplicObjType* pointer_type() const;
  void* ptr() const;

public:  // Class Machinery
  virtual BObjectImp* copy() const POL_OVERRIDE;

  virtual std::string getStringRep() const POL_OVERRIDE;
  virtual void printOn( std::ostream& ) const POL_OVERRIDE;
  virtual size_t sizeEstimate() const POL_OVERRIDE;

private:
  void* ptr_;
  const BApplicObjType* pointer_type_;
};


class BApplicObjBase : public BObjectImp
{
public:
  explicit BApplicObjBase( const BApplicObjType* object_type );

  const BApplicObjType* object_type() const;

public:  // Class Machinery
  virtual BObjectImp* copy() const POL_OVERRIDE = 0;

  virtual std::string getStringRep() const POL_OVERRIDE;
  virtual void printOn( std::ostream& ) const POL_OVERRIDE;
  virtual size_t sizeEstimate() const POL_OVERRIDE = 0;

private:
  const BApplicObjType* object_type_;
};

inline BApplicObjBase::BApplicObjBase( const BApplicObjType* object_type )
    : BObjectImp( OTApplicObj ), object_type_( object_type )
{
}

inline const BApplicObjType* BApplicObjBase::object_type() const
{
  return object_type_;
}


template <class T>
class BApplicObj : public BApplicObjBase
{
public:
  explicit BApplicObj( const BApplicObjType* object_type );
  BApplicObj( const BApplicObjType*, const T& );

  T& value();
  const T& value() const;
  T* operator->();

  virtual const char* typeOf() const POL_OVERRIDE = 0;
  virtual u8 typeOfInt() const POL_OVERRIDE = 0;
  virtual BObjectImp* copy() const POL_OVERRIDE = 0;
  virtual size_t sizeEstimate() const POL_OVERRIDE;

protected:
  T obj_;
};

template <class T>
BApplicObj<T>::BApplicObj( const BApplicObjType* object_type ) : BApplicObjBase( object_type )
{
}

template <class T>
BApplicObj<T>::BApplicObj( const BApplicObjType* object_type, const T& obj )
    : BApplicObjBase( object_type ), obj_( obj )
{
}

template <class T>
inline size_t BApplicObj<T>::sizeEstimate() const
{
  return sizeof( BApplicObj<T> );
}

template <class T>
T& BApplicObj<T>::value()
{
  return obj_;
}

template <class T>
const T& BApplicObj<T>::value() const
{
  return obj_;
}

template <class T>
T* BApplicObj<T>::operator->()
{
  return &obj_;
}
}
}
#endif
