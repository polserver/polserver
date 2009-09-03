/*
History
=======

2005/01/23 Shinigami: Realm::Con-/Destructor - Tokuno MapDimension doesn't fit blocks of 64x64 (WGRID_SIZE)
2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of is_shadowrealm, baserealm and shadowname

Notes
=======

*/

#include "clib/stl_inc.h"

#include "realm.h"
#include "realmdescriptor.h"
#include "mapserver.h"
#include "staticserver.h"
#include "maptileserver.h"
#include "pol/uworld.h"

Realm::Realm( const string& realm_name, const string& realm_path ) :
	is_shadowrealm(false),
	toplevel_item_count(0),
	mobile_count(0),
	_descriptor( RealmDescriptor::Load( realm_name, realm_path ) ),
	_mapserver( MapServer::Create( _descriptor ) ),
	_staticserver( new StaticServer( _descriptor ) ),
	_maptileserver( new MapTileServer( _descriptor ) )
{

	unsigned int gridwidth = width() / WGRID_SIZE;
	unsigned int gridheight = height() / WGRID_SIZE;

	// Tokuno-Fix
	if (gridwidth * WGRID_SIZE < width())
		gridwidth++;
	if (gridheight * WGRID_SIZE < height())
		gridheight++;

	zone = new Zone*[gridwidth];

	for(unsigned int i=0; i<gridwidth; i++)
		zone[i] = new Zone[gridheight];
}

Realm::Realm( const string& realm_name, Realm* realm ) :
	is_shadowrealm(true),
	baserealm(realm),
	shadowname(realm_name),
	toplevel_item_count(0),
	mobile_count(0),
	_descriptor( )
{
	unsigned int gridwidth = width() / WGRID_SIZE;
	unsigned int gridheight = height() / WGRID_SIZE;

	// Tokuno-Fix
	if (gridwidth * WGRID_SIZE < width())
		gridwidth++;
	if (gridheight * WGRID_SIZE < height())
		gridheight++;

	zone = new Zone*[gridwidth];

	for(unsigned int i=0; i<gridwidth; i++)
		zone[i] = new Zone[gridheight];
}

Realm::~Realm()
{
	unsigned int gridwidth = width() / WGRID_SIZE;

	// Tokuno-Fix
	if (gridwidth * WGRID_SIZE < width())
		gridwidth++;

	for(unsigned int i=0; i<gridwidth; i++)
		delete[] zone[i];
	delete[] zone;
}

unsigned Realm::width() const
{
	return _Descriptor().width;
}

unsigned Realm::height() const
{
    return _Descriptor().height;
}

unsigned Realm::season() const
{
    return _Descriptor().season;
}

bool Realm::valid( unsigned x, unsigned y, int z ) const
{
    return (x < _Descriptor().width && y < _Descriptor().height &&
            z >= -128 && z <= 127);
}

const string Realm::name() const
{
	if (is_shadowrealm)
		return shadowname;
	return _descriptor.name;
}
