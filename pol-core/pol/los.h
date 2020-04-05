/** @file
 *
 * @par History
 */


#ifndef H_LOS_H
#define H_LOS_H

#include "base/position.h"
#include "baseobject.h"

#include <utility>

namespace Pol
{
namespace Core
{
// TODO: Pos4d should be enough, remove it
class LosObj : public ULWObject
{
public:
  LosObj( u16 ix, u16 iy, s8 iz, Realms::Realm* irealm ) : ULWObject( UOBJ_CLASS::INVALID )
  {
    pos( Pos4d( ix, iy, iz, irealm ) );
  }
  LosObj( Pos4d p ) : ULWObject( UOBJ_CLASS::INVALID ) { pos( std::move( p ) ); }
};
}  // namespace Core
}  // namespace Pol
#endif  // H_LOS_H
