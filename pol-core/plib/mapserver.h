/*
History
=======

Notes
=======

*/

#ifndef PLIB_MAPSERVER_H
#define PLIB_MAPSERVER_H

#include "mapsolid.h"
#include "realmdescriptor.h"
namespace Pol {
  namespace Plib {
	struct MAPCELL;
	class MapShapeList;

	class MapServer
	{
	public:
	  static MapServer* Create( const RealmDescriptor& descriptor );

	  virtual ~MapServer();

	  virtual MAPCELL GetMapCell( unsigned short x, unsigned short y ) const = 0;
	  void GetMapShapes( MapShapeList& list, unsigned short x, unsigned short y, unsigned int anyflags ) const;
      virtual size_t memorySize( ) const;

	protected:
	  explicit MapServer( const RealmDescriptor& descriptor );

	  RealmDescriptor _descriptor;

	private:
	  // the indexes and shape data are always loaded into memory.
	  vector< SOLIDX2_ELEM* > _index1; // points into _index2
	  vector< SOLIDX2_ELEM > _index2;
	  vector< SOLIDS_ELEM > _shapedata;

	  void LoadSolids();
	  void LoadSecondLevelIndex();
	  void LoadFirstLevelIndex();

	  // not implemented:
	  MapServer& operator=( const MapServer& );
	  MapServer( const MapServer& );
	};
  }
}
#endif
