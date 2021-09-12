
#include "clib/logfacility.h"
#include "clib/rawtypes.h"

#include "base/position.h"
#include "base/vector.h"
#include "globals/uvars.h"
#include "realms/realm.h"
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
  UnitTest( []() { return Pos2d( 2, 1 ) != Pos2d( 2, 1 ); }, false, "2,1!=2,1" );

  UnitTest( []() { return Pos2d( 2, 1 ).from_origin(); }, Vec2d( 2, 1 ), "2,1.from_origin()" );
  UnitTest( []() { return Pos2d( 2, 1 ).pol_distance( Pos2d( 0, 0 ) ); }, 2, "2,1.pol_distance()" );
  UnitTest( []() { return Pos2d( 2, 1 ).in_range( Pos2d( 0, 0 ), 2 ); }, true, "2,1.in_range()" );

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

  UnitTest(
      []()
      {
        Pos2d p( 0, 0 );
        p.move_self( FACING_S );
        return p;
      },
      Pos2d( 0, 1 ), "0,0.move_self(S)" );
  UnitTest(
      []()
      {
        std::vector<Vec2d> mod{ { { 0, -1 },
                                  { 1, -1 },
                                  { 1, 0 },
                                  { 1, 1 },
                                  { 0, 1 },
                                  { -1, 1 },
                                  { -1, 0 },
                                  { -1, -1 },
                                  { 0, 0 } } };
        std::vector<UFACING> res = { FACING_N,  FACING_NE, FACING_E,  FACING_SE, FACING_S,
                                     FACING_SW, FACING_W,  FACING_NW, FACING_N };
        const Pos2d p( 1, 2 );
        for ( size_t i = 0; i < mod.size(); ++i )
        {
          Pos2d tar = p + mod[i];
          UFACING facing = p.direction_toward( tar );
          if ( facing != res[i] )
          {
            INFO_PRINT << facing << " != " << res[i] << "\n";
            return false;
          }
        }
        return true;
      },
      true, "direction_toward" );
  UnitTest( []() { return Pos2d( 2, 1 ).direction_away( Pos2d( 4, 1 ) ); }, FACING_W,
            "2,1.direction_away(4,1)" );

  UnitTest(
      []()
      {
        fmt::Writer tmp;
        tmp << Pos2d( 0, 0 );
        return true;
      },
      true, "" );
}

