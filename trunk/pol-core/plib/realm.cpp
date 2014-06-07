/*
History
=======

2005/01/23 Shinigami: Realm::Con-/Destructor - Tokuno MapDimension doesn't fit blocks of 64x64 (WGRID_SIZE)
2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of is_shadowrealm, baserealm and shadowname

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "realm.h"
#include "realmdescriptor.h"
#include "mapserver.h"
#include "staticserver.h"
#include "maptileserver.h"
#include "../pol/uworld.h"

namespace Pol {
  namespace Plib {
	Realm::Realm( const string& realm_name, const string& realm_path ) :
	  is_shadowrealm( false ),
	  toplevel_item_count( 0 ),
	  mobile_count( 0 ),
	  _descriptor( RealmDescriptor::Load( realm_name, realm_path ) ),
	  _mapserver( MapServer::Create( _descriptor ) ),
	  _staticserver( new StaticServer( _descriptor ) ),
	  _maptileserver( new MapTileServer( _descriptor ) )
	{

	  size_t gridwidth = width( ) / Core::WGRID_SIZE;
	  size_t gridheight = height( ) / Core::WGRID_SIZE;

	  // Tokuno-Fix
	  if ( gridwidth * Core::WGRID_SIZE < width( ) )
		gridwidth++;
	  if ( gridheight * Core::WGRID_SIZE < height( ) )
		gridheight++;

	  zone = new Core::Zone*[gridwidth];

	  for ( size_t i = 0; i < gridwidth; i++ )
		zone[i] = new Core::Zone[gridheight];
	}

	Realm::Realm( const string& realm_name, Realm* realm ) :
	  is_shadowrealm( true ),
	  baserealm( realm ),
	  shadowname( realm_name ),
	  toplevel_item_count( 0 ),
	  mobile_count( 0 ),
	  _descriptor()
	{
	  size_t gridwidth = width( ) / Core::WGRID_SIZE;
	  size_t gridheight = height( ) / Core::WGRID_SIZE;

	  // Tokuno-Fix
	  if ( gridwidth * Core::WGRID_SIZE < width( ) )
		gridwidth++;
	  if ( gridheight * Core::WGRID_SIZE < height( ) )
		gridheight++;

	  zone = new Core::Zone*[gridwidth];

	  for ( size_t i = 0; i < gridwidth; i++ )
		zone[i] = new Core::Zone[gridheight];
	}

	Realm::~Realm()
	{
	  size_t gridwidth = width() / Core::WGRID_SIZE;

	  // Tokuno-Fix
	  if ( gridwidth * Core::WGRID_SIZE < width( ) )
		gridwidth++;

	  for ( size_t i = 0; i < gridwidth; i++ )
		delete[] zone[i];
	  delete[] zone;
	}

    size_t Realm::sizeEstimate() const
    {
      size_t size = sizeof( *this );
      size += shadowname.capacity();
      // zone **
      unsigned int gridwidth = width() / Core::WGRID_SIZE;
      unsigned int gridheight = height() / Core::WGRID_SIZE;

      // Tokuno-Fix
      if ( gridwidth * Core::WGRID_SIZE < width() )
        gridwidth++;
      if ( gridheight * Core::WGRID_SIZE < height() )
        gridheight++;
      for ( unsigned x = 0; x < gridwidth; ++x )
      {
        for ( unsigned y = 0; y < gridheight; ++y )
        {
          size += 3 * sizeof(void**)+zone[x][y].characters.capacity() * sizeof( void* );
          size += 3 * sizeof(void**)+zone[x][y].npcs.capacity() * sizeof( void* );
          size += 3 * sizeof(void**)+zone[x][y].items.capacity() * sizeof( void* );
          size += 3 * sizeof(void**)+zone[x][y].multis.capacity() * sizeof( void* );
        }
      }

      // estimated set footprint
      size += 3 * sizeof(void*)+global_hulls.size( ) * ( sizeof(unsigned int)+3 * sizeof( void* ) );
      size += _descriptor.sizeEstimate()
        + ((!_mapserver) ? 0 : _mapserver->sizeEstimate())
        + ((!_staticserver) ? 0 : _staticserver->sizeEstimate())
        + ((!_maptileserver) ? 0 : _maptileserver->sizeEstimate());
      return size;
    }

	unsigned short Realm::width() const
	{
	  return _Descriptor().width;
	}

	unsigned short Realm::height() const
	{
	  return _Descriptor().height;
	}

	unsigned Realm::season() const
	{
	  return _Descriptor().season;
	}

	bool Realm::valid( unsigned short x, unsigned short y, short z ) const
	{
	  return ( x < _Descriptor().width && y < _Descriptor().height &&
			   z >= -128 && z <= 127 );
	}

	const string Realm::name() const
	{
	  if ( is_shadowrealm )
		return shadowname;
	  return _descriptor.name;
	}
  }
}