
#include "../../clib/logfacility.h"
#include "../../clib/rawtypes.h"
#include "../base/position.h"
#include "../base/vector.h"
#include "../globals/uvars.h"
#include "../realms/realm.h"
#include "testenv.h"

namespace Pol
{
namespace Testing
{
using namespace Core;

void pos2d_test()
{
  Pos2d v;
  Pos2d v1( 1, 2 );
  auto* realm = gamestate.main_realm;

  UnitTest( [&]() { return v.x() == 0 && v.y() == 0; }, true, "init" );
  UnitTest( [&]() { return v1.x() == 1 && v1.y() == 2; }, true, "v1 init" );
  UnitTest( [&]() { return v1; }, Pos2d( 1, 2 ), "v1 comp" );

  UnitTest( [&]() { return v += Vec2d( 5, 6 ); }, Pos2d( 5, 6 ), "+=" );
  UnitTest( [&]() { return v -= Vec2d( 5, 6 ); }, Pos2d( 0, 0 ), "-=" );
  UnitTest( []() { return Pos2d( 1, 2 ) + Vec2d( 3, 4 ); }, Pos2d( 4, 6 ), "+" );
  UnitTest( []() { return Pos2d( 1, 2 ) - Vec2d( 3, 4 ); }, Pos2d( 0, 0 ), "-" );

  UnitTest(
      [&]()
      {
        v.x( 10 ).y( 20 );
        return v == Pos2d( 10, 20 );
      },
      true, "setX,setY" );

  UnitTest(
      []()
      {
        Pos2d v2( 0xffff, 0 );
        return v2 += Vec2d( 5, -5 );
      },
      Pos2d( 0xffff, 0 ), "+= clip" );

  UnitTest( []() { return Pos2d( 1, 1 ) < Pos2d( 2, 1 ); }, true, "1,1<2,1" );
  UnitTest( []() { return Pos2d( 1, 1 ) < Pos2d( 1, 1 ); }, false, "1,1<1,1" );
  UnitTest( []() { return Pos2d( 1, 1 ) < Pos2d( 1, 2 ); }, true, "1,1<1,2" );
  UnitTest( []() { return Pos2d( 1, 2 ) > Pos2d( 1, 1 ); }, true, "1,2>1,1" );
  UnitTest( []() { return Pos2d( 1, 1 ) > Pos2d( 1, 1 ); }, false, "1,1>1,1" );
  UnitTest( []() { return Pos2d( 2, 1 ) > Pos2d( 1, 1 ); }, true, "2,1>1,1" );
  UnitTest( []() { return Pos2d( 2, 1 ) >= Pos2d( 2, 1 ); }, true, "2,1>=2,1" );
  UnitTest( []() { return Pos2d( 2, 1 ) <= Pos2d( 2, 1 ); }, true, "2,1<=2,1" );

  UnitTest( []() { return Pos2d( 2, 1 ).from_origin(); }, Vec2d( 2, 1 ), "2,1.from_origin()" );
  UnitTest( []() { return Pos2d( 2, 1 ).pol_distance( Pos2d( 0, 0 ) ); }, 2, "2,1.pol_distance()" );
  UnitTest( []() { return Pos2d( 2, 1 ).inRange( Pos2d( 0, 0 ), 2 ); }, true, "2,1.inRange()" );

  UnitTest( [&]() { return Pos2d( realm->width() + 10, realm->height() + 10 ).crop( realm ); },
            Pos2d( realm->width() - 1, realm->height() - 1 ), "crop(realm)" );
  UnitTest( [&]() { return Pos2d( realm->width() + 10, realm->height() + 10 ).crop( nullptr ); },
            Pos2d( realm->width() + 10, realm->height() + 10 ), "crop(nullptr)" );

  UnitTest( []() { return Pos2d( 2, 1 ).can_move_to( Vec2d( -2, -2 ), nullptr ); }, false,
            "2,1.can_move_to(-2,-2)" );
  UnitTest( []() { return Pos2d( 2, 1 ).can_move_to( Vec2d( 2, 2 ), nullptr ); }, true,
            "2,1.can_move_to(2,2)" );
  UnitTest(
      [&]() {
        return Pos2d( realm->width() - 1, realm->height() - 1 )
            .can_move_to( Vec2d( 2, 2 ), nullptr );
      },
      true, "realm.can_move_to(2,2,nullptr)" );
  UnitTest(
      [&]() {
        return Pos2d( realm->width() - 1, realm->height() - 1 ).can_move_to( Vec2d( 2, 2 ), realm );
      },
      false, "realm.can_move_to(2,2,realm)" );

  UnitTest( []() { return Pos2d( 2, 1 ).direction_toward( Pos2d( 0, 0 ) ); }, FACING_NW,
            "2,1.direction_toward(0,0)" );
  UnitTest( []() { return Pos2d( 2, 1 ).direction_toward( Pos2d( 4, 1 ) ); }, FACING_E,
            "2,1.direction_toward(4,1)" );
  UnitTest( []() { return Pos2d( 2, 1 ).direction_away( Pos2d( 4, 1 ) ); }, FACING_W,
            "2,1.direction_away(4,1)" );
}
}  // namespace Testing
}  // namespace Pol
