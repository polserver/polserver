/*
History
=======
2006/10/28 Shinigami: GCC 4.1.1 fix - invalid use of constructor as a template
2009/09/05 Turley: Added struct .? and .- as shortcut for .exists() and .erase()

Notes
=======

*/

#ifndef __BSCRIPT_BOBJECT_H
#define __BSCRIPT_BOBJECT_H
#define __BOBJECT_H

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "clib/clib.h"
#include "clib/fixalloc.h"
#include "clib/passert.h"

#include "clib/refptr.h"
#include "escriptv.h"

class BObjectImp;
class BObject;
class BObjectRef;
class ContIterator;
class Executor;

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

class BObjectImp : public ref_counted
{
public:
    enum BObjectType 
    { 
        OTUnknown, 
        OTUninit, 
        OTString, 
        OTLong, 
        OTDouble, 
        OTArray, 
        OTApplicPtr, 
        OTApplicObj,
        OTError,
        OTDictionary,
        OTStruct,
        OTPacket,
		OTBinaryFile
    };

#if INLINE_BOBJECTIMP_CTOR
    explicit BObjectImp( BObjectType type ) : type_(type) 
    {
        ++eobject_imp_count;
        ++eobject_imp_constructions;

    }
    virtual ~BObjectImp() 
    {
        --eobject_imp_count;
    }
#else
    explicit BObjectImp( BObjectType type );
    virtual ~BObjectImp();
#endif


    // Class Machinery:
    static BObjectImp* unpack( const char* pstr );
    static BObjectImp* unpack( istream& is );
    bool isa( BObjectType type ) const;
    BObjectType type() const;
#if BOBJECTIMP_DEBUG
    unsigned int instance() const { return instance_; }
#endif
    static const char *typestr( BObjectType typ );

    virtual BObjectImp *copy() const = 0;
    virtual string getStringRep() const = 0;
    virtual string getFormattedStringRep() const;
    
    virtual unsigned long sizeEstimate() const = 0; 
    virtual const char* typeOf() const;


    virtual string pack() const;
    virtual void packonto( ostream& os ) const;
    virtual void printOn(ostream&) const;

    // Operators and stuff
    virtual bool isEqual(const BObjectImp& objimp) const;
    virtual bool isLessThan(const BObjectImp& objimp) const;

    virtual bool isLE( const BObjectImp& objimp ) const;
    virtual bool isLT( const BObjectImp& objimp ) const;
    virtual bool isGT( long val ) const;
    virtual bool isGE( long val ) const;

	// TOCONSIDER: use double dispatch rather than isa() checking
	// in operators. Requires support here, obviously.
    virtual BObjectImp* selfPlusObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfMinusObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfTimesObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfDividedByObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfModulusObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfBitShiftRightObjImp(const BObjectImp& objimp) const;
	virtual BObjectImp* selfBitShiftLeftObjImp(const BObjectImp& objimp) const;
	virtual BObjectImp* selfBitAndObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfBitXorObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfBitOrObjImp(const BObjectImp& objimp) const;
    
    virtual BObjectImp* bitnot() const;

    virtual BObjectRef operDotPlus( const char* name );
	virtual BObjectRef operDotMinus( const char* name );
	virtual BObjectRef operDotQMark( const char* name );

	virtual void operInsertInto( BObject& obj, const BObjectImp& objimp );
    
	virtual void operPlusEqual( BObject& obj, const BObjectImp& objimp );
	virtual void operMinusEqual( BObject& obj, const BObjectImp& objimp );
	virtual void operTimesEqual( BObject& obj, const BObjectImp& objimp );
	virtual void operDivideEqual( BObject& obj, const BObjectImp& objimp );
	virtual void operModulusEqual( BObject& obj, const BObjectImp& objimp );

    virtual BObject operator-() const;

    virtual char /*BObjectRef */member( const BObject& obj ) const { return 0; }
    virtual char /*BObjectImp* */ str_member( const string& membername ) const { return 0;} 
    
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex );
    virtual BObjectRef set_member( const char* membername, BObjectImp* valueimp );
    virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef get_member_id( const int id ); //test id
    virtual BObjectRef set_member_id( const int id, BObjectImp* valueimp ); //test id

    virtual BObjectRef OperSubscript( const BObject& obj );
    virtual BObjectRef OperMultiSubscript( stack<BObjectRef>& indices );
    virtual BObjectRef OperMultiSubscriptAssign( stack<BObjectRef>& indices, BObjectImp* target );

    virtual bool isTrue() const;
    virtual bool contains( const BObjectImp& objimp ) const;

    virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target );
    
    virtual BObjectImp* inverse() const;

    virtual ContIterator* createIterator( BObject* pIterVal );

    friend ostream& operator << (ostream&, const BObjectImp& );
