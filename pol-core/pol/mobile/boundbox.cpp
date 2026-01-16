/** @file
 *
 * @par History
 */


#include "boundbox.h"


namespace Pol::Mobile
{
bool BoundingBox::contains( const Core::Pos2d& pos ) const
{
  for ( const auto& elem : areas )
  {
    if ( elem.contains( pos ) )
      return true;
  }
  return false;
}

void BoundingBox::addarea( Core::Range2d area )
{
  areas.emplace_back( std::move( area ) );
}
}  // namespace Pol::Mobile