void pos3d_test()
{
  Pos3d v;
  Pos3d v1( 1, 2, 3 );
  auto* realm = gamestate.main_realm;

  UnitTest( [&]() { return v.x() == 0 && v.y() == 0 && v.z() == 0; }, true, "init" );
  UnitTest( [&]() { return v1.x() == 1 && v1.y() == 2 && v1.z() == 3; }, true, "v1 init" );
  UnitTest( [&]() { return v1; }, Pos3d( 1, 2, 3 ), "v1 comp" );
  UnitTest( [&]() { return v1.xy(); }, Pos2d( 1, 2 ), "v1.xy comp" );
  UnitTest( [&]() { return Pos3d( Pos2d( 1, 2 ), 3 ); }, Pos3d( 1, 2, 3 ), "init pos2d" );

  UnitTest( [&]() { return v += Vec2d( 5, 6 ); }, Pos3d( 5, 6, 0 ), "+=" );
  UnitTest( [&]() { return v -= Vec2d( 5, 6 ); }, Pos3d( 0, 0, 0 ), "-=" );
  UnitTest( [&]() { return v += Vec3d( 5, 6, 3 ); }, Pos3d( 5, 6, 3 ), "+=" );
  UnitTest( [&]() { return v -= Vec3d( 5, 6, 3 ); }, Pos3d( 0, 0, 0 ), "-=" );

  UnitTest( []() { return Pos3d( 1, 2, 3 ) + Vec2d( 3, 4 ); }, Pos3d( 4, 6, 3 ), "+" );
  UnitTest( []() { return Pos3d( 1, 2, 3 ) - Vec2d( 3, 4 ); }, Pos3d( 0, 0, 3 ), "-" );
  UnitTest( []() { return Pos3d( 1, 2, 3 ) + Vec3d( 3, 4, 1 ); }, Pos3d( 4, 6, 4 ), "+" );
  UnitTest( []() { return Pos3d( 1, 2, 3 ) - Vec3d( 3, 4, 7 ); }, Pos3d( 0, 0, -4 ), "-" );

  UnitTest( []() { return Pos3d( 5, 2, 3 ) - Pos3d( 3, 4, 10 ); }, Vec3d( 2, -2, -7 ),
            "5,2,3-3,4,10" );
  UnitTest( []() { return Pos3d( 5, 2, 3 ) - Pos2d( 3, 4 ); }, Vec2d( 2, -2 ), "5,2,3-3,4" );
  UnitTest( []() { return Pos2d( 5, 2 ) - Pos3d( 3, 4, 1 ); }, Vec2d( 2, -2 ), "5,2-3,4,1" );

  UnitTest(
      [&]()
      {
        v.x( 10 ).y( 20 ).z( -10 );
        return v == Pos3d( 10, 20, -10 );
      },
      true, "setX,setY,setZ" );

  UnitTest(
      []()
      {
        Pos3d v2( 0xffff, 0, -128 );
        return v2 += Vec3d( 5, -5, -6 );
      },
      Pos3d( 0xffff, 0, -128 ), "+= clip" );

  UnitTest( []() { return Pos3d( 1, 1, 1 ) < Pos3d( 2, 1, 1 ); }, true, "1,1,1<2,1,1" );
  UnitTest( []() { return Pos3d( 1, 1, 1 ) < Pos3d( 1, 1, 1 ); }, false, "1,1,1<1,1,1" );
  UnitTest( []() { return Pos3d( 1, 1, 1 ) < Pos3d( 1, 2, 1 ); }, true, "1,1,1<1,2,1" );
  UnitTest( []() { return Pos3d( 1, 2, 1 ) > Pos3d( 1, 1, 1 ); }, true, "1,2,1>1,1,1" );
  UnitTest( []() { return Pos3d( 1, 1, 1 ) > Pos3d( 1, 1, 1 ); }, false, "1,1,1>1,1,1" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ) > Pos3d( 1, 1, 1 ); }, true, "2,1,1>1,1,1" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ) >= Pos3d( 2, 1, 1 ); }, true, "2,1,1>=2,1,1" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ) <= Pos3d( 2, 1, 1 ); }, true, "2,1,1<=2,1,1" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ) < Pos3d( 2, 1, 2 ); }, true, "2,1,1<2,1,2" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ) > Pos3d( 2, 1, -2 ); }, true, "2,1,1>2,1,-2" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ) != Pos3d( 2, 1, -2 ); }, true, "2,1,1!=2,1,-2" );

  UnitTest( []() { return Pos3d( 1, 1, 1 ) == Pos2d( 1, 1 ); }, true, "1,1,1<1,1" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ) != Pos2d( 1, 1 ); }, true, "2,1,1<1,1" );
  UnitTest( []() { return Pos3d( 1, 1, 1 ) < Pos2d( 2, 1 ); }, true, "1,1,1<2,1" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ) > Pos2d( 1, 1 ); }, true, "2,1,1<1,1" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ) >= Pos2d( 1, 1 ); }, true, "2,1,1>=1,1" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ) <= Pos2d( 3, 1 ); }, true, "2,1,1>=3,1" );

  UnitTest( []() { return Pos3d( 2, 1, -1 ).from_origin(); }, Vec3d( 2, 1, -1 ),
            "2,1,-1.from_origin()" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ).pol_distance( Pos3d( 0, 0, 0 ) ); }, 2,
            "2,1,1.pol_distance()" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ).in_range( Pos2d( 0, 0 ), 2 ); }, true,
            "2,1,1.in_range()" );
  UnitTest( []() { return Pos3d( 2, 1, 1 ).in_range( Pos3d( 10, 0, 0 ), 2 ); }, false,
            "2,1,1.in_range(10,0,0)" );

  UnitTest( [&]() { return Pos3d( realm->width() + 10, realm->height() + 10, 0 ).crop( realm ); },
            Pos3d( realm->width() - 1, realm->height() - 1, 0 ), "crop(realm)" );
  UnitTest( [&]() { return Pos3d( realm->width() + 10, realm->height() + 10, 0 ).crop( nullptr ); },
            Pos3d( realm->width() + 10, realm->height() + 10, 0 ), "crop(nullptr)" );

  UnitTest( []() { return Pos3d( 2, 1, 0 ).can_move_to( Vec2d( -2, -2 ), nullptr ); }, false,
            "2,1,0.can_move_to(-2,-2)" );
  UnitTest( []() { return Pos3d( 2, 1, 0 ).can_move_to( Vec2d( 2, 2 ), nullptr ); }, true,
            "2,1,0.can_move_to(2,2)" );
  UnitTest(
      [&]()
      {
        return Pos3d( realm->width() - 1, realm->height() - 1, 0 )
            .can_move_to( Vec2d( 2, 2 ), nullptr );
      },
      true, "realm.can_move_to(2,2,nullptr)" );
  UnitTest(
      [&]() {
        return Pos3d( realm->width() - 1, realm->height() - 1, 0 )
            .can_move_to( Vec2d( 2, 2 ), realm );
      },
      false, "realm.can_move_to(2,2,realm)" );
  UnitTest(
      []()
      {
        Pos3d p( 0, 0, 0 );
        p.move_self( FACING_S );
        return p;
      },
      Pos3d( 0, 1, 0 ), "0,0,0.move_self(S)" );

  UnitTest(
      []()
      {
        fmt::Writer tmp;
        tmp << Pos3d( 0, 0, 0 );
        return true;
      },
      true, "" );
}

