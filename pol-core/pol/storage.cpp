/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 */


#include "pol/storage.h"

#include <exception>
#include <string>
#include <time.h>

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/clib.h"
#include "clib/logfacility.h"
#include "clib/rawtypes.h"
#include "clib/stlutil.h"
#include "clib/streamsaver.h"
#include "plib/systemstate.h"
#include "pol/containr.h"
#include "pol/fnsearch.h"
#include "pol/globals/uvars.h"
#include "pol/item/item.h"
#include "pol/loaddata.h"
#include "pol/mkscrobj.h"
#include "pol/ufunc.h"


namespace Pol::Core
{
using namespace Bscript;

StorageArea::StorageArea( std::string name ) : _name( name ) {}

StorageArea::~StorageArea()
{
  while ( !_items.empty() )
  {
    Cont::iterator itr = _items.begin();
    Items::Item* item = ( *itr ).second;
    // The area is being torn down, so the item has nothing to be removed from; it must not go
    // looking for the map this loop is emptying.
    Items::abandon( *item );
    item->destroy();
    _items.erase( itr );
  }
}

size_t StorageArea::estimateSize() const
{
  size_t size = _name.capacity() + Clib::memsize( _items );
  return size;
}

Items::Item* StorageArea::find_root_item( const std::string& name )
{
  // LINEAR_SEARCH
  Cont::iterator itr = _items.find( name );
  if ( itr != _items.end() )
  {
    return ( *itr ).second;
  }
  return nullptr;
}

bool StorageArea::delete_root_item( const std::string& name )
{
  Cont::iterator itr = _items.find( name );
  if ( itr != _items.end() )
  {
    Items::Item* item = ( *itr ).second;
    // The area outlives the item, so the item really does leave it -- and taking it out is what
    // erases the entry, which is why nothing is erased here.
    Items::detach( *item );
    item->destroy();
    return true;
  }
  return false;
}

bool StorageArea::remove_root_item( const std::string& key, Items::Item* item )
{
  Cont::iterator itr = _items.find( key );
  if ( itr == _items.end() || ( *itr ).second != item )
    return false;

  _items.erase( itr );
  item->inuse( false );
  return true;
}

void StorageArea::insert_root_item( Items::Item* item )
{
  item->setposition( Pos4d() );
  item->inuse( true );
  _items.insert( make_pair( item->name(), item ) );
}

extern Items::Item* read_item( Clib::ConfigElem& elem );  // from UIMPORT.CPP

void StorageArea::load_item( Clib::ConfigElem& elem )
{
  u32 container_serial = 0;                                  // defaults to item at storage root,
  (void)elem.remove_prop( "CONTAINER", &container_serial );  // so the return value can be ignored
  // Where the item goes is the loader's business, so it reads the properties that say so rather
  // than leaving them on the item for somebody else to interpret.
  u8 saved_layer = static_cast<u8>( elem.remove_ushort( "LAYER", 0 ) );
  u8 saved_slot = static_cast<u8>( elem.remove_ushort( "SLOTINDEX", 0 ) );

  Items::Item* item = read_item( elem );
  // Austin added 8/10/2006, protect against further crash if item is null. Should throw instead?
  if ( item == nullptr )
  {
    elem.warn_with_line( "Error reading item SERIAL or OBJTYPE." );
    return;
  }
  if ( container_serial == 0 )
  {
    (void)Items::relocate_loaded( *item, Items::InStorage{ this, item->name() } );
  }
  else
  {
    Items::Item* cont_item = Core::system_find_item( container_serial );

    if ( cont_item )
    {
      add_loaded_item( cont_item, item, saved_layer, saved_slot );
    }
    else
    {
      defer_item_insertion( item, container_serial, saved_layer, saved_slot );
    }
  }
}
StorageArea* Storage::find_area( const std::string& name )
{
  AreaCont::iterator itr = areas.find( name );
  if ( itr == areas.end() )
    return nullptr;
  return ( *itr ).second;
}

StorageArea* Storage::create_area( const std::string& name )
{
  AreaCont::iterator itr = areas.find( name );
  if ( itr == areas.end() )
  {
    StorageArea* area = new StorageArea( name );
    areas[name] = area;
    return area;
  }

  return ( *itr ).second;
}

StorageArea* Storage::create_area( Clib::ConfigElem& elem )
{
  const char* rest = elem.rest();
  if ( rest != nullptr && rest[0] )
  {
    return create_area( rest );
  }

  std::string name = elem.remove_string( "NAME" );
  return create_area( name );
}


void StorageArea::print( Clib::StreamWriter& sw ) const
{
  for ( const auto& cont_item : _items )
  {
    const Items::Item* item = cont_item.second;
    if ( item->saveonexit() )
      item->printOn( sw );
  }
}

void StorageArea::for_each_root_item(
    const std::function<void( const std::string&, Items::Item* )>& f ) const
{
  for ( const auto& entry : _items )
    f( entry.first, entry.second );
}

void Storage::for_each_area( const std::function<void( StorageArea& )>& f ) const
{
  for ( const auto& area : areas )
    f( *area.second );
}

void Storage::read( Clib::ConfigFile& cf )
{
  static int num_until_dot = 1000;
  unsigned int nobjects = 0;

  StorageArea* area = nullptr;
  Clib::ConfigElem elem;

  clock_t start = clock();

  while ( cf.read( elem ) )
  {
    if ( --num_until_dot == 0 )
    {
      INFO_PRINT( "." );
      num_until_dot = 1000;
    }
    if ( elem.type_is( "StorageArea" ) )
    {
      area = create_area( elem );
    }
    else if ( elem.type_is( "Item" ) )
    {
      if ( area != nullptr )
      {
        try
        {
          area->load_item( elem );
        }
        catch ( std::exception& )
        {
          if ( !Plib::systemstate.config.ignore_load_errors )
            throw;
        }
      }
      else
      {
        ERROR_PRINTLN( "Storage: Got an ITEM element, but don't have a StorageArea to put it." );
        throw std::runtime_error( "Data file integrity error" );
      }
    }
    else
    {
      ERROR_PRINTLN( "Unexpected element type {} in storage file.", elem.type() );
      throw std::runtime_error( "Data file integrity error" );
    }
    ++nobjects;
  }

  clock_t end = clock();
  int ms = static_cast<int>( ( end - start ) * 1000.0 / CLOCKS_PER_SEC );

  INFO_PRINTLN( " {} elements in {} ms.", nobjects, ms );
}

void Storage::print( Clib::StreamWriter& sw ) const
{
  for ( const auto& area : areas )
  {
    sw.begin( "StorageArea" );
    sw.add( "Name", area.first );
    sw.end();
    area.second->print( sw );
  }
}

void Storage::clear()
{
  while ( !areas.empty() )
  {
    delete ( ( *areas.begin() ).second );
    areas.erase( areas.begin() );
  }
}

size_t Storage::estimateSize() const
{
  size_t size = Clib::memsize( areas );
  for ( const auto& area : areas )
  {
    if ( area.second != nullptr )
      size += area.second->estimateSize();
  }
  return size;
}
}  // namespace Pol::Core
