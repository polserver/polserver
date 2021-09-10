/** @file
 *
 * @par History
 */


#ifndef H_LOS_H
#define H_LOS_H

#include "baseobject.h"

namespace Pol
{
namespace Core
{
class LosObj : public ULWObject
{
public:
  LosObj( u16 ix, u16 iy, s8 iz, Realms::Realm* irealm ) : ULWObject( UOBJ_CLASS::INVALID )
  {
    pos( Pos4d( ix, iy, iz, irealm ) );
  }
};
}  // namespace Core
}  // namespace Pol
#endif  // H_LOS_H