void pos4d_test()
{
  auto* r = gamestate.main_realm;
  Pos4d v;
  Pos4d v1( 1, 2, 3, r );

  UnitTest( [&]() { return v.x() == 0 && v.y() == 0 && v.z() == 0 && v.realm() == nullptr; }, true,
            "init" );
  UnitTest( [&]() { return v1.x() == 1 && v1.y() == 2 && v1.z() == 3 && v1.realm() == r; }, true,
            "v1 init" );
  UnitTest( [&]() { return v1; }, Pos4d( 1, 2, 3, r ), "v1 comp" );
  UnitTest( [&]() { return v1.xy(); }, Pos2d( 1, 2 ), "v1.xy comp" );
  UnitTest( [&]() { return v1.xyz(); }, Pos3d( 1, 2, 3 ), "v1.xyz comp" );
  UnitTest( [&]() { return Pos4d( Pos2d( 1, 2 ), 3, r ); }, Pos4d( 1, 2, 3, r ), "init pos2d" );
  UnitTest( [&]() { return Pos4d( Pos3d( 1, 2, 3 ), r ); }, Pos4d( 1, 2, 3, r ), "init pos3d" );

  UnitTest( [&]() { return v += Vec2d( 5, 6 ); }, Pos4d( 5, 6, 0, nullptr ), "+=" );
  UnitTest( [&]() { return v -= Vec2d( 5, 6 ); }, Pos4d( 0, 0, 0, nullptr ), "-=" );
  UnitTest( [&]() { return v += Vec3d( 5, 6, 3 ); }, Pos4d( 5, 6, 3, nullptr ), "+=" );
  UnitTest( [&]() { return v -= Vec3d( 5, 6, 3 ); }, Pos4d( 0, 0, 0, nullptr ), "-=" );

  UnitTest( [&]() { return Pos4d( 1, 2, 3, r ) + Vec2d( 3, 4 ); }, Pos4d( 4, 6, 3, r ), "+" );
  UnitTest( [&]() { return Pos4d( 1, 2, 3, r ) - Vec2d( 3, 4 ); }, Pos4d( 0, 0, 3, r ), "-" );
  UnitTest( [&]() { return Pos4d( 1, 2, 3, r ) + Vec3d( 3, 4, 1 ); }, Pos4d( 4, 6, 4, r ), "+" );
  UnitTest( [&]() { return Pos4d( 1, 2, 3, r ) - Vec3d( 3, 4, 7 ); }, Pos4d( 0, 0, -4, r ), "-" );

  UnitTest( [&]() { return Pos4d( 5, 2, 3, r ) - Pos3d( 3, 4, 10 ); }, Vec3d( 2, -2, -7 ),
            "5,2,3,r-3,4,10" );
  UnitTest( [&]() { return Pos3d( 5, 2, 3 ) - Pos4d( 3, 4, 10, r ); }, Vec3d( 2, -2, -7 ),
            "5,2,3-3,4,10,r" );
  UnitTest( [&]() { return Pos4d( 5, 2, 3, r ) - Pos2d( 3, 4 ); }, Vec2d( 2, -2 ), "5,2,3-3,4" );
  UnitTest( [&]() { return Pos2d( 5, 2 ) - Pos4d( 3, 4, 1, r ); }, Vec2d( 2, -2 ), "5,2-3,4,1" );

  UnitTest(
      [&]()
      {
        v.x( 10 ).y( 20 ).z( -10 );
        return v == Pos4d( 10, 20, -10, nullptr );
      },
      true, "setX,setY,setZ" );

  UnitTest( [&]() { return Pos4d( 0xffff, 0xffff, -128, r ); },
            Pos4d( r->width() - 1, r->height() - 1, -128, r ), "init clip" );
  UnitTest( [&]() { return Pos4d( 0, 0, 0, r ).x( 0xffff ).y( 0xffff ); },
            Pos4d( r->width() - 1, r->height() - 1, 0, r ), "setX,setY crop" );


  UnitTest( [&]() { return Pos4d( 1, 1, 1, r ) < Pos4d( 2, 1, 1, r ); }, true, "1,1,1<2,1,1r" );
  UnitTest( [&]() { return Pos4d( 1, 1, 1, r ) < Pos4d( 1, 1, 1, r ); }, false, "1,1,1<1,1,1r" );
  UnitTest( [&]() { return Pos4d( 1, 1, 1, r ) < Pos4d( 1, 2, 1, r ); }, true, "1,1,1<1,2,1r" );
  UnitTest( [&]() { return Pos4d( 1, 2, 1, r ) > Pos4d( 1, 1, 1, r ); }, true, "1,2,1>1,1,1r" );
  UnitTest( [&]() { return Pos4d( 1, 1, 1, r ) > Pos4d( 1, 1, 1, r ); }, false, "1,1,1>1,1,1r" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) > Pos4d( 1, 1, 1, r ); }, true, "2,1,1>1,1,1r" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) >= Pos4d( 2, 1, 1, r ); }, true, "2,1,1>=2,1,1r" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) <= Pos4d( 2, 1, 1, r ); }, true, "2,1,1<=2,1,1r" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) < Pos4d( 2, 1, 2, r ); }, true, "2,1,1<2,1,2r" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) > Pos4d( 2, 1, -2, r ); }, true, "2,1,1>2,1,-2r" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) != Pos4d( 2, 1, -2, r ); }, true, "2,1,1!=2,1,-2r" );
  UnitTest( [&]() { return Pos4d( 1, 1, 1, r ) < Pos4d( 2, 1, 1, nullptr ); }, false,
            "1,1,1<2,1,1nullptr" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) > Pos4d( 1, 1, 1, nullptr ); }, false,
            "2,1,1<1,1,1nullptr" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) >= Pos4d( 2, 1, 1, nullptr ); }, false,
            "2,1,1>=2,1,1nullptr" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) <= Pos4d( 2, 1, 1, nullptr ); }, false,
            "2,1,1<=2,1,1nullptr" );

  UnitTest( [&]() { return Pos4d( 1, 1, 1, r ) < Pos3d( 2, 1, 1 ); }, true, "1,1,1<2,1,1" );
  UnitTest( [&]() { return Pos4d( 1, 1, 1, r ) < Pos3d( 1, 1, 1 ); }, false, "1,1,1<1,1,1" );
  UnitTest( [&]() { return Pos4d( 1, 1, 1, r ) < Pos3d( 1, 2, 1 ); }, true, "1,1,1<1,2,1" );
  UnitTest( [&]() { return Pos4d( 1, 2, 1, r ) > Pos3d( 1, 1, 1 ); }, true, "1,2,1>1,1,1" );
  UnitTest( [&]() { return Pos4d( 1, 1, 1, r ) > Pos3d( 1, 1, 1 ); }, false, "1,1,1>1,1,1" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) > Pos3d( 1, 1, 1 ); }, true, "2,1,1>1,1,1" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) >= Pos3d( 2, 1, 1 ); }, true, "2,1,1>=2,1,1" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) <= Pos3d( 2, 1, 1 ); }, true, "2,1,1<=2,1,1" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) < Pos3d( 2, 1, 2 ); }, true, "2,1,1<2,1,2" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) > Pos3d( 2, 1, -2 ); }, true, "2,1,1>2,1,-2" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) != Pos3d( 2, 1, -2 ); }, true, "2,1,1!=2,1,-2" );

  UnitTest( [&]() { return Pos4d( 1, 1, 1, r ) == Pos2d( 1, 1 ); }, true, "1,1,1<1,1" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) != Pos2d( 1, 1 ); }, true, "2,1,1<1,1" );
  UnitTest( [&]() { return Pos4d( 1, 1, 1, r ) < Pos2d( 2, 1 ); }, true, "1,1,1<2,1" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) > Pos2d( 1, 1 ); }, true, "2,1,1<1,1" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) >= Pos2d( 1, 1 ); }, true, "2,1,1>=1,1" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ) <= Pos2d( 3, 1 ); }, true, "2,1,1>=3,1" );

  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ).in_range( Pos2d( 0, 0 ), 2 ); }, true,
            "2,1,1.in_range()" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ).in_range( Pos3d( 10, 0, 0 ), 2 ); }, false,
            "2,1,1.in_range(10,0,0)" );
  UnitTest( [&]() { return Pos4d( 2, 1, 1, r ).in_range( Pos4d( 0, 0, 0, nullptr ), 2 ); }, false,
            "2,1,1.in_range(nullptr)" );

  UnitTest( [&]() { return Pos4d( 2, 1, 0, r ).can_move_to( Vec2d( -2, -2 ) ); }, false,
            "2,1,0.can_move_to(-2,-2)" );
  UnitTest( [&]() { return Pos4d( 2, 1, 0, r ).can_move_to( Vec2d( 2, 2 ) ); }, true,
            "2,1,0.can_move_to(2,2)" );
  UnitTest( [&]()
            { return Pos4d( r->width() - 1, r->height() - 1, 0, r ).can_move_to( Vec2d( 2, 2 ) ); },
            false, "width,height,0.can_move_to(2,2)" );

  UnitTest( [&]() { return Pos4d( 2, 1, 0, r ).pol_distance( Pos4d( 0, 0, 0, r ) ); }, 2,
            "2,1.pol_distance()" );
  UnitTest( [&]() { return Pos4d( 2, 1, 0, r ).pol_distance( Pos4d( 0, 0, 0, nullptr ) ); }, 0xffff,
            "2,1.pol_distance(nullptr)" );
  UnitTest( [&]() { return Pos4d( 0, 0, 0, r ).move( FACING_N ); }, Pos4d( 0, 0, 0, r ),
            "0,0,0.move(N)" );
  UnitTest( [&]() { return Pos4d( 0, 0, 0, r ).move( FACING_S ); }, Pos4d( 0, 1, 0, r ),
            "0,0,0.move(S)" );

  UnitTest(
      [&]()
      {
        Pos4d p( 0, 0, 0, r );
        p.move_self( FACING_S );
        return p;
      },
      Pos4d( 0, 1, 0, r ), "0,0,0.move_self(S)" );
  UnitTest(
      [&]()
      {
        fmt::Writer tmp;
        tmp << Pos4d( 0, 0, 0, r );
        return true;
      },
      true, "" );
}
}  // namespace Testing
}  // namespace Pol
