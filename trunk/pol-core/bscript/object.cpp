/*
History
=======
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2008/02/11 Turley:    ObjArray::unpack() will accept zero length Arrays and Erros from Array-Elements
2009/09/05 Turley:    Added struct .? and .- as shortcut for .exists() and .erase()
2009/12/21 Turley:    ._method() call fix

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/clib.h"
#include "../clib/stlutil.h"
#include "../clib/random.h"

#include "berror.h"
#include "bobject.h"
#include "bstruct.h"
#include "dict.h"
#include "impstr.h"
#include "escriptv.h"
#include "executor.h"
#include "objmembers.h"
#include "objmethods.h"

fixed_allocator<sizeof(BObject),256> bobject_alloc;
fixed_allocator<sizeof(UninitObject),256> uninit_alloc;
fixed_allocator<sizeof(BLong),256> blong_alloc;
fixed_allocator<sizeof(Double),256> double_alloc;

size_t BObjectRef::sizeEstimate() const
{
	if (get())
		return sizeof(BObjectRef) + get()->sizeEstimate();
	else
		return sizeof(BObjectRef);
}
size_t BObject::sizeEstimate() const
{
	if (objimp.get())
		return sizeof(BObject) + objimp.get()->sizeEstimate();
	else
		return sizeof(BObject);
}


/*
  pack formats:
	 sSTRING\0		  string
	 iINTEGER\0		 integer
	 rREAL\0			real
	 u\0				uninitialized
	 aNN:ELEMS		  array
	 SNN:STRING		 

  Examples:
		57			  i57
		4.3			 r4.3
		"hello world"   shello world
		{ 5,3 }		 a2:i5i3
		{ 5, "hey" }	a2:i5S3:hey
		{ 5, "hey", 7 } a3:i5S3:heyi7
*/

