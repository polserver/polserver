/** @file
 *
 * @par History
 */


#ifndef PLIB_MAPSERVER_H
#define PLIB_MAPSERVER_H

#include <vector>

#include "mapsolid.h"
#include "realmdescriptor.h"


namespace Pol::Plib
{
class MapShapeList;
struct MAPCELL;

class MapServer
{
public:
  static MapServer* Create( const RealmDescriptor& descriptor );

  virtual ~MapServer() = default;

  virtual MAPCELL GetMapCell( unsigned short x, unsigned short y ) const = 0;
  void GetMapShapes( MapShapeList& list, unsigned short x, unsigned short y,
                     unsigned int anyflags ) const;
  virtual size_t sizeEstimate() const;

protected:
  explicit MapServer( const RealmDescriptor& descriptor );

  const RealmDescriptor _descriptor;

private:
  // the indexes and shape data are always loaded into memory.
  std::vector<SOLIDX2_ELEM*> _index1;  // points into _index2
  std::vector<SOLIDX2_ELEM> _index2;
  std::vector<SOLIDS_ELEM> _shapedata;

  void LoadSolids();
  void LoadSecondLevelIndex();
  void LoadFirstLevelIndex();

  // not implemented:
  MapServer& operator=( const MapServer& ) = delete;
  MapServer( const MapServer& ) = delete;
};
}  // namespace Pol::Plib

#endif
