
#include "clib/logfacility.h"
#include "clib/rawtypes.h"

#include "base/position.h"
#include "base/range.h"
#include "base/vector.h"
#include "globals/uvars.h"
#include "realms/realm.h"
#include "testenv.h"

namespace Pol
{
namespace Testing
{
using namespace Core;

void range2d_test()
{
  const Pos2d p1( 1, 2 );
  const Pos2d p2( 3, 4 );
  auto* r = gamestate.main_realm;
  UnitTest(
      [&]()
      {
        Range2d a( p1, p2, r );
        return a.nw() == p1 && a.se() == p2;
      },
      true, "range(1,2,3,4)" );
  UnitTest( [&]() { return Range2d( p1, p2, r ); }, Range2d( p1, p2, r ),
            "range(1,2,3,4)==(1,2,3,4)" );
  UnitTest( [&]() { return Range2d( p2, p1, r ); }, Range2d( p1, p2, r ),
            "range(3,4,1,2)==(1,2,3,4)" );
  UnitTest( [&]() { return Range2d( Pos4d( p1, 0, r ), Pos4d( p2, 0, r ) ); }, Range2d( p1, p2, r ),
            "range(p4(1,2,3,4))==(1,2,3,4)" );
  UnitTest( [&]() { return Range2d( p2, p1, r ) != Range2d( p1, p2, r ); }, false,
            "range(3,4,1,2)!=(1,2,3,4)" );

  UnitTest( [&]() { return Range2d( p2, p1, r ).contains( Pos2d( 1, 2 ) ); }, true,
            "contains(1,2)" );
  UnitTest( [&]() { return Range2d( p2, p1, r ).contains( Pos2d( 3, 5 ) ); }, false,
            "contains(3,5)" );
  UnitTest( [&]()
            { return Range2d( Pos2d( 1, 1 ), Pos2d( 2, 2 ), nullptr ).contains( Pos2d( 1, 0 ) ); },
            false, "1,1,2,2.contains(1,0)" );
  UnitTest( [&]()
            { return Range2d( Pos2d( 1, 1 ), Pos2d( 2, 2 ), nullptr ).contains( Pos2d( 0, 1 ) ); },
            false, "1,1,2,2.contains(0,1)" );
  UnitTest( [&]()
            { return Range2d( Pos2d( 1, 1 ), Pos2d( 2, 2 ), nullptr ).contains( Pos2d( 2, 0 ) ); },
            false, "1,1,2,2.contains(2,0)" );
  UnitTest( [&]()
            { return Range2d( Pos2d( 1, 1 ), Pos2d( 2, 2 ), nullptr ).contains( Pos2d( 2, 3 ) ); },
            false, "1,1,2,2.contains(2,3)" );
  UnitTest( [&]()
            { return Range2d( Pos2d( 1, 1 ), Pos2d( 2, 2 ), nullptr ).contains( Pos2d( 3, 1 ) ); },
            false, "1,1,2,2.contains(3,1)" );

  UnitTest(
      [&]()
      { return Range2d( p2, p1, r ).intersect( Range2d( Pos2d( 0, 0 ), Pos2d( 2, 2 ), r ) ); },
      true, "intersect(0,0,2,2)" );
  UnitTest(
      [&]()
      { return Range2d( p2, p1, r ).intersect( Range2d( Pos2d( 3, 3 ), Pos2d( 4, 4 ), r ) ); },
      true, "intersect(3,3,4,4)" );
  UnitTest(
      [&]()
      { return Range2d( p2, p1, r ).intersect( Range2d( Pos2d( 4, 3 ), Pos2d( 4, 4 ), r ) ); },
      false, "intersect(4,3,4,4)" );
  UnitTest(
      [&]()
      {
        Range2d a( p1, p2, r );
        std::vector<Pos2d> rangeres;
        rangeres.assign( a.begin(), a.end() );
        std::vector<Pos2d> res{ { { 1, 2 },
                                  { 2, 2 },
                                  { 3, 2 },
                                  { 1, 3 },
                                  { 2, 3 },
                                  { 3, 3 },
                                  { 1, 4 },
                                  { 2, 4 },
                                  { 3, 4 } } };
        if ( rangeres != res )
        {
          INFO_PRINT << "size: " << rangeres.size() << " " << res.size() << "\n";
          for ( size_t i = 0; i < res.size(); ++i )
          {
            INFO_PRINT << res[i] << " " << rangeres[i] << "\n";
          }
          return false;
        }
        return true;
      },
      true, "itr" );

  UnitTest( [&]() { return fmt::format( "{:->25}", Range2d( p1, p2, r ) ); },
            "--( ( 1, 2 ) - ( 3, 4 ) )", "format padding" );

  UnitTest( [&]() { return Range2d( Pos4d( 2, 2, 0, r ), 1 ); },
            Range2d( Pos4d( 1, 1, 0, r ), Pos4d( 3, 3, 0, r ) ), "range(1,1,3,3)" );
  UnitTest( [&]() { return Range2d( Pos4d( 2, 2, 0, r ), 0 ); },
            Range2d( Pos4d( 2, 2, 0, r ), Pos4d( 2, 2, 0, r ) ), "range(2,2,2,2)" );

  UnitTest(
      [&]()
      {
        auto r1 = Range2d( Pos4d( 2, 2, 0, r ), 0 );
        auto r2 = Range2d( Pos4d( 2, 2, 0, r ), Pos4d( 5, 5, 0, r ) );
        return r1.intersect( r2 );
      },
      true, "(2,2,2,2).intersect(3,3,5,5)" );
}

void range3d_test()
{
  const Pos3d p1( 1, 2, -5 );
  const Pos3d p2( 3, 4, 5 );
  auto* r = gamestate.main_realm;
  UnitTest(
      [&]()
      {
        Range3d a( p1, p2, r );
        return a.nw() == p1.xy() && a.se() == p2.xy() && a.nw_b() == p1 && a.se_t() == p2;
      },
      true, "range(1,2,3,4)" );
  UnitTest( [&]() { return Range3d( p1, p2, r ); }, Range3d( p1, p2, r ),
            "range(1,2,3,4)==(1,2,3,4)" );
  UnitTest( [&]() { return Range3d( p2, p1, r ); }, Range3d( p1, p2, r ),
            "range(3,4,1,2)==(1,2,3,4)" );
  UnitTest( [&]() { return Range3d( Pos4d( p1, r ), Pos4d( p2, r ) ); }, Range3d( p1, p2, r ),
            "range(p4(1,2,3,4))==(1,2,3,4)" );
  UnitTest( [&]() { return Range3d( p1, p2, r ).range(); }, Range2d( p1.xy(), p2.xy(), r ),
            "range(1,2,3,4)==2d(1,2,3,4)" );
  UnitTest( [&]() { return Range3d( p2, p1, r ) != Range3d( p1, p2, r ); }, false,
            "range(3,4,1,2)!=(1,2,3,4)" );

  UnitTest( [&]() { return Range3d( p2, p1, r ).contains( Pos2d( 1, 2 ) ); }, true,
            "contains(1,2)" );
  UnitTest( [&]() { return Range3d( p2, p1, r ).contains( Pos2d( 3, 5 ) ); }, false,
            "contains(3,5)" );
  UnitTest( [&]() { return Range3d( p2, p1, r ).contains( Pos3d( 1, 2, 3 ) ); }, true,
            "contains(1,2,3)" );
  UnitTest( [&]() { return Range3d( p2, p1, r ).contains( Pos3d( 1, 2, 10 ) ); }, false,
            "contains(1,2,10)" );

  UnitTest(
      [&]()
      { return Range3d( p2, p1, r ).intersect( Range2d( Pos2d( 0, 0 ), Pos2d( 2, 2 ), r ) ); },
      true, "intersect(0,0,2,2)" );
  UnitTest(
      [&]()
      { return Range3d( p2, p1, r ).intersect( Range2d( Pos2d( 3, 3 ), Pos2d( 4, 4 ), r ) ); },
      true, "intersect(3,3,4,4)" );
  UnitTest(
      [&]()
      { return Range3d( p2, p1, r ).intersect( Range2d( Pos2d( 4, 3 ), Pos2d( 4, 4 ), r ) ); },
      false, "intersect(4,3,4,4)" );
  UnitTest(
      [&]() {
        return Range3d( p2, p1, r ).intersect( Range3d( Pos3d( 0, 0, 0 ), Pos3d( 2, 2, 2 ), r ) );
      },
      true, "intersect(0,0,0,2,2,2)" );
  UnitTest(
      [&]() {
        return Range3d( p2, p1, r ).intersect( Range3d( Pos3d( 3, 3, -10 ), Pos3d( 4, 4, 4 ), r ) );
      },
      true, "intersect(3,3,-10,4,4,4)" );
  UnitTest(
      [&]() {
        return Range3d( p2, p1, r ).intersect( Range3d( Pos3d( 3, 3, 4 ), Pos3d( 4, 4, 40 ), r ) );
      },
      true, "intersect(3,3,4,4,4,40)" );
  UnitTest(
      [&]() {
        return Range3d( p2, p1, r ).intersect( Range3d( Pos3d( 4, 3, 0 ), Pos3d( 4, 4, 4 ), r ) );
      },
      false, "intersect(4,3,0,4,4,4)" );

  UnitTest( [&]() { return fmt::format( "{:->32}", Range3d( p1, p2, r ) ); },
            "--( ( 1, 2, -5 ) - ( 3, 4, 5 ) )", "format padding" );
}
}  // namespace Testing
}  // namespace Pol
