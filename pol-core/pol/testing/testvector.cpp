
#include "base/vector.h"
#include "clib/logfacility.h"
#include "clib/rawtypes.h"
#include "testenv.h"

namespace Pol
{
namespace Testing
{
using namespace Core;

void vector2d_test()
{
  Vec2d v;
  Vec2d v1( 1, 2 );
  UnitTest( [&]() { return v.x() == 0 && v.y() == 0; }, true, "init" );
  UnitTest( [&]() { return v1.x() == 1 && v1.y() == 2; }, true, "v1 init" );
  UnitTest( [&]() { return v1; }, Vec2d( 1, 2 ), "v1 comp" );

  UnitTest( [&]() { return v += Vec2d( 5, 6 ); }, Vec2d( 5, 6 ), "+=" );
  UnitTest( [&]() { return v -= Vec2d( 5, 6 ); }, Vec2d( 0, 0 ), "-=" );
  UnitTest( []() { return Vec2d( 1, 2 ) + Vec2d( 3, 4 ); }, Vec2d( 4, 6 ), "+" );
  UnitTest( []() { return Vec2d( 1, 2 ) - Vec2d( 3, 4 ); }, Vec2d( -2, -2 ), "-" );

  UnitTest(
      [&]()
      {
        v.x( 10 ).y( 20 );
        return v == Vec2d( 10, 20 );
      },
      true, "setX,setY" );

  UnitTest(
      []()
      {
        Vec2d v2( 0x7fff, -0x8000 );
        return v2 += Vec2d( 5, -5 );
      },
      Vec2d( 0x7fff, -0x8000 ), "+= clip" );

  UnitTest(
      []()
      {
        fmt::Writer tmp;
        tmp << Vec2d( 0, 0 );
        return true;
      },
      true, "" );
}

void vector3d_test()
{
  Vec3d v;
  Vec3d v1( 1, 2, 3 );
  UnitTest( [&]() { return v.x() == 0 && v.y() == 0 && v.z() == 0; }, true, "init" );
  UnitTest( [&]() { return v1.x() == 1 && v1.y() == 2 && v1.z() == 3; }, true, "v1 init" );
  UnitTest( [&]() { return v1; }, Vec3d( 1, 2, 3 ), "v1 comp" );
  UnitTest( []() { return Vec3d( Vec2d( 1, 2 ), 3 ); }, Vec3d( 1, 2, 3 ), "2d init" );

  UnitTest( [&]() { return v += Vec2d( 5, 6 ); }, Vec3d( 5, 6, 0 ), "+=" );
  UnitTest( [&]() { return v -= Vec2d( 5, 6 ); }, Vec3d( 0, 0, 0 ), "-=" );
  UnitTest( [&]() { return v += Vec3d( 5, 6, -3 ); }, Vec3d( 5, 6, -3 ), "+=Vec3d" );
  UnitTest( [&]() { return v -= Vec3d( 5, 6, -3 ); }, Vec3d( 0, 0, 0 ), "-=Vec3d" );
  UnitTest( []() { return Vec3d( 1, 2, 3 ) + Vec2d( 3, 4 ); }, Vec3d( 4, 6, 3 ), "+" );
  UnitTest( []() { return Vec3d( 1, 2, 3 ) - Vec2d( 3, 4 ); }, Vec3d( -2, -2, 3 ), "-" );
  UnitTest( []() { return Vec3d( 1, 2, 3 ) + Vec3d( 3, 4, 2 ); }, Vec3d( 4, 6, 5 ), "+Vec3d" );
  UnitTest( []() { return Vec3d( 1, 2, 3 ) - Vec3d( 3, 4, 2 ); }, Vec3d( -2, -2, 1 ), "-Vec3d" );

  UnitTest(
      [&]()
      {
        v.x( 10 ).y( 20 ).z( 5 );
        return v == Vec3d( 10, 20, 5 );
      },
      true, "setX,setY,setZ" );
  UnitTest(
      [&]()
      {
        v.xy( Vec2d( -10, -20 ) ).z( -5 );
        return v == Vec3d( -10, -20, -5 );
      },
      true, "setX,setY,setZ" );

  UnitTest(
      []()
      {
        Vec3d v2( 0x7fff, -0x8000, -0x8000 );
        return v2 += Vec3d( 5, -5, -10 );
      },
      Vec3d( 0x7fff, -0x8000, -0x8000 ), "+= clip" );
  UnitTest( []() { return Vec3d( 1, 1, 1 ) == Vec2d( 1, 1 ); }, true, "1,1,1==1,1" );
  UnitTest( []() { return Vec3d( 2, 1, 1 ).xy() == Vec2d( 2, 1 ); }, true, "2,1,1==1,1" );

  UnitTest( []() { return Vec3d( 1, 1, 1 ) != Vec3d( 1, 1, 1 ); }, false, "1,1,1!=1,1,1" );

  UnitTest( []() { return Vec3d( 1, 1, 1 ) != Vec2d( 1, 1 ); }, false, "1,1,1!=1,1" );

  UnitTest(
      []()
      {
        fmt::Writer tmp;
        tmp << Vec3d( 0, 0, 0 );
        return true;
      },
      true, "" );
}
}  // namespace Testing
}  // namespace Pol