private:
    BObjectType type_;
#if BOBJECTIMP_DEBUG
    unsigned int instance_;
    static unsigned int instances_;
#endif
};

inline bool BObjectImp::isa( BObjectType type ) const
{
    return (type == type_);
}

inline BObjectImp::BObjectType BObjectImp::type() const
{
    return type_;
}

class BObject : public ref_counted
{
public:
    explicit BObject( BObjectImp *objimp ) : ref_counted(), objimp(objimp) { passert(objimp); }
    BObject( const BObject& obj ) : ref_counted(), objimp(obj.objimp) {}
	~BObject();
     // NOTE: BObject should not be derived from!

    unsigned long sizeEstimate() const;

    void* operator new( size_t len );
    void operator delete( void * );

    bool operator!=(const BObject& obj) const;
    bool operator==(const BObject& obj) const;
    bool operator<(const BObject& obj)  const;
    bool operator<=(const BObject& obj) const;
    bool operator>(const BObject& obj) const;
    bool operator>=(const BObject& obj) const;

	BObjectImp* operator->() const { return objimp.get(); }

    bool isTrue() const { return objimp->isTrue(); }

    void assign(const BObjectImp& objimp);

    BObject* clone() const;

    bool isa( BObjectImp::BObjectType type ) const;

    friend ostream& operator << (ostream&, const BObject& );
    void printOn(ostream&) const;

    BObjectImp* impptr();
    const BObjectImp* impptr() const;
    BObjectImp& impref();
    const BObjectImp& impref() const;
    
    void setimp( BObjectImp* imp );

private:
    ref_ptr<BObjectImp> objimp;

    BObject& operator=(const BObject& obj);
};

typedef vector< ref_ptr<BObjectImp> > BObjectImpRefVec;

extern fixed_allocator<sizeof(BObject),256> bobject_alloc;

inline void* BObject::operator new( size_t len )
{
    return bobject_alloc.allocate();
}

inline void BObject::operator delete( void * p )
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
    objimp.set(imp);
}

class BObjectRef : public ref_ptr<BObject>
{
public:
    explicit BObjectRef( BObject *pobj = NULL ) : ref_ptr<BObject>(pobj) {}
    explicit BObjectRef( BObjectImp *pimp ) : ref_ptr<BObject>( new BObject(pimp) ) {}
    void set( BObject *obj ) { ref_ptr<BObject>::set( obj ); }
    unsigned long sizeEstimate() const;
};


class UninitObject : public BObjectImp
{
  public:
    UninitObject(); 
    UninitObject(const UninitObject& i);

    static UninitObject* SharedInstance;
    static ref_ptr<BObjectImp> SharedInstanceOwner;

    virtual BObjectImp *copy() const;
    virtual unsigned long sizeEstimate() const; 
    virtual string getStringRep() const { return "<uninitialized object>"; }
    virtual void printOn( ostream& os ) const;

    virtual bool isTrue() const;
    virtual bool isEqual(const BObjectImp& objimp ) const;
    virtual bool isLessThan(const BObjectImp& objimp) const;

    void* operator new( size_t len );
    void operator delete( void * );

    static UninitObject* create()
    {
        return SharedInstance;
    }
    static void ReleaseSharedInstance()
    {
        SharedInstanceOwner.clear();
        SharedInstance = NULL;
    }
};
extern fixed_allocator<sizeof(UninitObject),256> uninit_alloc;

inline void* UninitObject::operator new( size_t len )
{
    return uninit_alloc.allocate();
}

inline void UninitObject::operator delete( void * p )
{
    uninit_alloc.deallocate( p );
}
class ObjArray : public BObjectImp
{
public:
    typedef vector<string> NameCont;
    typedef NameCont::iterator name_iterator;
    typedef NameCont::const_iterator const_name_iterator;

    typedef vector<BObjectRef> Cont;
    typedef Cont::iterator iterator;
    typedef Cont::const_iterator const_iterator;
    