BObjectImp* BObjectImp::unpack( istream& is )
{
	char typech;
	if (is >> typech)
	{
		switch( typech )
		{
			case 's': return String::unpack( is );
			case 'S': return String::unpackWithLen( is );
			case 'i': return BLong::unpack( is );
			case 'r': return Double::unpack( is );
			case 'u': return UninitObject::create();
			case 'a': return ObjArray::unpack( is );
			case 'd': return BDictionary::unpack( is );
			case 't': return BStruct::unpack( is );
			case 'e': return BError::unpack( is );
			case 'x': return UninitObject::create();

			default:
				return new BError( "Unknown object type '" + string(1,typech) + "'" );
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

BObject::~BObject()
{
}



BObject *BObject::clone() const
{
	return new BObject( objimp->copy() );
}

bool BObject::operator!=(const BObject& obj) const 
{ 
	return ! (objimp->isEqual( obj.impref() ));   
}
bool BObject::operator==(const BObject& obj) const 
{ 
	return objimp->isEqual( obj.impref() );	
}
bool BObject::operator<(const BObject& obj) const
{ 
	return objimp->isLessThan(*obj.objimp); 
}
bool BObject::operator<=(const BObject& obj) const
{
	return objimp->isLessThan(*obj.objimp) ||
		   objimp->isEqual(*obj.objimp);
}
bool BObject::operator>(const BObject& obj) const
{
	return !objimp->isLessThan(*obj.objimp) &&
		   !objimp->isEqual(*obj.objimp);
}
bool BObject::operator>=(const BObject& obj) const
{
	return !objimp->isLessThan(*obj.objimp);
}

////////////////////// BObjectImp //////////////////////
#if BOBJECTIMP_DEBUG
#include "../clib/hbmutex.h"
# if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3 )
typedef unordered_map<unsigned int, BObjectImp*> bobjectimps;
# else
typedef hash_map<unsigned int, BObjectImp*> bobjectimps;
# endif

bobjectimps bobjectimp_instances;
int display_bobjectimp_instance( BObjectImp* imp )
{
	cout << imp->instance() << ": " << imp->getStringRep() << endl;
	return 0;
}
void display_bobjectimp_instances()
{
	cout << "bobjectimp instances: " << bobjectimp_instances.size() << endl;
	for( bobjectimps::iterator itr = bobjectimp_instances.begin(); itr != bobjectimp_instances.end(); ++itr )
	{
		display_bobjectimp_instance( (*itr).second );
	}
}
#endif

#if !INLINE_BOBJECTIMP_CTOR
unsigned int BObjectImp::instances_ = 0;
Mutex BObjectImp::bobjectimp_mutex;
BObjectImp::BObjectImp( BObjectType type ) :
	type_(type),
	instance_(0)
{
	LocalMutex guard(&bobjectimp_mutex);
	instance_=instances_++;
	++eobject_imp_count;
	++eobject_imp_constructions;
	bobjectimp_instances[ instance_ ] = this;
}

BObjectImp::~BObjectImp()
{
	LocalMutex guard(&bobjectimp_mutex);
	bobjectimp_instances.erase( instance_ );
	--eobject_imp_count;
}
#endif

string BObjectImp::pack() const
{
	OSTRINGSTREAM os;
	packonto( os );
	return OSTRINGSTREAM_STR(os);
}

void BObjectImp::packonto( ostream& os ) const
{
	os << "u";
}

string BObjectImp::getFormattedStringRep() const
{
	return getStringRep();
}

const char *BObjectImp::typestr( BObjectType typ )
{
	switch( typ )
	{
		case OTUnknown:	 return "Unknown";
		case OTUninit:	  return "Uninit";
		case OTString:	  return "String";
		case OTLong:		return "Integer";
		case OTDouble:	  return "Double";
		case OTArray:	   return "Array";
		case OTApplicPtr:   return "ApplicPtr";
		case OTApplicObj:   return "ApplicObj";
		case OTError:	   return "Error";
		case OTDictionary:	return "Dictionary";
		case OTStruct:		return "Struct";
		case OTPacket:		return "Packet";
		case OTBinaryFile:  return "BinaryFile";
		default:			return "Undefined";
	}
}

const char* BObjectImp::typeOf() const
{
	return typestr( type_ );
}


int BObjectImp::typeOfInt() const
{
	return type_;
}

// These two functions are just here for completeness.
bool BObjectImp::isEqual(const BObjectImp& objimp) const
{
  return (this == &objimp);
}
bool BObjectImp::isLessThan(const BObjectImp& objimp) const
{
  return (this < &objimp);
}

bool BObjectImp::isLE( const BObjectImp& objimp ) const
{
	return isEqual(objimp) || isLessThan(objimp);
}
bool BObjectImp::isLT( const BObjectImp& objimp ) const
{
	return isLessThan(objimp);
}

bool BObjectImp::isGT( int val ) const
{
	return false;
}
bool BObjectImp::isGE( int val ) const
{
	return false;
}

BObjectImp* BObjectImp::array_assign( BObjectImp* idx, BObjectImp* target )
{
	return this;
}

BObjectRef BObjectImp::OperMultiSubscript( stack<BObjectRef>& indices )
{
	BObjectRef index = indices.top();
	indices.pop();
	BObjectRef ref = OperSubscript( *index );
	if (indices.empty())
		return ref;
	else
		return (*ref).impptr()->OperMultiSubscript( indices );
}

BObjectRef BObjectImp::OperMultiSubscriptAssign( stack<BObjectRef>& indices, BObjectImp* target )
{
	BObjectRef index = indices.top();
	indices.pop();
	if (indices.empty())
	{
		BObjectImp* imp = array_assign( (*index).impptr(), target );
		return BObjectRef( imp );
	}
	else
	{
		BObjectRef ref = OperSubscript( *index );
		return (*ref).impptr()->OperMultiSubscript( indices );
	}
}

BObjectImp* BObjectImp::selfPlusObjImp(const BObjectImp&  objimp ) const
{
	return objimp.selfPlusObj(*this);
}
BObjectImp* BObjectImp::selfPlusObj(const BObjectImp& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfPlusObj(const BLong& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfPlusObj(const Double& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfPlusObj(const String& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfPlusObj(const ObjArray& objimp) const
{
	return copy();
}
void BObjectImp::selfPlusObjImp(BObjectImp& objimp, BObject& obj)
{
	objimp.selfPlusObj(*this, obj);
}
void BObjectImp::selfPlusObj(BObjectImp& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfPlusObj(BLong& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfPlusObj(Double& objimp, BObject& obj)
{
	//obj.setimp( selfPlusObj(objimp) );
}
void BObjectImp::selfPlusObj(String& objimp, BObject& obj)
{
	//obj.setimp( selfPlusObj(objimp) );
}
void BObjectImp::selfPlusObj(ObjArray& objimp, BObject& obj)
{
	//obj.setimp( selfPlusObj(objimp) );
}

BObjectImp* BObjectImp::selfMinusObjImp(const BObjectImp& objimp) const
{
	return objimp.selfMinusObj(*this);
}
BObjectImp* BObjectImp::selfMinusObj(const BObjectImp& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfMinusObj(const BLong& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfMinusObj(const Double& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfMinusObj(const String& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfMinusObj(const ObjArray& objimp) const
{
	return copy();
}
void BObjectImp::selfMinusObjImp(BObjectImp& objimp, BObject& obj)
{
	objimp.selfMinusObjImp(*this, obj);
}
void BObjectImp::selfMinusObj(BObjectImp& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfMinusObj(BLong& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfMinusObj(Double& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfMinusObj(String& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfMinusObj(ObjArray& objimp, BObject& obj)
{
	//
}

BObjectImp* BObjectImp::selfTimesObjImp(const BObjectImp& objimp) const
{
	return objimp.selfTimesObj(*this);
}
BObjectImp* BObjectImp::selfTimesObj(const BObjectImp& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfTimesObj(const BLong& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfTimesObj(const Double& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfTimesObj(const String& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfTimesObj(const ObjArray& objimp) const
{
	return copy();
}
void BObjectImp::selfTimesObjImp(BObjectImp& objimp, BObject& obj)
{
	objimp.selfTimesObjImp(*this, obj);
}
void BObjectImp::selfTimesObj(BObjectImp& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfTimesObj(BLong& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfTimesObj(Double& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfTimesObj(String& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfTimesObj(ObjArray& objimp, BObject& obj)
{
	//
}

BObjectImp* BObjectImp::selfDividedByObjImp(const BObjectImp& objimp) const
{
	return objimp.selfDividedByObj(*this);
}
BObjectImp* BObjectImp::selfDividedByObj(const BObjectImp& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfDividedByObj(const BLong& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfDividedByObj(const Double& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfDividedByObj(const String& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfDividedByObj(const ObjArray& objimp) const
{
	return copy();
}
void BObjectImp::selfDividedByObjImp(BObjectImp& objimp, BObject& obj)
{
	objimp.selfDividedByObj(*this, obj);
}
void BObjectImp::selfDividedByObj(BObjectImp& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfDividedByObj(BLong& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfDividedByObj(Double& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfDividedByObj(String& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfDividedByObj(ObjArray& objimp, BObject& obj)
{
	//
}

BObjectImp* BObjectImp::selfModulusObjImp(const BObjectImp& objimp) const
{
	return objimp.selfModulusObj(*this);
}
BObjectImp* BObjectImp::selfModulusObj(const BObjectImp& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfModulusObj(const BLong& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfModulusObj(const Double& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfModulusObj(const String& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfModulusObj(const ObjArray& objimp) const
{
	return copy();
}
void BObjectImp::selfModulusObjImp(BObjectImp& objimp, BObject& obj)
{
	objimp.selfModulusObj(*this, obj);
}
void BObjectImp::selfModulusObj(BObjectImp& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfModulusObj(BLong& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfModulusObj(Double& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfModulusObj(String& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfModulusObj(ObjArray& objimp, BObject& obj)
{
	//
}

BObjectImp* BObjectImp::selfBitShiftRightObjImp(const BObjectImp& objimp) const
{
	return objimp.selfBitShiftRightObj(*this);
}
BObjectImp* BObjectImp::selfBitShiftRightObj(const BObjectImp& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitShiftRightObj(const BLong& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitShiftRightObj(const Double& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitShiftRightObj(const String& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitShiftRightObj(const ObjArray& objimp) const
{
	return copy();
}
void BObjectImp::selfBitShiftRightObjImp(BObjectImp& objimp, BObject& obj)
{
	objimp.selfBitShiftRightObj(*this, obj);
}
void BObjectImp::selfBitShiftRightObj(BObjectImp& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitShiftRightObj(BLong& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitShiftRightObj(Double& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitShiftRightObj(String& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitShiftRightObj(ObjArray& objimp, BObject& obj)
{
	//
}

BObjectImp* BObjectImp::selfBitShiftLeftObjImp(const BObjectImp& objimp) const
{
	return objimp.selfBitShiftLeftObj(*this);
}
BObjectImp* BObjectImp::selfBitShiftLeftObj(const BObjectImp& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitShiftLeftObj(const BLong& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitShiftLeftObj(const Double& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitShiftLeftObj(const String& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitShiftLeftObj(const ObjArray& objimp) const
{
	return copy();
}
void BObjectImp::selfBitShiftLeftObjImp(BObjectImp& objimp, BObject& obj)
{
	objimp.selfBitShiftLeftObj(*this, obj);
}
void BObjectImp::selfBitShiftLeftObj(BObjectImp& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitShiftLeftObj(BLong& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitShiftLeftObj(Double& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitShiftLeftObj(String& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitShiftLeftObj(ObjArray& objimp, BObject& obj)
{
	//
}

BObjectImp* BObjectImp::selfBitAndObjImp(const BObjectImp& objimp) const
{
	return objimp.selfBitAndObj(*this);
}
BObjectImp* BObjectImp::selfBitAndObj(const BObjectImp& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitAndObj(const BLong& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitAndObj(const Double& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitAndObj(const String& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitAndObj(const ObjArray& objimp) const
{
	return copy();
}
void BObjectImp::selfBitAndObjImp(BObjectImp& objimp, BObject& obj)
{
	objimp.selfBitAndObj(*this, obj);
}
void BObjectImp::selfBitAndObj(BObjectImp& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitAndObj(BLong& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitAndObj(Double& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitAndObj(String& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitAndObj(ObjArray& objimp, BObject& obj)
{
	//
}

BObjectImp* BObjectImp::selfBitOrObjImp(const BObjectImp& objimp) const
{
	return objimp.selfBitOrObj(*this);
}
BObjectImp* BObjectImp::selfBitOrObj(const BObjectImp& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitOrObj(const BLong& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitOrObj(const Double& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitOrObj(const String& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitOrObj(const ObjArray& objimp) const
{
	return copy();
}
void BObjectImp::selfBitOrObjImp(BObjectImp& objimp, BObject& obj)
{
	objimp.selfBitOrObj(*this, obj);
}
void BObjectImp::selfBitOrObj(BObjectImp& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitOrObj(BLong& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitOrObj(Double& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitOrObj(String& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitOrObj(ObjArray& objimp, BObject& obj)
{
	//
}

BObjectImp* BObjectImp::selfBitXorObjImp(const BObjectImp& objimp) const
{
	return objimp.selfBitXorObj(*this);
}
BObjectImp* BObjectImp::selfBitXorObj(const BObjectImp& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitXorObj(const BLong& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitXorObj(const Double& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitXorObj(const String& objimp) const
{
	return copy();
}
BObjectImp* BObjectImp::selfBitXorObj(const ObjArray& objimp) const
{
	return copy();
}
void BObjectImp::selfBitXorObjImp(BObjectImp& objimp, BObject& obj)
{
	objimp.selfBitXorObj(*this, obj);
}
void BObjectImp::selfBitXorObj(BObjectImp& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitXorObj(BLong& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitXorObj(Double& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitXorObj(String& objimp, BObject& obj)
{
	//
}
void BObjectImp::selfBitXorObj(ObjArray& objimp, BObject& obj)
{
	//
}

BObjectImp* BObjectImp::bitnot() const
{
	return copy();
}

void BObjectImp::operInsertInto( BObject& obj, const BObjectImp& objimp )
{
	obj.setimp( new BError( "Object is not a 'container'" ) );
}


void BObjectImp::operPlusEqual( BObject& obj, BObjectImp& objimp )
{
	objimp.selfPlusObjImp(*this, obj);
	//obj.setimp( objimp.selfPlusObjImp( *this ) );
}

void BObjectImp::operMinusEqual( BObject& obj, BObjectImp& objimp )
{
	objimp.selfMinusObjImp(*this, obj);
	//obj.setimp( selfMinusObjImp( objimp ) );
}

void BObjectImp::operTimesEqual( BObject& obj, BObjectImp& objimp )
{
	objimp.selfTimesObjImp(*this, obj);
	//obj.setimp( selfTimesObjImp( objimp ) );
}

void BObjectImp::operDivideEqual( BObject& obj, BObjectImp& objimp )
{
	objimp.selfDividedByObjImp(*this, obj);
	//obj.setimp( selfDividedByObjImp( objimp ) );
}

void BObjectImp::operModulusEqual( BObject& obj, BObjectImp& objimp )
{
	objimp.selfModulusObjImp(*this, obj);
	//obj.setimp( selfModulusObjImp( objimp ) );
}

BObject BObjectImp::operator-(void) const
{
  BObjectImp *newobj = inverse();
  return BObject(newobj);
}

BObjectImp* BObjectImp::inverse() const
{
	return UninitObject::create();
}

BObjectRef BObjectImp::OperSubscript( const BObject& obj )
{
	return BObjectRef(copy());
}

/*
  "All Objects are inherently good."
*/
bool BObjectImp::isTrue(void) const
{
  return true;
}

BObjectImp* BObjectImp::call_method( const char* methodname, Executor& ex )
{
	return new BError( string("Method '") + methodname + "' not found" );
}
BObjectImp* BObjectImp::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
	OSTRINGSTREAM os;
	os << "Method id '" << id << "' (" << getObjMethod(id)->code << ") not found";
	return new BError( string(OSTRINGSTREAM_STR(os)) );
}
BObjectRef BObjectImp::set_member( const char* membername, BObjectImp* valueimp )
{
	return BObjectRef( new BError( string("Member '") + membername + "' not found" ) );
}
BObjectRef BObjectImp::get_member( const char* membername )
{
	return BObjectRef( new BError( "Object does not support members" ) );
}
BObjectRef BObjectImp::get_member_id( const int id )
{
	ObjMember* memb = getObjMember(id);

	return get_member(memb->code);
}
BObjectRef BObjectImp::set_member_id( const int id, BObjectImp* valueimp )
{
	ObjMember* memb = getObjMember(id);

	return set_member(memb->code, valueimp);
}
long BObjectImp::contains( const BObjectImp& imp ) const
{
	return 0;
}

BObjectRef BObjectImp::operDotPlus( const char* name )
{
	return BObjectRef( new BError( "Operator .+ undefined" ) );
}

BObjectRef BObjectImp::operDotMinus( const char* name )
{
	return BObjectRef( new BError( "Operator .- undefined" ) );
}

BObjectRef BObjectImp::operDotQMark( const char* name )
{
	return BObjectRef( new BError( "Operator .? undefined" ) );
}

UninitObject* UninitObject::SharedInstance;
ref_ptr<BObjectImp> UninitObject::SharedInstanceOwner;

UninitObject::UninitObject() : BObjectImp(OTUninit)
{
}

BObjectImp *UninitObject::copy( void ) const
{
	return create();
}

size_t UninitObject::sizeEstimate() const
{
	return sizeof(UninitObject);
}

bool UninitObject::isTrue() const
{
	return false;
}
bool UninitObject::isEqual( const BObjectImp& imp ) const
{
	return (imp.isa( OTError ) || imp.isa( OTUninit ));
}

bool UninitObject::isLessThan(const BObjectImp& imp) const
{
	if (imp.isa( OTError ) || imp.isa( OTUninit ))
		return false;
	
	return true;
}


ObjArray::ObjArray() : BObjectImp(OTArray)
{
}

ObjArray::ObjArray( BObjectType type ) : BObjectImp( type )
{
}

ObjArray::ObjArray( const ObjArray& copyfrom ) :
	BObjectImp(copyfrom.type()),
	name_arr( copyfrom.name_arr ),
	ref_arr( copyfrom.ref_arr )
{
	deepcopy();
}

void ObjArray::deepcopy()
{
	for( iterator itr = ref_arr.begin(); itr != ref_arr.end(); ++itr )
	{
		if (itr->get())
		{
			/*
				NOTE: all BObjectRefs in an ObjArray reference BNamedObjects not BObjects
				HMMM, can this BNamedObject get destructed before we're done with it?
				No, we're making a copy, leaving the original be.
				(SO, bno's refcount should be >1 here)
			*/

			BObject *bo = itr->get();
			itr->set( new BObject( bo->impptr()->copy() ) );
		}
	}
}

BObjectImp *ObjArray::copy( void ) const
{  
	ObjArray *nobj = new ObjArray(*this);
	return nobj;
}

size_t ObjArray::sizeEstimate() const
{
	size_t size = sizeof(ObjArray);

	for( const_iterator itr = ref_arr.begin(); itr != ref_arr.end(); ++itr )
	{
		size += (*itr).sizeEstimate();
	}
	
	size += name_arr.size() * sizeof(string);
	for( const_name_iterator itr = name_arr.begin(); itr != name_arr.end(); ++itr )
	{
		size += (*itr).capacity();
	}

	return size;
}

/*
  Equality for arrays:
	  if the other guy is an array, check each element
	  otherwise not equal.
	  note that struct names aren't checked.
  TODO: check structure names too?
*/
bool ObjArray::isEqual(const BObjectImp& imp ) const
{
	if (!imp.isa( OTArray ))
		return false;

	const ObjArray& thatarr = static_cast<const ObjArray&>(imp);
	if (thatarr.ref_arr.size() != ref_arr.size())
		return false;

	for( unsigned i = 0; i < ref_arr.size(); ++i )
	{
		const BObjectRef& thisref = ref_arr[ i ];
		const BObjectRef& thatref = thatarr.ref_arr[i];

		const BObject* thisobj = thisref.get();
		const BObject* thatobj = thatref.get();
		if (thisobj != NULL && thatobj != NULL)
		{
			const BObjectImp& thisimp = thisobj->impref();
			const BObjectImp& thatimp = thatobj->impref();

			if (thisimp.isEqual( thatimp ))
				continue;
			else
				return false;
		}
		else if (thisobj == NULL && thatobj == NULL)
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
	if (index > ref_arr.size())
		return NULL;
	const BObjectRef& ref = ref_arr[ index-1 ];
	if (ref.get() == NULL)
		return NULL;
	return ref.get()->impptr();
}

BObjectImp* ObjArray::array_assign( BObjectImp* idx, BObjectImp* target )
{
	if (idx->isa( OTLong ))
	{
		BLong& lng = (BLong &) *idx;

		unsigned index = (unsigned) lng.value();
		if (index > ref_arr.size())
			ref_arr.resize( index );
		else if (index <= 0)
			return new BError( "Array index out of bounds" );
		
		BObjectRef& ref = ref_arr[ index-1 ];
		BObject* refobj = ref.get();
		if (refobj != NULL)
		{
			refobj->setimp( target );
		}
		else
		{
			ref.set( new BObject( target ) );
		}
		return ref->impptr();
	}
	else
	{
		return UninitObject::create();
	}
}

void ObjArray::operInsertInto(BObject& obj, const BObjectImp& objimp)
{
	ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}

BObjectImp* ObjArray::selfPlusObj(const BObjectImp& objimp) const
{
	auto_ptr<ObjArray> result (new ObjArray( *this ));
	result->ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
	return result.release();
}
BObjectImp* ObjArray::selfPlusObj(const BLong& objimp) const
{
	auto_ptr<ObjArray> result (new ObjArray( *this ));
	result->ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
	return result.release();
}
BObjectImp* ObjArray::selfPlusObj(const Double& objimp) const
{
	auto_ptr<ObjArray> result (new ObjArray( *this ));
	result->ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
	return result.release();
}
BObjectImp* ObjArray::selfPlusObj(const String& objimp) const
{
	auto_ptr<ObjArray> result (new ObjArray( *this ));
	result->ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
	return result.release();
}

BObjectImp* ObjArray::selfPlusObj(const ObjArray& objimp) const
{
	auto_ptr<ObjArray> result (new ObjArray( *this ));

	for( const_iterator itr = objimp.ref_arr.begin(); itr != objimp.ref_arr.end(); ++itr )
	{
		if (itr->get())
		{
			/*
				NOTE: all BObjectRefs in an ObjArray reference BNamedObjects not BObjects
				HMMM, can this BNamedObject get destructed before we're done with it?
				No, we're making a copy, leaving the original be.
				(SO, bno's refcount should be >1 here)
			*/
			BObject *bo = itr->get();

			result->ref_arr.push_back( BObjectRef( new BObject( (*bo)->copy() ) ) );
		}
		else
		{
			result->ref_arr.push_back( BObjectRef() );
		}
	}
	return result.release();
}

BObjectImp* ObjArray::selfPlusObjImp(const BObjectImp& other) const
{
	return other.selfPlusObj(*this);
}
void ObjArray::selfPlusObjImp(BObjectImp&  objimp , BObject& obj)
{
	objimp.selfPlusObj(*this,obj);
}
void ObjArray::selfPlusObj(BObjectImp& objimp, BObject& obj)
{
	ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}
void ObjArray::selfPlusObj(BLong& objimp, BObject& obj)
{
	ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}
void ObjArray::selfPlusObj(Double& objimp, BObject& obj)
{
	ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}
void ObjArray::selfPlusObj(String& objimp, BObject& obj)
{
	ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}
void ObjArray::selfPlusObj(ObjArray& objimp, BObject& obj)
{
	for( const_iterator itr = objimp.ref_arr.begin(), itrend = objimp.ref_arr.end(); itr != itrend; ++itr )
	{
		if (itr->get())
		{
			/*
				NOTE: all BObjectRefs in an ObjArray reference BNamedObjects not BObjects
				HMMM, can this BNamedObject get destructed before we're done with it?
				No, we're making a copy, leaving the original be.
				(SO, bno's refcount should be >1 here)
			*/
			BObject *bo = itr->get();

			ref_arr.push_back( BObjectRef( new BObject( (*bo)->copy() ) ) );
		}
		else
		{
			ref_arr.push_back( BObjectRef() );
		}
	}
}

BObjectRef ObjArray::OperMultiSubscript( stack<BObjectRef>& indices )
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
	//return BObjectRef(new BError( "Subscript out of range" ));

    unsigned index;
    if (start.isa( OTLong ))
    {
        BLong& lng = (BLong&) start;
        index = (unsigned) lng.value();
        if (index == 0 || index > ref_arr.size()  )
            return BObjectRef(new BError( "Array start index out of bounds" ));
    } else return BObjectRef(copy());

	// now the end index

    unsigned end;
    if (length.isa( OTLong ))
    {
		BLong& lng = (BLong &) length;
        end = (unsigned) lng.value();
		if (end == 0 || end > ref_arr.size() )
			return BObjectRef(new BError( "Array end index out of bounds" ));
    }
    else return BObjectRef(copy());

	ObjArray* str = new ObjArray();


	//auto_ptr<ObjArray> result (new ObjArray());
	unsigned i = 0;
	for( const_iterator itr = ref_arr.begin(); itr != ref_arr.end(); ++itr )
	{
		if (++i < index) continue;
		if (i > end) break;
		if (itr->get())
		{
			BObject *bo = itr->get();
			str->ref_arr.push_back( BObjectRef( new BObject( (*bo)->copy() ) ) );
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
//	return result.release();
	return BObjectRef( str );
}

BObjectRef ObjArray::OperSubscript( const BObject& rightobj )
{
	const BObjectImp& right = rightobj.impref();
	if (right.isa( OTLong )) // vector
	{
		BLong& lng = (BLong &) right;

		unsigned index = (unsigned) lng.value();
		if (index > ref_arr.size())
		{
			return BObjectRef( new BError( "Array index out of bounds" ) );
		}
		else if (index <= 0)
			return BObjectRef( new BError( "Array index out of bounds" ) );
		
		BObjectRef& ref = ref_arr[ index-1 ];
		if (ref.get() == NULL)
			ref.set( new BObject( UninitObject::create() ) );
		return ref;
	}
	else if (right.isa( OTString ))
	{
		// TODO: search for named variables (structure members)
		return BObjectRef(copy());
	}
	else
	{
		// TODO: crap out
		return BObjectRef(copy());
	}
}

BObjectRef ObjArray::get_member( const char* membername )
{
	int i = 0;
	for( const_name_iterator itr = name_arr.begin();
		 itr != name_arr.end();
		 ++itr, ++i )
	{
		const string& name = (*itr);
		if (stricmp( name.c_str(), membername ) == 0)
		{
			return ref_arr[i];
		}
	}

	 return BObjectRef( UninitObject::create() );
}

BObjectRef ObjArray::set_member( const char* membername, BObjectImp* valueimp )
{
	int i = 0;
	for( const_name_iterator itr = name_arr.begin();
		 itr != name_arr.end();
		 ++itr, ++i )
	{
		const string& name = (*itr);
		if (stricmp( name.c_str(), membername ) == 0)
		{
		   ref_arr[i].get()->setimp( valueimp );
		   return ref_arr[i];
		}
	}
	return BObjectRef( UninitObject::create() );
}

BObjectRef ObjArray::operDotPlus( const char* name )
{
	for( name_iterator itr = name_arr.begin(); itr != name_arr.end(); ++itr )
	{
		if (stricmp( name, (*itr).c_str() ) == 0)
		{
			return BObjectRef( new BError( "Member already exists" ) );
		}
	}
	name_arr.push_back( name );
	BObject* pnewobj = new BObject( UninitObject::create() );
	ref_arr.push_back( BObjectRef( pnewobj ) );
	return BObjectRef( pnewobj );
}

void ObjArray::addElement( BObjectImp* imp )
{
	ref_arr.push_back( BObjectRef( new BObject( imp ) ) );
}

string ObjArray::getStringRep() const
{
	OSTRINGSTREAM os;
	os << "{ ";
	bool any = false;
	for( Cont::const_iterator itr = ref_arr.begin(); itr != ref_arr.end(); ++itr )
	{
		if (any)
			os << ", ";
		else
			any = true;

		BObject *bo = (itr->get());

		if (bo != NULL)
		{
			string tmp = bo->impptr()->getStringRep();
			os << tmp;
		}
	}
	os << " }";

	return OSTRINGSTREAM_STR(os);
}

long ObjArray::contains( const BObjectImp& imp ) const
{
	for( Cont::const_iterator itr = ref_arr.begin(); itr != ref_arr.end(); ++itr )
	{
		if ( itr->get() )
		{
			BObject *bo = (itr->get());
			if ( bo == NULL )
			{
				cout << scripts_thread_script << " - '" << imp << " in array{}' check. Invalid data at index " << (itr-ref_arr.begin())+1 << endl;
				continue;
			}
			else if ( bo->impptr()->isEqual(imp) )
			{
				return (static_cast<long>((itr-ref_arr.begin())+1));
			}
		}
	}
	return 0;
}

class objref_cmp
{
public:
	bool operator()(const BObjectRef& x1, const BObjectRef& x2) const
	{
		const BObject* b1 = x1.get();
		const BObject* b2 = x2.get();
		if (b1 == NULL || b2 == NULL)
			return (&x1 < &x2);

		const BObject& r1 = *b1;
		const BObject& r2 = *b2;
		return (r1 < r2);
	}

};

BObjectImp* ObjArray::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
	switch(id)
	{
	case MTH_SIZE:
		if (ex.numParams() == 0)
			return new BLong( static_cast<int>(ref_arr.size()) );
		else
			return new BError( "array.size() doesn't take parameters." );

	case MTH_ERASE:
		if (name_arr.empty())
		{
			if (ex.numParams() == 1)
			{
				int idx;
				if (ex.getParam( 0, idx, 1, static_cast<int>(ref_arr.size()) )) // 1-based index
				{
					ref_arr.erase( ref_arr.begin() + idx - 1 );
					return new BLong(1);
				}
				else
				{
					return NULL;
				}
			}
			else
				return new BError( "array.erase(index) requires a parameter." );
		}
		break;
	case MTH_EXISTS:
		if (name_arr.empty())
		{
			if (ex.numParams() == 1)
			{
                int idx;
				if (ex.getParam( 0, idx ) && idx >= 0)
				{
					bool exists = (idx <= (int)ref_arr.size());
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
		if (name_arr.empty())
		{
			if (ex.numParams() == 2)
			{
				int idx;
				BObjectImp* imp = ex.getParamImp( 1 );
				if (ex.getParam( 0, idx, 1, static_cast<int>(ref_arr.size()+1) ) && imp != NULL) // 1-based index
				{ 
					--idx;
	// FIXME: 2008 Upgrades needed here? Make sure still working correctly under 2008
#if (defined(_WIN32) && _MSC_VER >= 1300) || (!defined(USE_STLPORT) && __GNUC__)
					BObjectRef tmp;
					ref_arr.insert( ref_arr.begin() + idx, tmp );
#else
					ref_arr.insert( ref_arr.begin() + idx );
#endif
					BObjectRef& ref = ref_arr[ idx ];
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
		if (name_arr.empty())
		{
			if (ex.numParams() == 1)
			{
				int idx;
				if (ex.getParam( 0, idx, 0, static_cast<int>(ref_arr.size()) )) // 1-based index
				{ 
					ref_arr.erase( ref_arr.begin() + idx, ref_arr.end() );
					return new BLong(1);
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
		if (name_arr.empty())
		{
			if (ex.numParams() == 1)
			{
				BObjectImp* imp = ex.getParamImp( 0 );
				if (imp) 
				{ 
					ref_arr.push_back( BObjectRef( new BObject( imp->copy() ) ) );

					return new BLong(1);
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
		if (name_arr.empty())
		{
			if (ex.numParams() == 0)
			{
				reverse( ref_arr.begin(), ref_arr.end() );
				return new BLong(1);
			}
			else
				return new BError( "array.reverse() doesn't take parameters." );
		}
		break;
	case MTH_SORT:
		if (name_arr.empty())
		{
			if (ex.numParams() == 0)
			{
				sort( ref_arr.begin(), ref_arr.end(), objref_cmp() );
				return new BLong(1);
			}
			else
				return new BError( "array.sort() doesn't take parameters." );
		}
		break;
	case MTH_RANDOMENTRY:
		if (name_arr.empty())
		{
			if (ex.numParams() == 0)
			{
				if (!ref_arr.empty())
				{
					const BObjectRef& ref = ref_arr[ random_int( static_cast<int>(ref_arr.size()) ) ];
					if (ref.get() == NULL)
						return NULL;
					return ref.get()->impptr();
				}
			}
			else
				return new BError( "array.randomentry() doesn't take parameters." );
		}
		break;

	default:
		return NULL;
	}
	return NULL;
}


BObjectImp* ObjArray::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return NULL;
}

void ObjArray::packonto( ostream& os ) const
{
	os << "a" << ref_arr.size() << ":";
	for( const_iterator itr = ref_arr.begin(); itr != ref_arr.end(); ++itr )
	{
		if (itr->get())
		{
			BObject *bo = itr->get();
			bo->impptr()->packonto( os );
		}
		else
		{
			os << "x";
		}
	}
}

BObjectImp* ObjArray::unpack( istream& is )
{
	unsigned arrsize;
	char colon;
	if (! (is >> arrsize >> colon))
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
	auto_ptr<ObjArray> arr (new ObjArray);
	arr->ref_arr.resize( arrsize );
	for( unsigned i = 0; i < arrsize; ++i )
	{
		BObjectImp* imp = BObjectImp::unpack( is );
		if (imp != NULL && !imp->isa(OTUninit))
		{
			arr->ref_arr[ i ].set( new BObject( imp ) );
		}
	}
	return arr.release();
}

BApplicPtr::BApplicPtr( const BApplicObjType* pointer_type, void *ptr ) :
	BObjectImp( OTApplicPtr ),
	ptr_(ptr),
	pointer_type_(pointer_type)
{
}

BObjectImp *BApplicPtr::copy() const
{
	return new BApplicPtr( pointer_type_, ptr_ );
}

size_t BApplicPtr::sizeEstimate() const
{
	return sizeof(BApplicPtr);
}

const BApplicObjType* BApplicPtr::pointer_type() const
{
	return pointer_type_;
}

void *BApplicPtr::ptr() const
{
	return ptr_;
}

string BApplicPtr::getStringRep() const
{
	return "<appptr>";
}


void BApplicPtr::printOn( ostream& os) const
{
	os << "<appptr>";
}

string BApplicObjBase::getStringRep() const
{
	return string("<appobj:") + typeOf() + ">";
}

void BApplicObjBase::printOn( ostream& os ) const
{
	os << getStringRep();
}

