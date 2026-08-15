/** @file
 *
 * @par History
 */


#include "pol/testing/testenv.h"

#include "pol/skilladv.h"


namespace Pol::Testing
{
namespace
{
using Core::base_to_raw;
using Core::raw_to_base;

// raw is a base-2 logarithm of the displayed skill, so raw_to_base is lossy by design: a whole
// span of raw values shares one base. Only the base -> raw -> base direction can round trip,
// and only where base_to_raw runs its correction loop.

void test_known_conversions()
{
  // Powers of two along the chain, exact in both directions.
  const struct
  {
    unsigned int raw;
    unsigned short base;
  } known[] = { { 512, 25 },        { 1024, 50 },       { 2048, 100 },       { 3072, 150 },
                { 4096, 200 },      { 5120, 225 },      { 6144, 250 },       { 7168, 275 },
                { 8192, 300 },      { 16384, 400 },     { 32768, 500 },      { 65536, 600 },
                { 131072, 700 },    { 262144, 800 },    { 524288, 900 },     { 1048576, 1000 },
                { 2097152, 1100 },  { 4194304, 1200 },  { 8388608, 1300 },   { 16777216, 1400 },
                { 33554432, 1500 }, { 67108864, 1600 }, { 134217728, 1700 }, { 268435456, 1800 },
                { 536870912, 1900 } };

  UnitTest(
      [&]()
      {
        for ( const auto& k : known )
        {
          if ( raw_to_base( k.raw ) != k.base || base_to_raw( k.base ) != k.raw )
            return false;
        }
        return true;
      },
      true, "the power of two chain converts both ways" );
}

void test_base_roundtrip()
{
  // From 10.0 up, base_to_raw corrects itself until raw_to_base agrees, so every value returns.
  UnitTest(
      [&]()
      {
        for ( unsigned short base = 100; base <= 2100; ++base )
        {
          if ( raw_to_base( base_to_raw( base ) ) != base )
            return false;
        }
        return true;
      },
      true, "base 100..2100 survives a round trip" );

  // Below 10.0 base_to_raw returns base * 2048 / 100 without that correction, so only the
  // quarters land on a raw that converts back. Everything else comes back a tenth low.
  UnitTest(
      [&]()
      {
        for ( unsigned short base = 0; base < 100; ++base )
        {
          bool exact = ( base == 0 || base == 25 || base == 50 || base == 75 );
          unsigned short back = raw_to_base( base_to_raw( base ) );
          if ( exact ? ( back != base ) : ( back != base - 1 ) )
            return false;
        }
        return true;
      },
      true, "below base 100 only the quarters round trip, the rest lose a tenth" );
}

void test_monotonic()
{
  UnitTest(
      [&]()
      {
        unsigned short previous = 0;
        for ( unsigned int raw = 0; raw < 5000000; raw += 97 )
        {
          unsigned short base = raw_to_base( raw );
          if ( base < previous )
            return false;
          previous = base;
        }
        return true;
      },
      true, "raw_to_base never decreases as raw grows" );
}

void test_edges()
{
  UnitTest( []() { return raw_to_base( 0 ); }, static_cast<unsigned short>( 0 ),
            "raw 0 is base 0" );
  UnitTest( []() { return base_to_raw( 0 ); }, 0u, "base 0 is raw 0" );

  // A raw below 2048/100 has no tenth to show yet, which is why raw 20 is base 0 even though
  // base_to_raw( 1 ) hands back exactly 20.
  UnitTest( []() { return raw_to_base( 20 ); }, static_cast<unsigned short>( 0 ),
            "raw 20 rounds down to base 0" );
  UnitTest( []() { return base_to_raw( 1 ); }, 20u, "base 1 is raw 20" );

  // base_to_raw clamps its argument at 2100, so everything past it gives the same raw.
  UnitTest( []() { return base_to_raw( 2100 ); }, 2147483648u, "base 2100 is the top raw" );
  UnitTest( []() { return base_to_raw( 2101 ) == base_to_raw( 2100 ); }, true,
            "base past 2100 is clamped" );
  UnitTest( []() { return base_to_raw( 65535 ) == base_to_raw( 2100 ); }, true,
            "the largest base is clamped too" );

  // raw_to_base has no such ceiling and runs on past what base_to_raw can produce.
  UnitTest( []() { return raw_to_base( 0xFFFFFFFF ); }, static_cast<unsigned short>( 2199 ),
            "the largest raw is base 2199" );
}
}  // namespace

void skilladv_test()
{
  test_known_conversions();
  test_base_roundtrip();
  test_monotonic();
  test_edges();
}

}  // namespace Pol::Testing
