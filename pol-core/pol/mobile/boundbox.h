/** @file
 *
 * @par History
 */


#ifndef __BOUNDBOX_H
#define __BOUNDBOX_H

#include "base/range.h"
#include <vector>


namespace Pol::Mobile
{
class BoundingBox
{
public:
  bool contains( const Core::Pos2d& pos ) const;
  void addarea( Core::Range2d area );

private:
  using Areas = std::vector<Core::Range2d>;
  Areas areas;
};
}  // namespace Pol::Mobile

#endif  // BOUNDBOX_H
