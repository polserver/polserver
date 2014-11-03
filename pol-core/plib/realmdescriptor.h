/*
History
=======

Notes
=======

*/

#ifndef PLIB_REALMDESCRIPTOR_H
#define PLIB_REALMDESCRIPTOR_H

#include <string>
#include <cstddef>

namespace Pol {
  namespace Clib {
	class ConfigElem;
  }
  namespace Plib {

	class RealmDescriptor
	{
	public:
	  static RealmDescriptor Load( const std::string& realm_name, const std::string& realm_path = "" );
	  RealmDescriptor();
	  // default copy ctor and operator= are okay.

	  const std::string name;
	  const std::string file_path;
	  const unsigned short width;
	  const unsigned short height;
	  const unsigned uomapid;         // map[n].mul, need to be able to tell the client this.
	  const bool uodif;               // use the *dif files?
	  const unsigned num_map_patches;
	  const unsigned num_static_patches;
	  const unsigned season;
	  const std::string mapserver_type;    // "memory" or "file"

	  std::string path( const std::string& filename ) const;
	  bool operator==( const RealmDescriptor& rdesc ) const
	  {
		return ( name == rdesc.name && uomapid == rdesc.uomapid );
	  }
	  RealmDescriptor & operator=( const RealmDescriptor & ) { return *this; }
      size_t sizeEstimate() const;
	private:
	  RealmDescriptor( const std::string& realm_name, const std::string& realm_path, Clib::ConfigElem& elem );
	};
  }
}
#endif
