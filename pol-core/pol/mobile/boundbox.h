/** @file
 *
 * @par History
 */


#ifndef __BOUNDBOX_H
#define __BOUNDBOX_H

#include "../base/position.h"
#include <vector>

namespace Pol
{
namespace Mobile
{
struct Area
{
  Core::Pos2d topleft;
  Core::Pos2d bottomright;
};

class BoundingBox
{
public:
  bool contains( const Core::Pos2d& pos ) const;
  void addarea( const Area& area );

private:
  typedef std::vector<Area> Areas;
  Areas areas;
};
}  // namespace Mobile
}  // namespace Pol
#endif  // BOUNDBOX_H
