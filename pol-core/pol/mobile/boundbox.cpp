/** @file
 *
 * @par History
 */


#include "boundbox.h"

namespace Pol
{
namespace Mobile
{
bool BoundingBox::contains( const Core::Pos2d& pos ) const
{
  for ( const auto& elem : areas )
  {
    if ( pos >= elem.topleft && pos <= elem.bottomright )
    {
      return true;
    }
  }
  return false;
}

void BoundingBox::addarea( const Area& area )
{
  areas.push_back( area );
}
}  // namespace Mobile
}  // namespace Pol
