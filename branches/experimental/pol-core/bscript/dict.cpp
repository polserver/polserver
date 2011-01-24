/*
History
=======
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2008/02/11 Turley:    BDictionary::unpack() will accept zero length Dictionarys
2009/12/21 Turley:    ._method() call fix

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "dict.h"
#include "executor.h"

#include "../clib/stlutil.h"

#include "berror.h"
#include "impstr.h"
#include "contiter.h"
#include "objmembers.h"
#include "objmethods.h"

BDictionary::BDictionary() : BObjectImp( OTDictionary )
{
}

BDictionary::BDictionary(BObjectType type) : BObjectImp( type )
{
}

BDictionary::BDictionary( const BDictionary& dict, BObjectType type ) :
    BObjectImp( type )
{
    for( Contents::const_iterator itr = dict.contents_.begin(); itr != dict.contents_.end(); ++itr )
    {
        const BObject& bkeyobj = (*itr).first;
        const BObjectRef& bvalref = (*itr).second;

        contents_[ bkeyobj ] = BObjectRef( new BObject( bvalref->impref().copy() ) );
    }
}

BDictionary::BDictionary( istream& is, unsigned size, BObjectType type ) :
    BObjectImp( type )
{
    for( unsigned i = 0; i < size; ++i )
    {
        BObjectImp* keyimp = BObjectImp::unpack( is );
        BObjectImp* valimp = BObjectImp::unpack( is );
		if (keyimp != NULL && valimp != NULL)
        {
            BObject keyobj(keyimp);
            contents_[ keyobj ].set( new BObject( valimp ) );
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

class BDictionaryIterator : public ContIterator
{
public:
    BDictionaryIterator( BDictionary* pDict, BObject* pIterVal );
    virtual BObject* step();

private:
    BObject m_DictObj;
    BDictionary* m_pDict;
    BObjectRef m_IterVal;
    BObject m_Key;
    bool m_First;
};
BDictionaryIterator::BDictionaryIterator( BDictionary* pDict, BObject* pIterVal ) :
    m_DictObj( pDict ),
    m_pDict( pDict ),
    m_IterVal( pIterVal ),
    m_Key( UninitObject::create() ),
    m_First(true)
{
}

BObject* BDictionaryIterator::step()
{
    if (m_First)
    {
        BDictionary::Contents::iterator itr = m_pDict->contents_.begin();
        if (itr == m_pDict->contents_.end())
            return NULL;

        m_First = false;
        const BObject& okey = (*itr).first;
        m_Key.setimp( okey.impptr()->copy() );
        m_IterVal->setimp( m_Key.impptr() );

        BObjectRef& oref = (*itr).second;
        return oref.get();
    }
    else
    {
        BDictionary::Contents::iterator itr = m_pDict->contents_.find( m_Key );
        if (itr == m_pDict->contents_.end())
            return NULL;
        ++itr;
        if (itr == m_pDict->contents_.end())
            return NULL;

        const BObject& okey = (*itr).first;
        m_Key.setimp( okey.impptr()->copy() );
        m_IterVal->setimp( m_Key.impptr() );

        BObjectRef& oref = (*itr).second;
        return oref.get();
    }
}

ContIterator* BDictionary::createIterator( BObject* pIterVal )
{
    return new BDictionaryIterator( this, pIterVal );
}

BObjectImp* BDictionary::copy() const
{
    return new BDictionary( *this );
}

unsigned int BDictionary::sizeEstimate() const
{
    unsigned int size = sizeof(BDictionary);
    for( Contents::const_iterator itr = contents_.begin(); itr != contents_.end(); ++itr )
    {
        const BObject& bkeyobj = (*itr).first;
        const BObjectRef& bvalref = (*itr).second;
        size += bkeyobj.sizeEstimate() + bvalref.sizeEstimate();
    }
    return size;
}

unsigned BDictionary::mapcount() const
{
    return contents_.size();
}

BObjectRef BDictionary::set_member( const char* membername, BObjectImp* value )
{
    BObject key( new String(membername) );

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

BObjectRef BDictionary::get_member( const char* membername )
{
    BObject key( new String(membername) );

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


BObjectRef BDictionary::OperSubscript( const BObject& obj )
{
    if (obj->isa( OTString ) || obj->isa( OTLong ) || obj->isa( OTDouble ) || obj->isa( OTApplicObj ))
    {
        Contents::iterator itr = contents_.find( obj );
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
    else
    {
        return BObjectRef( new BError( "Dictionary keys must be integer, real, or string" ) );
    }
}

BObjectImp* BDictionary::array_assign( BObjectImp* idx, BObjectImp* target )
{
    if (idx->isa( OTString ) || idx->isa( OTLong ) || idx->isa( OTDouble ) || idx->isa( OTApplicObj ))
    {
        BObject obj( idx );
        Contents::iterator itr = contents_.find( obj );
        if (itr != contents_.end())
        {
            BObjectRef& oref = (*itr).second;
            oref->setimp( target );
            return target;
        }
        else
        {
            contents_[ obj ].set( new BObject( target ) );
            return target;
        }
    }
    else
    {
       return new BError( "Dictionary keys must be integer, real, or string" );
    }
}

void BDictionary::addMember( const char* name, BObjectRef val )
{
    BObject key( new String(name) );
    contents_[key] = val;
}

void BDictionary::addMember( const char* name, BObjectImp* imp )
{
    BObject key( new String(name) );
    contents_[key] = BObjectRef(imp);
}

void BDictionary::addMember( BObjectImp* keyimp, BObjectImp* valimp )
{
    BObject key( keyimp );
    contents_[key] = BObjectRef(valimp);
}

BObjectImp* BDictionary::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
    BObject* keyobj;
    BObject* valobj;
    switch(id)
    {
    case MTH_SIZE:
		if (ex.numParams() == 0)
			return new BLong( contents_.size() );
		else
			return new BError( "dictionary.size() doesn't take parameters." );
        break;
    case MTH_ERASE:
		if (ex.numParams() == 1 &&
			(keyobj = ex.getParamObj( 0 )))
        {
            if (! (keyobj->isa( OTLong ) || keyobj->isa( OTString ) || keyobj->isa( OTDouble ) || keyobj->isa( OTApplicObj )))
                return new BError( "Dictionary keys must be integer, real, or string" );
            int nremove = contents_.erase( *keyobj );
            return new BLong( nremove );
        }
        else
        {
			return new BError( "dictionary.erase(key) requires a parameter." );
        }
        break;
    case MTH_INSERT:
		if (ex.numParams() == 2 &&
			(keyobj = ex.getParamObj( 0 )) &&
			(valobj = ex.getParamObj( 1 )))
        {
            if (! (keyobj->isa( OTLong ) || keyobj->isa( OTString ) || keyobj->isa( OTDouble ) || keyobj->isa( OTApplicObj )))
                return new BError( "Dictionary keys must be integer, real, or string" );
            BObject key( keyobj->impptr()->copy() );
            contents_[ key ] = BObjectRef( new BObject( valobj->impptr()->copy() ) );
            return new BLong( contents_.size() );
        }
        else
        {
            return new BError( "dictionary.insert(key,value) requires two parameters." );
        }
        break;
    case MTH_EXISTS:
		if (ex.numParams() == 1 &&
			(keyobj = ex.getParamObj( 0 )))
        {
            if (! (keyobj->isa( OTLong ) || keyobj->isa( OTString ) || keyobj->isa( OTDouble ) || keyobj->isa( OTApplicObj )))
                return new BError( "Dictionary keys must be integer, real, or string" );
            int count = contents_.count( *keyobj );
            return new BLong( count );
        }
        else
        {
            return new BError( "dictionary.exists(key) requires a parameter." );
        }
        break;
    case MTH_KEYS:
		if (ex.numParams() == 0)
		{
	        ObjArray* arr = new ObjArray;
		    Contents::const_iterator itr = contents_.begin(), end = contents_.end();
	        for( ; itr != end; ++itr )
			{
	            const BObject& bkeyobj = (*itr).first;

	            arr->addElement( bkeyobj.impref().copy() );
			}
			return arr;
		}
		else
			return new BError( "dictionary.keys() doesn't take parameters." );
        break;
    default:
        return NULL;
    }
}

BObjectImp* BDictionary::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return NULL;
}

char BDictionary::packtype() const
{
    return 'd';
}

const char* BDictionary::typetag() const
{
    return "dict";
}

const char* BDictionary::typeOf() const
{
    return "Dictionary";
}

void BDictionary::packonto( ostream& os ) const
{
    os << packtype() << contents_.size() << ":";
    Contents::const_iterator itr = contents_.begin(), end = contents_.end();
    for( ; itr != end; ++itr )
    {
        const BObject& bkeyobj = (*itr).first;
        const BObjectRef& bvalref = (*itr).second;

        bkeyobj.impref().packonto( os );
        bvalref->impref().packonto( os );
    }
}


BObjectImp* BDictionary::unpack( istream& is )
{
    unsigned size;
    char colon;
    if (! (is >> size >> colon))
    {
        return new BError( "Unable to unpack dictionary elemcount" );
    }
	if ( (int)size < 0 )
    {
        return new BError( "Unable to unpack dictionary elemcount. Length given must be positive integer!" );
    }
	if ( colon != ':' ) 
	{ 
		return new BError( "Unable to unpack dictionary elemcount. Bad format. Colon not found!" ); 
	}
	return new BDictionary( is, size );
}

string BDictionary::getStringRep() const
{
    OSTRINGSTREAM os;
    os << typetag() << "{ ";
    bool any = false;

    Contents::const_iterator itr = contents_.begin(), end = contents_.end();
    for( ; itr != end; ++itr )
    {
        const BObject& bkeyobj = (*itr).first;
        const BObjectRef& bvalref = (*itr).second;

        if (any)
            os << ", ";
        else
            any = true;

        FormatForStringRep( os, bkeyobj, bvalref );
    }

    os << " }";

    return OSTRINGSTREAM_STR(os);
}

void BDictionary::FormatForStringRep( ostream& os, 
                                      const BObject& bkeyobj, 
                                      const BObjectRef& bvalref ) const
{
    os << bkeyobj.impref().getFormattedStringRep()
       << " -> "
       << bvalref->impref().getFormattedStringRep();
}


BObjectRef BDictionary::operDotPlus( const char* name )
{
    BObject key( new String(name) );
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

const BDictionary::Contents& BDictionary::contents() const
{
	return contents_;
}



