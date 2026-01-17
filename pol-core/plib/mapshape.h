/** @file
 *
 * @par History
 */


#ifndef PLIB_MAPSHAPE_H
#define PLIB_MAPSHAPE_H

#include <vector>


namespace Pol::Plib
{
struct MapShape
{
  short z;
  short height;
  unsigned int flags;
};

class MapShapeList : public std::vector<MapShape>
{
};
}  // namespace Pol::Plib

#endif
