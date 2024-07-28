/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2008/02/11 Turley:    BStruct::unpack() will accept zero length Structs
 * - 2009/09/05 Turley:    Added struct .? and .- as shortcut for .exists() and .erase()
 * - 2009/12/21 Turley:    ._method() call fix
 */


#include "bstruct.h"

#include <stddef.h>

#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "berror.h"
#include "bobject.h"
#include "contiter.h"
#include "executor.h"
#include "impstr.h"
#include "objmethods.h"

namespace Pol
{
namespace Bscript
{
BStruct::BStruct() : BObjectImp( OTStruct ), contents_() {}

BStruct::BStruct( BObjectType type ) : BObjectImp( type ), contents_() {}

BStruct::BStruct( const BStruct& other, BObjectType type ) : BObjectImp( type ), contents_()
{
  for ( const auto& elem : other.contents_ )
  {
    const std::string& key = elem.first;
    const BObjectRef& bvalref = elem.second;

    contents_[key] = BObjectRef( new BObject( bvalref->impref().copy() ) );
  }
}

BStruct::BStruct( std::istream& is, unsigned size, BObjectType type )
    : BObjectImp( type ), contents_()
{
  for ( unsigned i = 0; i < size; ++i )
  {
    BObjectImp* keyimp = BObjectImp::unpack( is );
    BObjectImp* valimp = BObjectImp::unpack( is );
    if ( keyimp != nullptr && valimp != nullptr && keyimp->isa( OTString ) )
    {
      String* str = static_cast<String*>( keyimp );

      contents_[str->value()].set( new BObject( valimp ) );

      BObject cleaner( str );
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

BObjectImp* BStruct::copy() const
{
  passert( isa( OTStruct ) );
  return new BStruct( *this, OTStruct );
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
u8 BStruct::typeOfInt() const
{
  return OTStruct;
}


BObjectImp* BStruct::unpack( std::istream& is )
{
  unsigned size;
  char colon;
  if ( !( is >> size >> colon ) )
  {
    return new BError( "Unable to unpack struct elemcount" );
  }
  if ( (int)size < 0 )
  {
    return new BError(
        "Unable to unpack struct elemcount. Length given must be positive integer!" );
  }
  if ( colon != ':' )
  {
    return new BError( "Unable to unpack struct elemcount. Bad format. Colon not found!" );
  }
  return new BStruct( is, size, OTStruct );
}

void BStruct::FormatForStringRep( std::ostream& os, const std::string& key,
                                  const BObjectRef& bvalref ) const
{
  os << key << " = " << bvalref->impref().getFormattedStringRep();
}

class BStructIterator final : public ContIterator
{
public:
  BStructIterator( BStruct* pDict, BObject* pIterVal );
  virtual BObject* step() override;

private:
  BObject m_StructObj;
  BStruct* m_pStruct;
  BObjectRef m_IterVal;
  std::string key;
  bool m_First;
};
BStructIterator::BStructIterator( BStruct* pStruct, BObject* pIterVal )
    : m_StructObj( pStruct ),
      m_pStruct( pStruct ),
      m_IterVal( pIterVal ),
      key( "" ),
      m_First( true )
{
}

BObject* BStructIterator::step()
{
  if ( m_First )
  {
    auto itr = m_pStruct->contents_.begin();
    if ( itr == m_pStruct->contents_.end() )
      return nullptr;

    m_First = false;
    key = ( *itr ).first;
    m_IterVal->setimp( new String( key ) );

    BObjectRef& oref = ( *itr ).second;
    return oref.get();
  }
  else
  {
    auto itr = m_pStruct->contents_.find( key );
    if ( itr == m_pStruct->contents_.end() )
      return nullptr;
    ++itr;
    if ( itr == m_pStruct->contents_.end() )
      return nullptr;

    key = ( *itr ).first;
    m_IterVal->setimp( new String( key ) );

    BObjectRef& oref = ( *itr ).second;
    return oref.get();
  }
}

ContIterator* BStruct::createIterator( BObject* pIterVal )
{
  return new BStructIterator( this, pIterVal );
}

size_t BStruct::sizeEstimate() const
{
  return sizeof( BStruct ) +
         Clib::memsize( contents_, []( const auto& v ) { return v.sizeEstimate(); } );
}

size_t BStruct::mapcount() const
{
  return contents_.size();
}


BObjectRef BStruct::set_member( const char* membername, BObjectImp* value, bool copy )
{
  std::string key( membername );
  BObjectImp* target = copy ? value->copy() : value;
  auto itr = contents_.find( key );
  if ( itr != contents_.end() )
  {
    BObjectRef& oref = ( *itr ).second;
    oref->setimp( target );
    return oref;
  }
  else
  {
    BObjectRef ref( new BObject( target ) );
    contents_[key] = ref;
    return ref;
  }
}

// used programmatically
const BObjectImp* BStruct::FindMember( const char* name )
{
  std::string key( name );

  auto itr = contents_.find( key );
  if ( itr != contents_.end() )
  {
    return ( *itr ).second->impptr();
  }
  else
  {
    return nullptr;
  }
}

BObjectRef BStruct::get_member( const char* membername )
{
  std::string key( membername );

  auto itr = contents_.find( key );
  if ( itr != contents_.end() )
  {
    return ( *itr ).second;
  }
  else
  {
    return BObjectRef( UninitObject::create() );
  }
}

BObjectRef BStruct::OperSubscript( const BObject& obj )
{
  if ( obj->isa( OTString ) )
  {
    const String* keystr = static_cast<const String*>( obj.impptr() );

    auto itr = contents_.find( keystr->value() );
    if ( itr != contents_.end() )
    {
      BObjectRef& oref = ( *itr ).second;
      return oref;
    }
    else
    {
      return BObjectRef( UninitObject::create() );
    }
  }
  else if ( obj->isa( OTLong ) )
  {
    throw std::runtime_error( "Struct members cannot be accessed by an integer index" );
  }
  else
  {
    return BObjectRef( new BError( "Struct members can only be accessed by name" ) );
  }
}

BObjectImp* BStruct::array_assign( BObjectImp* idx, BObjectImp* target, bool copy )
{
  if ( idx->isa( OTString ) )
  {
    BObjectImp* new_target = copy ? target->copy() : target;

    String* keystr = static_cast<String*>( idx );

    auto itr = contents_.find( keystr->value() );
    if ( itr != contents_.end() )
    {
      BObjectRef& oref = ( *itr ).second;
      oref->setimp( new_target );
      return new_target;
    }
    else
    {
      contents_[keystr->value()].set( new BObject( new_target ) );
      return new_target;
    }
  }
  else if ( idx->isa( OTLong ) )
  {
    throw std::runtime_error(
        "some fool tried to use operator[] := on a struct, with an Integer index" );
  }
  else
  {
    return new BError( "Struct members can only be accessed by name" );
  }
}

void BStruct::addMember( const char* name, BObjectRef val )
{
  std::string key( name );
  contents_[key] = val;
}

void BStruct::addMember( const char* name, BObjectImp* imp )
{
  std::string key( name );
  contents_[key] = BObjectRef( imp );
}

BObjectImp* BStruct::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
{
  BObject* keyobj;
  BObject* valobj;
  switch ( id )
  {
  case MTH_SIZE:
    if ( ex.numParams() == 0 )
      return new BLong( static_cast<int>( contents_.size() ) );
    else
      return new BError( "struct.size() doesn't take parameters." );

  case MTH_ERASE:
    if ( ex.numParams() == 1 && ( keyobj = ex.getParamObj( 0 ) ) != nullptr )
    {
      if ( !keyobj->isa( OTString ) )
        return new BError( "Struct keys must be strings" );
      String* strkey = static_cast<String*>( keyobj->impptr() );
      int nremove = static_cast<int>( contents_.erase( strkey->value() ) );
      return new BLong( nremove );
    }
    else
    {
      return new BError( "struct.erase(key) requires a parameter." );
    }
    break;
  case MTH_INSERT:
    if ( ex.numParams() == 2 && ( keyobj = ex.getParamObj( 0 ) ) != nullptr &&
         ( valobj = ex.getParamObj( 1 ) ) != nullptr )
    {
      if ( !keyobj->isa( OTString ) )
        return new BError( "Struct keys must be strings" );
      String* strkey = static_cast<String*>( keyobj->impptr() );
      contents_[strkey->value()] = BObjectRef( new BObject( valobj->impptr()->copy() ) );
      return new BLong( static_cast<int>( contents_.size() ) );
    }
    else
    {
      return new BError( "struct.insert(key,value) requires two parameters." );
    }
    break;
  case MTH_EXISTS:
    if ( ex.numParams() == 1 && ( keyobj = ex.getParamObj( 0 ) ) != nullptr )
    {
      if ( !keyobj->isa( OTString ) )
        return new BError( "Struct keys must be strings" );
      String* strkey = static_cast<String*>( keyobj->impptr() );
      int count = static_cast<int>( contents_.count( strkey->value() ) );
      return new BLong( count );
    }
    else
    {
      return new BError( "struct.exists(key) requires a parameter." );
    }

  case MTH_KEYS:
    if ( ex.numParams() == 0 )
    {
      std::unique_ptr<ObjArray> arr( new ObjArray );
      for ( const auto& content : contents_ )
      {
        arr->addElement( new String( content.first ) );
      }
      return arr.release();
    }
    else
      return new BError( "struct.keys() doesn't take parameters." );
    break;
  default:
    return nullptr;
  }
}

BObjectImp* BStruct::call_method( const char* methodname, Executor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_method_id( objmethod->id, ex );
  else
    return nullptr;
}

void BStruct::packonto( std::ostream& os ) const
{
  os << packtype() << contents_.size() << ":";
  for ( const auto& content : contents_ )
  {
    const std::string& key = content.first;
    const BObjectRef& bvalref = content.second;

    String::packonto( os, key );
    bvalref->impref().packonto( os );
  }
}

std::string BStruct::getStringRep() const
{
  OSTRINGSTREAM os;
  os << typetag() << "{ ";
  bool any = false;

  for ( const auto& content : contents_ )
  {
    const std::string& key = content.first;
    const BObjectRef& bvalref = content.second;

    if ( any )
      os << ", ";
    else
      any = true;

    FormatForStringRep( os, key, bvalref );
  }

  os << " }";

  return OSTRINGSTREAM_STR( os );
}


BObjectRef BStruct::operDotPlus( const char* name )
{
  std::string key( name );
  if ( contents_.count( key ) == 0 )
  {
    auto pnewobj = new BObject( new UninitObject );
    contents_[key] = BObjectRef( pnewobj );
    return BObjectRef( pnewobj );
  }
  else
  {
    return BObjectRef( new BError( "Member already exists" ) );
  }
}

BObjectRef BStruct::operDotMinus( const char* name )
{
  std::string key( name );
  contents_.erase( key );
  return BObjectRef( new BLong( 1 ) );
}

BObjectRef BStruct::operDotQMark( const char* name )
{
  std::string key( name );
  int count = static_cast<int>( contents_.count( key ) );
  return BObjectRef( new BLong( count ) );
}

const BStruct::Contents& BStruct::contents() const
{
  return contents_;
}
}  // namespace Bscript
}  // namespace Pol
