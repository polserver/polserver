/** @file
 *
 * @par History
 */

#include "pol/testing/poltest.h"

#include "pol/testing/testenv.h"

#include <pol_global_config.h>

#ifdef ENABLE_BENCHMARK
#include <benchmark/benchmark.h>
#endif


namespace Pol::Testing
{
#define RUNTEST( t )               \
  INFO_PRINTLN( " - {}", ( #t ) ); \
  t();

bool run_pol_tests()
{
#ifdef ENABLE_BENCHMARK
  benchmark::RunSpecifiedBenchmarks();
  return true;
#endif
  RUNTEST( test_splitnamevalue )
  RUNTEST( test_convertquotedstring )
  RUNTEST( test_sanitizeUnicodeWithIso )
  RUNTEST( test_encodingconversions )

  RUNTEST( skilladv_test )
  RUNTEST( dynprops_test )
  RUNTEST( packet_test )
  RUNTEST( vector2d_test )
  RUNTEST( vector3d_test )
  RUNTEST( pos2d_test )
  RUNTEST( pos3d_test )
  RUNTEST( pos4d_test )
  RUNTEST( range2d_test )
  RUNTEST( range3d_test )

  RUNTEST( test_curlfeatures )

  RUNTEST( decay_test )
  RUNTEST( location_test )
  RUNTEST( zone_bookkeeping_test )
  RUNTEST( place_at_test )
  RUNTEST( container_slot_test )
  RUNTEST( clamp_test )
  RUNTEST( crypt_test )
  RUNTEST( webscript_test )
  RUNTEST( maptile_geometry_test )
  RUNTEST( mapsize_validation_test )
  RUNTEST( uoextension_test )

  RUNTEST( caseinsensitive_compare_test )
  RUNTEST( resolve_test )
  RUNTEST( ipmatch_test )
  RUNTEST( execution_error_format_test )
  RUNTEST( www_test )
  RUNTEST( clibutil_test )
  RUNTEST( clibmisc_test )
  RUNTEST( cfgfile_test )
  RUNTEST( uop_test )
  RUNTEST( tokens_test )
  RUNTEST( dynthreadpool_test )
  //  RUNTEST( dummy )

  UnitTest::display_test_results();
  return UnitTest::result();
}

}  // namespace Pol::Testing
