/** @file
 *
 * @par History
 */


#include "realmdescriptor.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/strutil.h"


namespace Pol
{
namespace Plib
{
RealmDescriptor RealmDescriptor::Load( const std::string& realm_name,
                                       const std::string& realm_path )
{
  std::string realm_cfg_filename;
  if ( realm_path == "" )
    realm_cfg_filename = "realm/" + realm_name + "/realm.cfg";
  else
    realm_cfg_filename = realm_path + "/realm.cfg";

  Clib::ConfigFile cf( realm_cfg_filename, "REALM" );
  Clib::ConfigElem elem;
  if ( !cf.read( elem ) )
    elem.throw_error( "Unable to read configuration file " + realm_cfg_filename );

  return RealmDescriptor( realm_name, realm_path, elem );
}

namespace
{
unsigned short calc_grid_size( const unsigned size )
{
  unsigned grid_size = size / WGRID_SIZE;
  // Tokuno-Fix
  if ( grid_size * WGRID_SIZE < size )
    grid_size++;
  return static_cast<unsigned short>( grid_size );
}
}

RealmDescriptor::RealmDescriptor( const std::string& realm_name, const std::string& realm_path,
                                  Clib::ConfigElem& elem )
    : name( realm_name ),
      file_path( realm_path ),
      width( elem.remove_ushort( "width" ) ),
      height( elem.remove_ushort( "height" ) ),
      uomapid( elem.remove_unsigned( "uomapid", 0 ) ),
      uodif( elem.remove_bool( "uodif", false ) ),
      num_map_patches( elem.remove_unsigned( "num_map_patches", 0 ) ),
      num_static_patches( elem.remove_unsigned( "num_static_patches", 0 ) ),
      season( elem.remove_unsigned( "season", 1 ) ),
      mapserver_type( Clib::strlower( elem.remove_string( "mapserver", "memory" ) ) ),
      grid_width( calc_grid_size( width ) ),
      grid_height( calc_grid_size( height ) )
{
}

size_t RealmDescriptor::sizeEstimate() const
{
  size_t size = sizeof( *this );
  size += name.capacity() + file_path.capacity() + mapserver_type.capacity();
  return size;
}

std::string RealmDescriptor::path( const std::string& filename ) const
{
  if ( file_path == "" )
    return "realm/" + name + "/" + filename;
  else
    return file_path + "/" + filename;
}
}
}
