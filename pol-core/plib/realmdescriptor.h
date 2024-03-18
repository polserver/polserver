/** @file
 *
 * @par History
 */


#ifndef PLIB_REALMDESCRIPTOR_H
#define PLIB_REALMDESCRIPTOR_H

#include <cstddef>
#include <string>

namespace Pol
{
namespace Clib
{
class ConfigElem;
}
namespace Plib
{
const unsigned WGRID_SIZE = 64;
const unsigned WGRID_SHIFT = 6;

class RealmDescriptor
{
public:
  static RealmDescriptor Load( const std::string& realm_name, const std::string& realm_path = "" );
  static constexpr unsigned short VERSION = 1;

  std::string name;
  std::string file_path;
  unsigned short width;
  unsigned short height;
  unsigned uomapid;  // map[n].mul, need to be able to tell the client this.
  bool uodif;        // use the *dif files?
  unsigned num_map_patches;
  unsigned num_static_patches;
  unsigned season;
  std::string mapserver_type;  // "memory" or "file"
  unsigned short grid_width;
  unsigned short grid_height;
  unsigned short version;

  std::string path( const std::string& filename ) const;
  bool operator==( const RealmDescriptor& rdesc ) const
  {
    return ( name == rdesc.name && uomapid == rdesc.uomapid );
  }
  size_t sizeEstimate() const;

private:
  RealmDescriptor( const std::string& realm_name, const std::string& realm_path,
                   Clib::ConfigElem& elem );
};
}
}
#endif
