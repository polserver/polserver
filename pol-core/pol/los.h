/** @file
 *
 * @par History
 */


#ifndef H_LOS_H
#define H_LOS_H

#include "uobject.h"
namespace Pol
{
namespace Core
{

struct LosObj
{
  LosObj( const UObject& uobj ) : serial( uobj.serial ),
    x( uobj.x ), y( uobj.y ), z( uobj.z ),
    obj_height( uobj.height ),
    look_height( uobj.los_height() )
  {}
  LosObj( u16 x, u16 y, s8 z ) : serial( 0 ),
    x( x ),
    y( y ),
    z( z ),
    obj_height( 0 ),
    look_height( 0 )
  {}
  LosObj( u16 x, u16 y, s8 z, u8 obj_height ) : serial( 0 ),
    x( x ),
    y( y ),
    z( z ),
    obj_height( obj_height ),
    look_height( obj_height )
  {}
  u32 serial;
  u16 x;
  u16 y;
  s8 z;
  u8 obj_height;
  u8 look_height; // where you're looking from, or to
};
}
}
#endif // H_LOS_H
