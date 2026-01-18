/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2008/02/11 Turley:    BDictionary::unpack() will accept zero length Dictionarys
 * - 2009/12/21 Turley:    ._method() call fix
 */


#include "dict.h"

#include <stddef.h>

#include "../clib/stlutil.h"
#include "berror.h"
#include "contiter.h"
#include "executor.h"
#include "impstr.h"
#include "objmethods.h"


namespace Pol::Bscript
{
BDictionary::BDictionary() : BObjectImp( OTDictionary ), contents_() {}

BDictionary::BDictionary( BObjectType type ) : BObjectImp( type ), contents_() {}

BDictionary::BDictionary( const BDictionary& dict, BObjectType type )
    : BObjectImp( type ), contents_()
{
  for ( const auto& elem : dict.contents_ )
  {
    const BObject& bkeyobj = elem.first;
    const BObjectRef& bvalref = elem.second;

    contents_[bkeyobj] = BObjectRef( new BObject( bvalref->impref().copy() ) );
  }
}

BDictionary::BDictionary( std::istream& is, unsigned size, BObjectType type )
    : BObjectImp( type ), contents_()
{
  for ( unsigned i = 0; i < size; ++i )
  {
    BObjectImp* keyimp = BObjectImp::unpack( is );
    BObjectImp* valimp = BObjectImp::unpack( is );
    if ( keyimp != nullptr && valimp != nullptr )
    {
      BObject keyobj( keyimp );
      contents_[keyobj].set( new BObject( valimp ) );
    }
    else
    {
      if ( keyimp )
        BObject objk( keyimp );
      if ( valimp )
        BObject objv( valimp );
    }
  }
}

class BDictionaryIterator final : public ContIterator
{
public:
  BDictionaryIterator( BDictionary* pDict, BObject* pIterVal );
  BObject* step() override;

private:
  BObject m_DictObj;
  BDictionary* m_pDict;
  BObjectRef m_IterVal;
  BObject m_Key;
  bool m_First;
};
BDictionaryIterator::BDictionaryIterator( BDictionary* pDict, BObject* pIterVal )
    : ContIterator(),
      m_DictObj( pDict ),
      m_pDict( pDict ),
      m_IterVal( pIterVal ),
      m_Key( UninitObject::create() ),
      m_First( true )
{
}

BObject* BDictionaryIterator::step()
{
  if ( m_First )
  {
    auto itr = m_pDict->contents_.begin();
    if ( itr == m_pDict->contents_.end() )
      return nullptr;

    m_First = false;
    const BObject& okey = ( *itr ).first;
    m_Key.setimp( okey.impptr()->copy() );
    m_IterVal->setimp( m_Key.impptr() );

    BObjectRef& oref = ( *itr ).second;
    return oref.get();
  }

  auto itr = m_pDict->contents_.find( m_Key );
  if ( itr == m_pDict->contents_.end() )
    return nullptr;
  ++itr;
  if ( itr == m_pDict->contents_.end() )
    return nullptr;

  const BObject& okey = ( *itr ).first;
  m_Key.setimp( okey.impptr()->copy() );
  m_IterVal->setimp( m_Key.impptr() );

  BObjectRef& oref = ( *itr ).second;
  return oref.get();
}

ContIterator* BDictionary::createIterator( BObject* pIterVal )
{
  return new BDictionaryIterator( this, pIterVal );
}

BObjectImp* BDictionary::copy() const
{
  return new BDictionary( *this );
}

size_t BDictionary::sizeEstimate() const
{
  return sizeof( BDictionary ) +
         Clib::memsize_keyvalue( contents_, []( const auto& v ) { return v.sizeEstimate(); } );
}

size_t BDictionary::mapcount() const
{
  return contents_.size();
}

BObjectRef BDictionary::set_member( const char* membername, BObjectImp* value, bool copy )
{
  BObject key( new String( membername ) );
  BObjectImp* target = copy ? value->copy() : value;

  auto itr = contents_.find( key );
  if ( itr != contents_.end() )
  {
    BObjectRef& oref = ( *itr ).second;
    oref->setimp( target );
    return oref;
  }

  BObjectRef ref( new BObject( target ) );
  contents_[key] = ref;
  return ref;
}

BObjectRef BDictionary::get_member( const char* membername )
{
  BObject key( new String( membername ) );

  auto itr = contents_.find( key );
  if ( itr != contents_.end() )
  {
    return ( *itr ).second;
  }

  return BObjectRef( UninitObject::create() );
}


BObjectRef BDictionary::OperSubscript( const BObject& obj )
{
  if ( obj->isa( OTString ) || obj->isa( OTLong ) || obj->isa( OTDouble ) ||
       obj->isa( OTApplicObj ) )
  {
    auto itr = contents_.find( obj );
    if ( itr != contents_.end() )
    {
      BObjectRef& oref = ( *itr ).second;
      return oref;
    }

    return BObjectRef( UninitObject::create() );
  }

  return BObjectRef( new BError( "Dictionary keys must be integer, real, or string" ) );
}

BObjectImp* BDictionary::array_assign( BObjectImp* idx, BObjectImp* target, bool copy )
{
  if ( idx->isa( OTString ) || idx->isa( OTLong ) || idx->isa( OTDouble ) ||
       idx->isa( OTApplicObj ) )
  {
    BObjectImp* new_target = copy ? target->copy() : target;

    BObject obj( idx );
    auto itr = contents_.find( obj );
    if ( itr != contents_.end() )
    {
      BObjectRef& oref = ( *itr ).second;
      oref->setimp( new_target );
      return new_target;
    }

    contents_[BObject( obj->copy() )].set( new BObject( new_target ) );
    return new_target;
  }

  return new BError( "Dictionary keys must be integer, real, or string" );
}

void BDictionary::addMember( const char* name, BObjectRef val )
{
  BObject key( new String( name ) );
  contents_[key] = std::move( val );
}

void BDictionary::addMember( const char* name, BObjectImp* imp )
{
  BObject key( new String( name ) );
  contents_[key] = BObjectRef( imp );
}

void BDictionary::addMember( BObjectImp* keyimp, BObjectImp* valimp )
{
  BObject key( keyimp );
  contents_[key] = BObjectRef( valimp );
}

BObjectImp* BDictionary::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
{
  BObject* keyobj;
  BObject* valobj;
  switch ( id )
  {
  case MTH_SIZE:
    if ( ex.numParams() == 0 )
      return new BLong( static_cast<int>( contents_.size() ) );
    else
      return new BError( "dictionary.size() doesn't take parameters." );
    break;
  case MTH_ERASE:
    if ( ex.numParams() == 1 && ( keyobj = ex.getParamObj( 0 ) ) != nullptr )
    {
      if ( !( keyobj->isa( OTLong ) || keyobj->isa( OTString ) || keyobj->isa( OTDouble ) ||
              keyobj->isa( OTApplicObj ) ) )
        return new BError( "Dictionary keys must be integer, real, or string" );
      int nremove = static_cast<int>( contents_.erase( *keyobj ) );
      return new BLong( nremove );
    }
    else
    {
      return new BError( "dictionary.erase(key) requires a parameter." );
    }
    break;
  case MTH_INSERT:
    if ( ex.numParams() == 2 && ( keyobj = ex.getParamObj( 0 ) ) != nullptr &&
         ( valobj = ex.getParamObj( 1 ) ) != nullptr )
    {
      if ( !( keyobj->isa( OTLong ) || keyobj->isa( OTString ) || keyobj->isa( OTDouble ) ||
              keyobj->isa( OTApplicObj ) ) )
        return new BError( "Dictionary keys must be integer, real, or string" );
      BObject key( keyobj->impptr()->copy() );
      contents_[key] = BObjectRef( new BObject( valobj->impptr()->copy() ) );
      return new BLong( static_cast<int>( contents_.size() ) );
    }
    else
    {
      return new BError( "dictionary.insert(key,value) requires two parameters." );
    }
    break;
  case MTH_EXISTS:
    if ( ex.numParams() == 1 && ( keyobj = ex.getParamObj( 0 ) ) != nullptr )
    {
      if ( !( keyobj->isa( OTLong ) || keyobj->isa( OTString ) || keyobj->isa( OTDouble ) ||
              keyobj->isa( OTApplicObj ) ) )
        return new BError( "Dictionary keys must be integer, real, or string" );
      int count = static_cast<int>( contents_.count( *keyobj ) );
      return new BLong( count );
    }
    else
    {
      return new BError( "dictionary.exists(key) requires a parameter." );
    }
    break;
  case MTH_KEYS:
    if ( ex.numParams() == 0 )
    {
      auto arr = new ObjArray;
      for ( const auto& content : contents_ )
      {
        const BObject& bkeyobj = content.first;

        arr->addElement( bkeyobj.impref().copy() );
      }
      return arr;
    }
    else
      return new BError( "dictionary.keys() doesn't take parameters." );
    break;
  default:
    return nullptr;
  }
}

BObjectImp* BDictionary::call_method( const char* methodname, Executor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_method_id( objmethod->id, ex );
  return nullptr;
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
u8 BDictionary::typeOfInt() const
{
  return OTDictionary;
}

void BDictionary::packonto( std::ostream& os ) const
{
  os << packtype() << contents_.size() << ":";
  for ( const auto& content : contents_ )
  {
    const BObject& bkeyobj = content.first;
    const BObjectRef& bvalref = content.second;

    bkeyobj.impref().packonto( os );
    bvalref->impref().packonto( os );
  }
}


BObjectImp* BDictionary::unpack( std::istream& is )
{
  unsigned size;
  char colon;
  if ( !( is >> size >> colon ) )
  {
    return new BError( "Unable to unpack dictionary elemcount" );
  }
  if ( (int)size < 0 )
  {
    return new BError(
        "Unable to unpack dictionary elemcount. Length given must be positive integer!" );
  }
  if ( colon != ':' )
  {
    return new BError( "Unable to unpack dictionary elemcount. Bad format. Colon not found!" );
  }
  return new BDictionary( is, size );
}

std::string BDictionary::getStringRep() const
{
  OSTRINGSTREAM os;
  os << typetag() << "{ ";
  bool any = false;

  for ( const auto& content : contents_ )
  {
    const BObject& bkeyobj = content.first;
    const BObjectRef& bvalref = content.second;

    if ( any )
      os << ", ";
    else
      any = true;

    FormatForStringRep( os, bkeyobj, bvalref );
  }

  os << " }";

  return OSTRINGSTREAM_STR( os );
}

void BDictionary::FormatForStringRep( std::ostream& os, const BObject& bkeyobj,
                                      const BObjectRef& bvalref ) const
{
  os << bkeyobj.impref().getFormattedStringRep() << " -> "
     << bvalref->impref().getFormattedStringRep();
}


BObjectRef BDictionary::operDotPlus( const char* name )
{
  BObject key( new String( name ) );
  if ( contents_.count( key ) == 0 )
  {
    auto pnewobj = new BObject( new UninitObject );
    contents_[key] = BObjectRef( pnewobj );
    return BObjectRef( pnewobj );
  }

  return BObjectRef( new BError( "Member already exists" ) );
}

const BDictionary::Contents& BDictionary::contents() const
{
  return contents_;
}
}  // namespace Pol::Bscript
