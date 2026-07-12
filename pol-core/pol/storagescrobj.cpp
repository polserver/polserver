#include "storagescrobj.h"

#include "bscript/berror.h"
#include "bscript/blong.h"
#include "bscript/bstring.h"
#include "bscript/buninit.h"

#include "globals/uvars.h"
#include "item/item.h"
#include "mkscrobj.h"

namespace Pol::Core
{
using namespace Bscript;

StorageAreaIterator::StorageAreaIterator( StorageArea* area, BObject* pIter )
    : ContIterator(), m_pIterVal( pIter ), key( "" ), _area( area )
{
}

BObject* StorageAreaIterator::step()
{
  StorageArea::Cont::iterator itr = _area->_items.lower_bound( key );
  if ( !key.empty() && itr != _area->_items.end() )
  {
    ++itr;
  }

  if ( itr == _area->_items.end() )
    return nullptr;

  key = ( *itr ).first;
  m_pIterVal->setimp( new String( key ) );
  BObject* result = new BObject( make_itemref( ( *itr ).second ) );
  return result;
}

ContIterator* StorageAreaImp::createIterator( BObject* pIterVal )
{
  return new StorageAreaIterator( _area, pIterVal );
}

BObjectRef StorageAreaImp::get_member( const char* membername )
{
  if ( stricmp( membername, "count" ) == 0 )
  {
    return BObjectRef( new BLong( static_cast<int>( _area->_items.size() ) ) );
  }
  if ( stricmp( membername, "totalcount" ) == 0 )
  {
    unsigned int total = 0;
    for ( auto& _item : _area->_items )
    {
      Items::Item* item = _item.second;
      total += item->item_count();
    }
    return BObjectRef( new BLong( total ) );
  }
  return BObjectRef( UninitObject::create() );
}

StorageAreasIterator::StorageAreasIterator( BObject* pIter )
    : ContIterator(), m_pIterVal( pIter ), key( "" )
{
}

BObject* StorageAreasIterator::step()
{
  Storage::AreaCont::iterator itr = gamestate.storage.areas.lower_bound( key );
  if ( !key.empty() && itr != gamestate.storage.areas.end() )
  {
    ++itr;
  }

  if ( itr == gamestate.storage.areas.end() )
    return nullptr;

  key = ( *itr ).first;
  m_pIterVal->setimp( new String( key ) );
  BObject* result = new BObject( new StorageAreaImp( ( *itr ).second ) );
  return result;
}

ContIterator* StorageAreasImp::createIterator( BObject* pIterVal )
{
  return new StorageAreasIterator( pIterVal );
}

BObjectRef StorageAreasImp::get_member( const char* membername )
{
  if ( stricmp( membername, "count" ) == 0 )
  {
    return BObjectRef( new BLong( static_cast<int>( gamestate.storage.areas.size() ) ) );
  }
  return BObjectRef( UninitObject::create() );
}

BObjectRef StorageAreasImp::OperSubscript( const BObject& obj )
{
  if ( obj.isa( OTString ) )
  {
    String& rtstr = (String&)obj.impref();
    std::string key = rtstr.value();

    Storage::AreaCont::iterator itr = gamestate.storage.areas.find( key );
    if ( itr != gamestate.storage.areas.end() )
    {
      return BObjectRef( new BObject( new StorageAreaImp( ( *itr ).second ) ) );
    }

    return BObjectRef( new BObject( new BError( "Storage Area not found" ) ) );
  }
  return BObjectRef( new BObject( new BError( "Invalid parameter type" ) ) );
}

BObjectImp* CreateStorageAreasImp()
{
  return new StorageAreasImp();
}
}  // namespace Pol::Core
