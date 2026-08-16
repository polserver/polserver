/** @file
 *
 * @par History
 */


#ifndef POL_TESTENV_H
#define POL_TESTENV_H

#include "clib/logfacility.h"

#include <string>

namespace Pol
{
namespace Testing
{
class UnitTest
{
public:
  template <typename F, typename T>
  UnitTest( F f, T res, const std::string& msg )
  {
    auto r = f();
    if ( r == res )
    {
      INFO_PRINTLN( "    {}", msg );
      UnitTest::inc_successes();
    }
    else
    {
      UnitTest::inc_failures();
      INFO_PRINT( "    {}: {} != {}", msg, r, res );
    }
  }
  static void inc_failures() { ++UnitTest::failures; }
  static void inc_successes() { ++UnitTest::successes; }
  static void display_test_results()
  {
    INFO_PRINTLN(
        "##############\n"
        "Successes: {}\n"
        "Failures:  {}\n"
        "##############",
        UnitTest::successes, UnitTest::failures );
  }

  static bool result() { return UnitTest::failures == 0; }

private:
  static unsigned int failures;
  static unsigned int successes;
};

// A unit test that needs real files works in unittest_temp/<test name>, under the working
// directory the test suite is started in. One base directory for all of them, so that a run
// which dies before its cleanup leaves everything it wrote in one recognisable place.
std::string unittest_path( const std::string& testname );

// Creates that directory empty and removes it again, whatever the test does in between.
class UnitTestDir
{
public:
  explicit UnitTestDir( std::string path );
  ~UnitTestDir();
  UnitTestDir( const UnitTestDir& ) = delete;
  UnitTestDir& operator=( const UnitTestDir& ) = delete;

  const std::string& path() const { return _path; }

private:
  std::string _path;
};

void test_splitnamevalue();
void test_convertquotedstring();
void test_sanitizeUnicodeWithIso();
void test_encodingconversions();

void skilladv_test();
void dynprops_test();
void dummy();
void packet_test();

void vector2d_test();
void vector3d_test();
void pos2d_test();
void pos3d_test();
void pos4d_test();
void range2d_test();
void range3d_test();

void test_curlfeatures();

void decay_test();
void location_test();
void zone_bookkeeping_test();
void place_at_test();
void container_slot_test();
void clamp_test();
void crypt_test();
void webscript_test();
void maptile_geometry_test();
void mapsize_validation_test();
void uoextension_test();
void caseinsensitive_compare_test();
void resolve_test();
void ipmatch_test();
void www_test();
void clibmisc_test();
void clibutil_test();
void cfgfile_test();
void uop_test();
void tokens_test();
void dynthreadpool_test();
}  // namespace Testing
}  // namespace Pol
#endif