    NameCont name_arr;
    Cont ref_arr;

	ObjArray();
	ObjArray( const ObjArray& i ); // copy constructor 

    virtual void packonto( ostream& os ) const;
    static BObjectImp* unpack( istream& is );
    virtual unsigned long sizeEstimate() const; 
    virtual BObjectImp *copy() const;

    virtual BObjectRef operDotPlus( const char* name );
    virtual bool contains( const BObjectImp& imp ) const;
	virtual void operInsertInto( BObject& obj, const BObjectImp& objimp );
    virtual void operPlusEqual( BObject& obj, const BObjectImp& objimp );
    virtual BObjectImp* selfPlusObjImp(const BObjectImp& other) const;
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex );
    virtual BObjectRef set_member( const char* membername, BObjectImp* value );
    virtual BObjectRef get_member( const char* membername );

    void addElement( BObjectImp* imp );

    const BObjectImp* imp_at( unsigned index ) const; // 1-based

    virtual string getStringRep() const;
    virtual void printOn( ostream& os ) const;

    virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target );
    virtual BObjectRef OperSubscript( const BObject& obj );
    virtual bool isEqual(const BObjectImp& objimp ) const;

    virtual ContIterator* createIterator( BObject* pIterVal );
protected:
    explicit ObjArray( BObjectType type );
    void deepcopy();
};

class GCCHelper
{
};

class BLong : public BObjectImp
{
  public:
#if BOBJECTIMP_DEBUG
    explicit BLong(long lval = 0L);
    BLong(const BLong& L);
#else
    explicit BLong(long lval = 0L) : BObjectImp( OTLong ), lval_(lval) {}
    BLong(const BLong& L) : BObjectImp( OTLong ), lval_(L.lval_) { }
#endif

    friend class GCCHelper;

  private:
    ~BLong() {}

  public:

    void* operator new( size_t len );
    void operator delete( void * );
    void operator delete( void *, size_t );

    static BObjectImp* unpack( const char* pstr );
    static BObjectImp* unpack( istream& is );
    virtual string pack() const;
    virtual void packonto( ostream& os ) const;
    virtual unsigned long sizeEstimate() const; 

    long value() const { return lval_; }
    long increment() { return ++lval_; }

  public: // Class Machinery
    virtual BObjectImp *copy() const;
    virtual BObjectImp* inverse() const { return new BLong(-lval_); }
    void copyvalue( const BLong& ni ) { lval_ = ni.lval_; }


    virtual bool isTrue() const;
    virtual bool isEqual(const BObjectImp& objimp) const;
    virtual bool isLessThan(const BObjectImp& objimp) const;

    virtual bool isLE( const BObjectImp& objimp ) const;
    virtual bool isLT( const BObjectImp& objimp ) const;
    virtual bool isGT( long val ) const;
    virtual bool isGE( long val ) const;

    virtual BObjectImp* selfPlusObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfMinusObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfTimesObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfDividedByObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfModulusObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfBitShiftRightObjImp(const BObjectImp& objimp) const;
	virtual BObjectImp* selfBitShiftLeftObjImp(const BObjectImp& objimp) const;
	virtual BObjectImp* selfBitAndObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfBitXorObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfBitOrObjImp(const BObjectImp& objimp) const;

    virtual BObjectImp* bitnot() const;

    virtual string getStringRep() const;
    virtual void printOn(ostream&) const;

  protected:
    long lval_;
};

extern fixed_allocator<sizeof(BLong),256> blong_alloc;
inline void* BLong::operator new( size_t len )
{
    passert_paranoid( len == sizeof(BLong) );
    return blong_alloc.allocate();
}
inline void BLong::operator delete( void * p )
{
    blong_alloc.deallocate( p );
}
inline void BLong::operator delete( void * p, size_t len )
{
    blong_alloc.deallocate( p );
}

class Double : public BObjectImp
{
  public:
    explicit Double(double dval = 0.0) : BObjectImp( OTDouble ), dval_(dval) { }
    Double(const Double& dbl) : BObjectImp( OTDouble ), dval_(dbl.dval_) { }

    friend class GCCHelper;

		// FIXME: 2008 Upgrades needed here? Still valid, or why the hell we even do this anyway???
#if defined(__GNUC__) || (defined(_WIN32) && _MSC_VER >= 1300)
  protected:
#else
  private:
#endif
     ~Double() {}

