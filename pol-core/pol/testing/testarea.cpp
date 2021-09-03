
#include "clib/logfacility.h"
#include "clib/rawtypes.h"

#include "base/area.h"
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

void area2d_test()
{
  const Pos2d p1( 1, 2 );
  const Pos2d p2( 3, 4 );
  auto* r = gamestate.main_realm;
  UnitTest(
      [&]()
      {
        Area2d a( p1, p2, r );
        return a.nw() == p1 && a.se() == p2;
      },
      true, "area(1,2,3,4)" );
  UnitTest( [&]() { return Area2d( p1, p2, r ); }, Area2d( p1, p2, r ),
            "area(1,2,3,4)==(1,2,3,4)" );
  UnitTest( [&]() { return Area2d( p2, p1, r ); }, Area2d( p1, p2, r ),
            "area(3,4,1,2)==(1,2,3,4)" );
  UnitTest( [&]() { return Area2d( Pos4d( p1, 0, r ), Pos4d( p2, 0, r ) ); }, Area2d( p1, p2, r ),
            "area(p4(1,2,3,4))==(1,2,3,4)" );
  UnitTest( [&]() { return Area2d( p2, p1, r ) != Area2d( p1, p2, r ); }, false,
            "area(3,4,1,2)!=(1,2,3,4)" );

  UnitTest( [&]() { return Area2d( p2, p1, r ).contains( Pos2d( 1, 2 ) ); }, true,
            "contains(1,2)" );
  UnitTest( [&]() { return Area2d( p2, p1, r ).contains( Pos2d( 3, 5 ) ); }, false,
            "contains(3,5)" );

  UnitTest( [&]()
            { return Area2d( p2, p1, r ).intersect( Area2d( Pos2d( 0, 0 ), Pos2d( 2, 2 ), r ) ); },
            true, "intersect(0,0,2,2)" );
  UnitTest( [&]()
            { return Area2d( p2, p1, r ).intersect( Area2d( Pos2d( 3, 3 ), Pos2d( 4, 4 ), r ) ); },
            true, "intersect(3,3,4,4)" );
  UnitTest( [&]()
            { return Area2d( p2, p1, r ).intersect( Area2d( Pos2d( 4, 3 ), Pos2d( 4, 4 ), r ) ); },
            false, "intersect(4,3,4,4)" );
  UnitTest(
      [&]()
      {
        Area2d a( p1, p2, r );
        std::vector<Pos2d> areares;
        areares.assign( a.begin(), a.end() );
        std::vector<Pos2d> res{ { { 1, 2 },
                                  { 2, 2 },
                                  { 3, 2 },
                                  { 1, 3 },
                                  { 2, 3 },
                                  { 3, 3 },
                                  { 1, 4 },
                                  { 2, 4 },
                                  { 3, 4 } } };
        if ( areares != res )
        {
          INFO_PRINT << "size: " << areares.size() << " " << res.size() << "\n";
          for ( size_t i = 0; i < res.size(); ++i )
          {
            INFO_PRINT << res[i] << " " << areares[i] << "\n";
          }
          return false;
        }
        return true;
      },
      true, "itr" );

  UnitTest(
      [&]()
      {
        fmt::Writer tmp;
        tmp << Area2d( p1, p2, r );
        return true;
      },
      true, "" );
}

void area3d_test()
{
  const Pos3d p1( 1, 2, -5 );
  const Pos3d p2( 3, 4, 5 );
  auto* r = gamestate.main_realm;
  UnitTest(
      [&]()
      {
        Area3d a( p1, p2, r );
        return a.nw() == p1.xy() && a.se() == p2.xy() && a.nw_b() == p1 && a.se_t() == p2;
      },
      true, "area(1,2,3,4)" );
  UnitTest( [&]() { return Area3d( p1, p2, r ); }, Area3d( p1, p2, r ),
            "area(1,2,3,4)==(1,2,3,4)" );
  UnitTest( [&]() { return Area3d( p2, p1, r ); }, Area3d( p1, p2, r ),
            "area(3,4,1,2)==(1,2,3,4)" );
  UnitTest( [&]() { return Area3d( Pos4d( p1, r ), Pos4d( p2, r ) ); }, Area3d( p1, p2, r ),
            "area(p4(1,2,3,4))==(1,2,3,4)" );
  UnitTest( [&]() { return Area3d( p1, p2, r ).area(); }, Area2d( p1.xy(), p2.xy(), r ),
            "area(1,2,3,4)==2d(1,2,3,4)" );
  UnitTest( [&]() { return Area3d( p2, p1, r ) != Area3d( p1, p2, r ); }, false,
            "area(3,4,1,2)!=(1,2,3,4)" );

  UnitTest( [&]() { return Area3d( p2, p1, r ).contains( Pos2d( 1, 2 ) ); }, true,
            "contains(1,2)" );
  UnitTest( [&]() { return Area3d( p2, p1, r ).contains( Pos2d( 3, 5 ) ); }, false,
            "contains(3,5)" );
  UnitTest( [&]() { return Area3d( p2, p1, r ).contains( Pos3d( 1, 2, 3 ) ); }, true,
            "contains(1,2,3)" );
  UnitTest( [&]() { return Area3d( p2, p1, r ).contains( Pos3d( 1, 2, 10 ) ); }, false,
            "contains(1,2,10)" );

  UnitTest( [&]()
            { return Area3d( p2, p1, r ).intersect( Area2d( Pos2d( 0, 0 ), Pos2d( 2, 2 ), r ) ); },
            true, "intersect(0,0,2,2)" );
  UnitTest( [&]()
            { return Area3d( p2, p1, r ).intersect( Area2d( Pos2d( 3, 3 ), Pos2d( 4, 4 ), r ) ); },
            true, "intersect(3,3,4,4)" );
  UnitTest( [&]()
            { return Area3d( p2, p1, r ).intersect( Area2d( Pos2d( 4, 3 ), Pos2d( 4, 4 ), r ) ); },
            false, "intersect(4,3,4,4)" );
  UnitTest(
      [&]()
      { return Area3d( p2, p1, r ).intersect( Area3d( Pos3d( 0, 0, 0 ), Pos3d( 2, 2, 2 ), r ) ); },
      true, "intersect(0,0,0,2,2,2)" );
  UnitTest(
      [&]() {
        return Area3d( p2, p1, r ).intersect( Area3d( Pos3d( 3, 3, -10 ), Pos3d( 4, 4, 4 ), r ) );
      },
      true, "intersect(3,3,-10,4,4,4)" );
  UnitTest(
      [&]()
      { return Area3d( p2, p1, r ).intersect( Area3d( Pos3d( 3, 3, 4 ), Pos3d( 4, 4, 40 ), r ) ); },
      true, "intersect(3,3,4,4,4,40)" );
  UnitTest(
      [&]()
      { return Area3d( p2, p1, r ).intersect( Area3d( Pos3d( 4, 3, 0 ), Pos3d( 4, 4, 4 ), r ) ); },
      false, "intersect(4,3,0,4,4,4)" );

  UnitTest(
      [&]()
      {
        fmt::Writer tmp;
        tmp << Area3d( p1, p2, r );
        return true;
      },
      true, "" );
}
}  // namespace Testing
}  // namespace Pol
