/** @file
 *
 * @par History
 */


#ifndef __BOUNDBOX_H
#define __BOUNDBOX_H

#include "base/range.h"
#include <vector>

namespace Pol
{
namespace Mobile
{
class BoundingBox
{
public:
  bool contains( const Core::Pos2d& pos ) const;
  void addarea( Core::Range2d area );

private:
  typedef std::vector<Core::Range2d> Areas;
  Areas areas;
};
}  // namespace Mobile
}  // namespace Pol
#endif  // BOUNDBOX_H
