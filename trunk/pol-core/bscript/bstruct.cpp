/*
History
=======
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2008/02/11 Turley: BStruct::unpack() will accept zero length Structs

Notes
=======

*/

#include "clib/stl_inc.h"

#include "clib/stlutil.h"
#include "bstruct.h"
#include "berror.h"
#include "impstr.h"
#include "contiter.h"
#include "executor.h"
#include "objmembers.h"
#include "objmethods.h"

BStruct::BStruct() : BObjectImp( OTStruct )
{
}

BStruct::BStruct( BObjectType type ) : BObjectImp( type )
{
}

BStruct::BStruct( const BStruct& other, BObjectType type ) :
    BObjectImp( type )
{
    for( Contents::const_iterator itr = other.contents_.begin(); itr != other.contents_.end(); ++itr )
    {
        const string& key = (*itr).first;
        const BObjectRef& bvalref = (*itr).second;

        contents_[ key ] = BObjectRef( new BObject( bvalref->impref().copy() ) );
    }
}

BStruct::BStruct( istream& is, unsigned size, BObjectType type ) :
    BObjectImp( type )
{
    for( unsigned i = 0; i < size; ++i )
    {
        BObjectImp* keyimp = BObjectImp::unpack( is );
        BObjectImp* valimp = BObjectImp::unpack( is );
        if (keyimp != NULL && valimp != NULL &&
            keyimp->isa( OTString ))
        {
            String* str = static_cast<String*>(keyimp);

            contents_[ str->value() ].set( new BObject( valimp ) );
            
            BObject cleaner( str );
        }
        else
        {
            if (keyimp)
                BObject objk(keyimp);
            if (valimp)
                BObject objv(valimp);
        }
    }
}

BObjectImp* BStruct::copy() const
{
    passert( isa(OTStruct) );
    return new BStruct(*this, OTStruct);
}
char BStruct::packtype() const
{
    return 't';
}

const char* BStruct::typetag() const
{
    return "struct";
}

const char* BStruct::typeOf() const
{
    return "Struct";
}


BObjectImp* BStruct::unpack( istream& is )
{
    unsigned size;
    char colon;
    if (! (is >> size >> colon))
    {
        return new BError( "Unable to unpack struct elemcount" );
    }
	if ( (int)size < 0 )
    {
        return new BError( "Unable to unpack struct elemcount. Length given must be positive integer!" );
    }
	if ( colon != ':' ) 
	{ 
		return new BError( "Unable to unpack struct elemcount. Bad format. Colon not found!" ); 
	}
	return new BStruct( is, size, OTStruct );
}

void BStruct::FormatForStringRep( ostream& os, 
                                  const string& key, 
                                  const BObjectRef& bvalref ) const
{
    os << key
       << " = "
       << bvalref->impref().getFormattedStringRep();
}

class BStructIterator : public ContIterator
{
public:
    BStructIterator( BStruct* pDict, BObject* pIterVal );
    virtual BObject* step();

private:
    BObject m_StructObj;
    BStruct* m_pStruct;
    BObjectRef m_IterVal;
    string key;
    bool m_First;
};
BStructIterator::BStructIterator( BStruct* pStruct, BObject* pIterVal ) :
    m_StructObj( pStruct ),
    m_pStruct( pStruct ),
    m_IterVal( pIterVal ),
    key( "" ),
    m_First(true)
{
}

BObject* BStructIterator::step()
{
    if (m_First)
    {
        BStruct::Contents::iterator itr = m_pStruct->contents_.begin();
        if (itr == m_pStruct->contents_.end())
            return NULL;

        m_First = false;
        key = (*itr).first;
        m_IterVal->setimp( new String( key ) );

        BObjectRef& oref = (*itr).second;
        return oref.get();
    }
    else
    {
        BStruct::Contents::iterator itr = m_pStruct->contents_.find( key );
        if (itr == m_pStruct->contents_.end())
            return NULL;
        ++itr;
        if (itr == m_pStruct->contents_.end())
            return NULL;

        key = (*itr).first;
        m_IterVal->setimp( new String(key) );

        BObjectRef& oref = (*itr).second;
        return oref.get();
    }
}

ContIterator* BStruct::createIterator( BObject* pIterVal )
{
    return new BStructIterator( this, pIterVal );
}

unsigned long BStruct::sizeEstimate() const
{
    unsigned long size = sizeof(BStruct);
    for( Contents::const_iterator itr = contents_.begin(); itr != contents_.end(); ++itr )
    {
        const string& bkey = (*itr).first;
        const BObjectRef& bvalref = (*itr).second;
        size += bkey.size() + bvalref.sizeEstimate();
    }
    return size;
}

unsigned BStruct::mapcount() const
{
    return contents_.size();
}


BObjectRef BStruct::set_member( const char* membername, BObjectImp* value )
{
    string key( membername );

    Contents::iterator itr = contents_.find( key );
    if (itr != contents_.end())
    {
        BObjectRef& oref = (*itr).second;
        oref->setimp( value );
        return oref;
    }
    else
    {
        BObjectRef ref( new BObject( value ) );
        contents_[ key ] = ref;
        return ref;
    }
}

// used programmatically
const BObjectImp* BStruct::FindMember( const char* name )
{
    string key( name );

    Contents::iterator itr = contents_.find( key );
    if (itr != contents_.end())
    {
        return (*itr).second->impptr();
    }
    else
    {
        return NULL;
    }
}

BObjectRef BStruct::get_member( const char* membername )
{
    string key( membername );

    Contents::iterator itr = contents_.find( key );
    if (itr != contents_.end())
    {
        return (*itr).second;
    }
    else
    {
        return BObjectRef(UninitObject::create());
    }
}

