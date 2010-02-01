/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/strutil.h"

#include "realmdescriptor.h"

RealmDescriptor RealmDescriptor::Load( const string& realm_name, const string& realm_path )
{
	string realm_cfg_filename;
	if ( realm_path == "" )
		realm_cfg_filename = "realm/"+realm_name+"/realm.cfg";
	else
		realm_cfg_filename = realm_path+"/realm.cfg";
	
	ConfigFile cf( realm_cfg_filename, "REALM" );
	ConfigElem elem;
	if (!cf.read( elem ))
		elem.throw_error( "Unable to read configuration file "+realm_cfg_filename );

	return RealmDescriptor( realm_name, realm_path, elem );
}

RealmDescriptor::RealmDescriptor( const string& realm_name, const string& realm_path, ConfigElem& elem ) :
	name(realm_name),
	file_path(realm_path),
	width(elem.remove_ushort( "width" )),
	height(elem.remove_ushort( "height" )),
	uomapid(elem.remove_unsigned( "uomapid", 0 )),
	uodif(elem.remove_bool( "uodif", false )),
	num_map_patches(elem.remove_unsigned( "num_map_patches", 0 )),
	num_static_patches(elem.remove_unsigned( "num_static_patches", 0 )),
	season(elem.remove_unsigned( "season", 1 )),
	mapserver_type(strlower( elem.remove_string( "mapserver", "memory" ) ))
{
}
RealmDescriptor::RealmDescriptor():
	name(""),
	file_path(""),
	width(0),
	height(0),
	uomapid(0),
	uodif(false),
	num_map_patches(0),
	num_static_patches(0),
	season(0),
	mapserver_type("")
{
}

string RealmDescriptor::path( const string& filename ) const
{
	if ( file_path == "" )
		return "realm/" + name + "/" + filename;
	else
		return file_path+"/"+filename;
}