  public:
    void* operator new( size_t len );
    void operator delete( void * );


    static BObjectImp* unpack( const char* pstr );
    static BObjectImp* unpack( istream& is );
    virtual string pack() const;
    virtual void packonto( ostream& os ) const;
    virtual unsigned long sizeEstimate() const; 

    double value() const { return dval_; }
    void copyvalue( const Double& dbl ) { dval_ = dbl.dval_; }
    double increment() { return ++dval_; }

  public:    // Class Machinery
    virtual bool isTrue() const { return (dval_ != 0.0); }

    virtual BObjectImp *copy() const
      {
          return new Double(*this);
      }
    virtual bool isEqual(const BObjectImp& objimp) const;
    virtual bool isLessThan(const BObjectImp& objimp) const;
    virtual bool isGT( long val ) const;
    virtual bool isGE( long val ) const;

    virtual BObjectImp* selfPlusObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfMinusObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfTimesObjImp(const BObjectImp& objimp) const;
    virtual BObjectImp* selfDividedByObjImp(const BObjectImp& objimp) const;

    virtual void unary_minus() { dval_ = - dval_; }
    virtual BObjectImp* inverse() const { return new Double(-dval_); }

    virtual string getStringRep() const;
    virtual void printOn(ostream&) const;
  private:
    double dval_;
};

extern fixed_allocator<sizeof(Double),256> double_alloc;
inline void* Double::operator new( size_t len )
{
    passert_paranoid( len == sizeof(Double) );
    return double_alloc.allocate();
}
inline void Double::operator delete( void * p )
{
    double_alloc.deallocate( p );
}

class BApplicObjType
{
};

/*
	Application Pointer object.  Meant to point to some application data.
	The script can't modify the value of the pointer, so the application can
	hand this off to the script and trust that when it gets it back, it's
	what was passed in.  pointer_type should be, typically, a class name.
*/
class BApplicPtr : public BObjectImp
{
  public:
    BApplicPtr( const BApplicObjType* pointer_type, void *ptr );

    const BApplicObjType* pointer_type() const;
    void *ptr() const; 
  
  public: // Class Machinery
    virtual BObjectImp *copy() const;

    virtual string getStringRep() const;
    virtual void printOn(ostream&) const;
    virtual unsigned long sizeEstimate() const; 

  private:
    void *ptr_;
    const BApplicObjType* pointer_type_;
};


class BApplicObjBase : public BObjectImp
{
public:
    explicit BApplicObjBase( const BApplicObjType* object_type );

    const BApplicObjType* object_type() const;
public: // Class Machinery
    virtual BObjectImp* copy() const = 0;

    virtual string getStringRep() const;
    virtual void printOn(ostream&) const;
    virtual unsigned long sizeEstimate() const = 0; 

private:
    const BApplicObjType* object_type_;
};

inline BApplicObjBase::BApplicObjBase( const BApplicObjType* object_type ) :
    BObjectImp( OTApplicObj ),
    object_type_( object_type )
{
}

inline const BApplicObjType* BApplicObjBase::object_type() const
{
    return object_type_;
}


template<class T>
class BApplicObj : public BApplicObjBase
{
public:
    explicit BApplicObj( const BApplicObjType* object_type );
    BApplicObj( const BApplicObjType*, const T& );
    
    T& value();
    const T& value() const;
    T* operator->();    

    virtual const char* typeOf() const = 0;
    virtual BObjectImp* copy() const = 0;
    virtual unsigned long sizeEstimate() const; 

protected:
    T obj_;
};

template<class T>
BApplicObj<T>::BApplicObj( const BApplicObjType* object_type ) :
    BApplicObjBase( object_type )
{
}

template<class T>
BApplicObj<T>::BApplicObj( const BApplicObjType* object_type, const T& obj ) :
    BApplicObjBase( object_type ),
    obj_(obj)
{
}

template<class T>
unsigned long BApplicObj<T>::sizeEstimate() const
{
    return sizeof(BApplicObj<T>);
}

template<class T>
T& BApplicObj<T>::value()
{
    return obj_;
}

template<class T>
const T& BApplicObj<T>::value() const
{
    return obj_;
}

template<class T>
T* BApplicObj<T>::operator->()
{
    return &obj_;
}

#endif
