/** @file
 *
 * @par History
 */


#ifndef __BOUNDBOX_H
#define __BOUNDBOX_H

#include <vector>

namespace Pol
{
namespace Mobile
{
struct Point
{
  unsigned short x;
  unsigned short y;
};

struct Area
{
  Point topleft;
  Point bottomright;
};

class BoundingBox
{
public:
  bool contains( unsigned short x, unsigned short y ) const;
  void addarea( const Area& area );

private:
  typedef std::vector<Area> Areas;
  Areas areas;
};
}
}
#endif  // BOUNDBOX_H