BObjectRef BStruct::OperSubscript( const BObject& obj )
{
    if (obj->isa( OTString ))
    {
        const String* keystr = static_cast<const String*>(obj.impptr());

        Contents::iterator itr = contents_.find( keystr->value() );
        if (itr != contents_.end())
        {
            BObjectRef& oref = (*itr).second;
            return oref;
        }
        else
        {
            return BObjectRef( UninitObject::create() );
        }
    }
    else if (obj->isa( OTLong ))
    {
        throw runtime_error( "some fool used operator[] on a struct, with an Integer index" );
    }
    else
    {
        return BObjectRef( new BError( "Struct members can only be accessed by name" ) );
    }
}

BObjectImp* BStruct::array_assign( BObjectImp* idx, BObjectImp* target )
{
    if (idx->isa( OTString ))
    {
        String* keystr = static_cast<String*>(idx);

        Contents::iterator itr = contents_.find( keystr->value() );
        if (itr != contents_.end())
        {
            BObjectRef& oref = (*itr).second;
            oref->setimp( target );
            return target;
        }
        else
        {
            contents_[ keystr->value() ].set( new BObject( target ) );
            return target;
        }
    }
    else if (idx->isa( OTLong ))
    {
        throw runtime_error( "some fool tried to use operator[] := on a struct, with an Integer index" );
    }
    else
    {
        return new BError( "Struct members can only be accessed by name" );
    }
}

void BStruct::addMember( const char* name, BObjectRef val )
{
    string key( name );
    contents_[key] = val;
}

void BStruct::addMember( const char* name, BObjectImp* imp )
{
    string key( name );
    contents_[key] = BObjectRef(imp);
}

BObjectImp* BStruct::call_method_id( const int id, Executor& ex )
{
    BObject* keyobj;
	BObject* valobj;
    switch(id)
    {
    case MTH_SIZE:
		if (ex.numParams() == 0)
			return new BLong( contents_.size() );
		else
			return new BError( "struct.size() doesn't take parameters." );

    case MTH_ERASE:
		if (ex.numParams() == 1 &&
			(keyobj = ex.getParamObj( 0 )))
        {
            if (!keyobj->isa( OTString ))
                return new BError( "Struct keys must be strings" );
            String* strkey = static_cast<String*>(keyobj->impptr());
            int nremove = contents_.erase( strkey->value() );
            return new BLong( nremove );
        }
        else
        {
			return new BError( "struct.erase(key) requires a parameter." );
        }
        break;
    case MTH_INSERT:
		if (ex.numParams() == 2 &&
			(keyobj = ex.getParamObj( 0 )) &&
			(valobj = ex.getParamObj( 1 )))
        {
            if (!keyobj->isa( OTString ))
                return new BError( "Struct keys must be strings" );
            String* strkey = static_cast<String*>(keyobj->impptr());
            contents_[ strkey->value() ] = BObjectRef( new BObject( valobj->impptr()->copy() ) );
            return new BLong( contents_.size() );
        }
        else
        {
            return new BError( "struct.insert(key,value) requires two parameters." );
        }
        break;
    case MTH_EXISTS:
		if (ex.numParams() == 1 &&
			(keyobj = ex.getParamObj( 0 )))
        {
            if (!keyobj->isa( OTString ))
                return new BError( "Struct keys must be strings" );
            String* strkey = static_cast<String*>(keyobj->impptr());
            int count = contents_.count( strkey->value() );
            return new BLong( count );
        }
        else
        {
            return new BError( "struct.exists(key) requires a parameter." );
        }

    case MTH_KEYS:
		if (ex.numParams() == 0)
		{
	        ObjArray* arr = new ObjArray;
		    Contents::const_iterator itr = contents_.begin(), end = contents_.end();
	        for( ; itr != end; ++itr )
			{
	            const string& key = (*itr).first;

	            arr->addElement( new String( key ) );
			}
			return arr;
		}
		else
			return new BError( "struct.keys() doesn't take parameters." );
        break;
    default:
        return NULL;
    }
}

BObjectImp* BStruct::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return NULL;
}

void BStruct::packonto( ostream& os ) const
{
    os << packtype() << contents_.size() << ":";
    Contents::const_iterator itr = contents_.begin(), end = contents_.end();
    for( ; itr != end; ++itr )
    {
        const string& key = (*itr).first;
        const BObjectRef& bvalref = (*itr).second;

        String::packonto( os, key );
        bvalref->impref().packonto( os );
    }
}

string BStruct::getStringRep() const
{
    OSTRINGSTREAM os;
    os << typetag() << "{ ";
    bool any = false;

    Contents::const_iterator itr = contents_.begin(), end = contents_.end();
    for( ; itr != end; ++itr )
    {
        const string& key = (*itr).first;
        const BObjectRef& bvalref = (*itr).second;

        if (any)
            os << ", ";
        else
            any = true;

        FormatForStringRep( os, key, bvalref );
    }

    os << " }";

    return OSTRINGSTREAM_STR(os);
}


BObjectRef BStruct::operDotPlus( const char* name )
{
    string key(name);
    if (contents_.count( key ) == 0)
    {
        BObject* pnewobj = new BObject(new UninitObject);
        contents_[ key ] = BObjectRef( pnewobj );
        return BObjectRef(pnewobj);
    }
    else
    {
        return BObjectRef( new BError( "Member already exists" ) );
    }
}

const BStruct::Contents& BStruct::contents() const
{
	return contents_;
}

