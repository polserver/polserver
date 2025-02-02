/** @file
 *
 * @par History
 */

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <string>

#include "../../clib/logfacility.h"
#include "../../clib/rawtypes.h"
#include "../../plib/uoexpansion.h"
#include "testenv.h"

namespace Pol
{
namespace Testing
{

void uoextension_test()
{
  using namespace Plib;
  UnitTest( []() { return getExpansionName( getExpansionVersion( "T2A" ) ); }, "T2A",
            "t2a expansion" );
  UnitTest( []() { return getExpansionName( getExpansionVersion( "LBR" ) ); }, "LBR",
            "lbr expansion" );
  UnitTest( []() { return getExpansionName( getExpansionVersion( "AOS" ) ); }, "AOS",
            "aos expansion" );
  UnitTest( []() { return getExpansionName( getExpansionVersion( "SE" ) ); }, "SE",
            "se expansion" );
  UnitTest( []() { return getExpansionName( getExpansionVersion( "ML" ) ); }, "ML",
            "ml expansion" );
  UnitTest( []() { return getExpansionName( getExpansionVersion( "KR" ) ); }, "KR",
            "kr expansion" );
  UnitTest( []() { return getExpansionName( getExpansionVersion( "SA" ) ); }, "SA",
            "sa expansion" );
  UnitTest( []() { return getExpansionName( getExpansionVersion( "HSA" ) ); }, "HSA",
            "hsa expansion" );
  UnitTest( []() { return getExpansionName( getExpansionVersion( "TOL" ) ); }, "TOL",
            "tol expansion" );
  UnitTest( []() { return getExpansionName( getExpansionVersion( "unknown" ) ); }, "T2A",
            "unknown expansion" );
  {
    ServerExpansion server;
    server.updateFromPolCfg( 7 );
    server.updateFromSSOpt( A9Feature::AOS, "AOS", 0 );
    UnitTest(
        [&]()
        {
          server.updateFromSSOpt( A9Feature::AOS, "AOS", 0 );
          return server.supportsAOS();
        },
        true, "server: aos support true" );
    UnitTest(
        [&]()
        {
          server.updateFromSSOpt( A9Feature::None, "T2A", 0 );
          return server.supportsAOS();
        },
        false, "server: aos support false" );
    UnitTest(
        [&]()
        {
          server.updateFromSSOpt( A9Feature::None, "T2A", 0 );
          return server.faceSupport();
        },
        FaceSupport::None, "server: face support none" );
    UnitTest(
        [&]()
        {
          server.updateFromSSOpt( A9Feature::None, "T2A", 1 );
          return server.faceSupport();
        },
        FaceSupport::Basic, "server: face support basic" );
    UnitTest(
        [&]()
        {
          server.updateFromSSOpt( A9Feature::None, "T2A", 2 );
          return server.faceSupport();
        },
        FaceSupport::RolePlay, "server: face support roleplay" );
    UnitTest(
        [&]()
        {
          server.updateFromSSOpt( A9Feature::None, "T2A", 10 );
          return server.faceSupport();
        },
        FaceSupport::RolePlay, "server: face support roleplay clamp" );
  }
  {
    ServerExpansion server;
    server.updateFromPolCfg( 7 );
    UnitTest( [&]() { return AccountExpansion( "AOS", B9Feature::AOS ).getCharSlots( server ); }, 7,
              "account AOS chr slots 7" );
    UnitTest( [&]() { return AccountExpansion( "T2A", B9Feature::AOS ).getCharSlots( server ); }, 5,
              "account T2A chr slots 5" );
  }
  {
    ServerExpansion server;
    server.updateFromPolCfg( 7 );
    UnitTest(
        [&]()
        { return AccountExpansion( "AOS", B9Feature::T2A ).calculatedExtensionFlags( server ); },
        B9Feature::T2A | B9Feature::Has7thSlot, "account AOS chr slots 7 feature" );
    UnitTest(
        [&]()
        { return AccountExpansion( "T2A", B9Feature::T2A ).calculatedExtensionFlags( server ); },
        B9Feature::T2A, "account T2A chr slots 7 feature" );
    server.updateFromPolCfg( 6 );
    UnitTest(
        [&]()
        { return AccountExpansion( "AOS", B9Feature::T2A ).calculatedExtensionFlags( server ); },
        B9Feature::T2A | B9Feature::Has6thSlot, "account AOS chr slots 6 feature" );
    UnitTest(
        [&]()
        { return AccountExpansion( "T2A", B9Feature::T2A ).calculatedExtensionFlags( server ); },
        B9Feature::T2A, "account T2A chr slots 6 feature" );
    server.updateFromPolCfg( 5 );
    UnitTest(
        [&]()
        { return AccountExpansion( "AOS", B9Feature::T2A ).calculatedExtensionFlags( server ); },
        B9Feature::T2A, "account AOS chr slots 5 feature" );
    UnitTest(
        [&]()
        { return AccountExpansion( "T2A", B9Feature::T2A ).calculatedExtensionFlags( server ); },
        B9Feature::T2A, "account T2A chr slots 5 feature" );
  }
  {
    ServerExpansion server;
    server.updateFromSSOpt( A9Feature::AOS, "KR", 2 );
    UnitTest(
        [&]()
        { return AccountExpansion( "KR", B9Feature::T2A ).calculatedExtensionFlags( server ); },
        B9Feature::T2A | B9Feature::KRFaces, "account KR faces feature" );
  }
  {
    ServerExpansion server;
    server.updateFromSSOpt( A9Feature::AOS, "AOS", 0 );
    UnitTest( [&]()
              { return AccountExpansion( "AOS", B9Feature::T2A ).calculateFeatureFlags( server ); },
              A9Feature::AOS | A9Feature::UO3DClientType, "account A9Feature 5 chars" );
    UnitTest(
        [&]()
        {
          server.updateFromPolCfg( 7 );
          return AccountExpansion( "AOS", B9Feature::T2A ).calculateFeatureFlags( server );
        },
        A9Feature::AOS | A9Feature::UO3DClientType | A9Feature::Has7thSlot,
        "account A9Feature 7 chars" );
    UnitTest(
        [&]()
        {
          server.updateFromPolCfg( 6 );
          return AccountExpansion( "AOS", B9Feature::T2A ).calculateFeatureFlags( server );
        },
        A9Feature::AOS | A9Feature::UO3DClientType | A9Feature::Has6thSlot,
        "account A9Feature 6 chars" );
    UnitTest(
        [&]()
        {
          server.updateFromPolCfg( 1 );
          return AccountExpansion( "AOS", B9Feature::T2A ).calculateFeatureFlags( server );
        },
        A9Feature::AOS | A9Feature::UO3DClientType | A9Feature::SingleCharacter |
            A9Feature::LimitSlots,
        "account A9Feature 1 chars" );
  }
}
}  // namespace Testing
}  // namespace Pol
