/** @file
 *
 * @par History
 */

#include "pol/testing/testenv.h"

#include <filesystem>
#include <system_error>
#include <utility>

#include "clib/fileutil.h"
#include "plib/systemstate.h"

namespace Pol::Testing
{
unsigned int UnitTest::failures = 0;
unsigned int UnitTest::successes = 0;

namespace
{
const std::string unittest_temp = "unittest_temp";
constexpr char CONSOLE_RED[] = "\x1b[31m";
}  // namespace

void UnitTest::report_failure( const std::string& msg, const std::string& got,
                               const std::string& expected )
{
  if ( Plib::systemstate.config.enable_colored_output )
    INFO_PRINTLN( "{}    failed: {}: {} != {}{}", CONSOLE_RED, msg, got, expected,
                  Clib::Logging::CONSOLE_RESET_COLOR );
  else
    INFO_PRINTLN( "    failed: {}: {} != {}", msg, got, expected );
}

std::string unittest_path( const std::string& testname )
{
  return unittest_temp + "/" + testname;
}

UnitTestDir::UnitTestDir( std::string path ) : _path( std::move( path ) )
{
  std::error_code ec;
  std::filesystem::remove_all( _path, ec );
  // make_dir rather than MakeDirectory: the base directory has to be made along the way.
  Clib::make_dir( _path.c_str() );
}

UnitTestDir::~UnitTestDir()
{
  std::error_code ec;
  std::filesystem::remove_all( _path, ec );
  // The base directory goes too, but only once the last test has taken its own away.
  std::filesystem::remove( unittest_temp, ec );
}
}  // namespace Pol::Testing
