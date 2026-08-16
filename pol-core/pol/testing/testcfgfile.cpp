/** @file
 *
 * @par History
 */

#include <exception>
#include <fstream>
#include <string>

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"
#include "pol/testing/testenv.h"

// The ConfigElem accessors that only C++ uses. Everything reachable from a script is already
// covered by testsuite/escript/cfg/; what is left is the typed removers no config in the tree
// happens to call, the get_prop / read_ family that throws instead of defaulting, and the
// failure paths of ConfigFile itself.

namespace Pol::Testing
{
namespace
{
const std::string testdir = unittest_path( "cfgfile" );

std::string write_cfg( const std::string& name, const std::string& body )
{
  const std::string path = testdir + "/" + name;
  std::ofstream ofs( path );
  ofs << body;
  return path;
}

// One element with a property of every shape the accessors below read back.
const std::string probe_body =
    "Probe Main\n"
    "{\n"
    "  Serial      0x40001234\n"
    "  Count       42\n"
    "  Ratio       2.5\n"
    "  Text        hello world\n"
    "  Quoted      \" spaced \"\n"
    "  NotANumber  fish\n"
    "}\n";

// A full element read out of its own file, so each case starts from every property rather than
// from whatever the previous one removed. An element reports its errors through the file it was
// read from, so the file has to outlive it: cf is declared first and destroyed last.
struct Probe
{
  Probe() : cf( ( testdir + "/probe.cfg" ).c_str(), "Probe" ) { cf.read( elem ); }
  Clib::ConfigFile cf;
  Clib::ConfigElem elem;
};

// The typed removers, each in both its found and its defaulted form.
void test_typed_removers()
{
  UnitTest(
      []()
      {
        Probe p;
        return p.elem.remove_unsigned( "Serial" );
      },
      0x40001234u, "remove_unsigned reads a hex value" );

  UnitTest(
      []()
      {
        Probe p;
        return p.elem.remove_unsigned( "Missing", 7 );
      },
      7u, "remove_unsigned falls back to its default" );

  UnitTest(
      []()
      {
        Probe p;
        return p.elem.remove_float( "Ratio", 0.0f );
      },
      2.5f, "remove_float reads a real" );

  UnitTest(
      []()
      {
        Probe p;
        return p.elem.remove_float( "Missing", 1.5f );
      },
      1.5f, "remove_float falls back to its default" );

  // Removing takes the property away, so asking twice defaults the second time.
  UnitTest(
      []()
      {
        Probe p;
        p.elem.remove_unsigned( "Count" );
        return p.elem.remove_unsigned( "Count", 99 );
      },
      99u, "a removed property is gone" );
}

// get_prop and read_string do not default -- they throw when the property is not there, and
// read_string leaves the element as it found it.
void test_reading_accessors()
{
  UnitTest(
      []()
      {
        Probe p;
        unsigned int serial = 0;
        p.elem.get_prop( "Serial", &serial );
        return serial;
      },
      0x40001234u, "get_prop reads without removing" );

  // Still there afterwards, which is the whole difference from remove_.
  UnitTest(
      []()
      {
        Probe p;
        unsigned int serial = 0;
        p.elem.get_prop( "Serial", &serial );
        p.elem.get_prop( "Serial", &serial );
        return serial;
      },
      0x40001234u, "get_prop can be called twice" );

  UnitTest(
      []()
      {
        Probe p;
        try
        {
          unsigned int serial = 0;
          p.elem.get_prop( "Missing", &serial );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "get_prop throws when the property is missing" );

  UnitTest(
      []()
      {
        Probe p;
        return p.elem.read_string( "Text" );
      },
      std::string( "hello world" ), "read_string reads the rest of the line" );

  UnitTest(
      []()
      {
        Probe p;
        return p.elem.read_string( "Missing", "fallback" );
      },
      std::string( "fallback" ), "read_string falls back to its default" );

  UnitTest(
      []()
      {
        Probe p;
        try
        {
          p.elem.read_string( "Missing" );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "read_string with no default throws when the property is missing" );

  // A quoted value keeps its surrounding whitespace; an unquoted one is trimmed.
  UnitTest(
      []()
      {
        Probe p;
        return p.elem.read_string( "Quoted" );
      },
      std::string( " spaced " ), "a quoted value keeps its spaces" );
}

// What each accessor does with a value it cannot parse, and with a name nothing declares.
void test_refusals()
{
  // remove_int parses with atoi, which has no way to report a refusal: a value that is not a
  // number at all reads as zero.
  UnitTest(
      []()
      {
        Probe p;
        return p.elem.remove_int( "NotANumber" );
      },
      0, "a non-numeric value reads as zero through remove_int" );

  // remove_ushort goes through the checked parser instead, and that one does refuse.
  UnitTest(
      []()
      {
        Probe p;
        try
        {
          p.elem.remove_ushort( "NotANumber" );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "a non-numeric value is refused by remove_ushort" );

  UnitTest(
      []()
      {
        Probe p;
        try
        {
          p.elem.remove_int( "Missing" );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "remove_int with no default throws when the property is missing" );

  UnitTest(
      []()
      {
        Probe p;
        try
        {
          p.elem.remove_string( "Missing" );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "remove_string with no default throws when the property is missing" );

  UnitTest(
      []()
      {
        Probe p;
        try
        {
          p.elem.throw_prop_not_found( "Whatever" );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "throw_prop_not_found throws" );

  // warn is the non-fatal sibling: it prints through the element's source and returns.
  UnitTest(
      []()
      {
        Probe p;
        p.elem.warn( "a warning from the test" );
        p.elem.warn_with_line( "a warning naming its line" );
        return true;
      },
      true, "warn and warn_with_line return rather than throw" );
}

// ConfigFile's own failures: a file that is not there, and an element type it was not told about.
void test_configfile_refusals()
{
  UnitTest(
      []()
      {
        try
        {
          Clib::ConfigFile cf( ( testdir + "/no_such_file.cfg" ).c_str(), "Probe" );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "opening a missing config file throws" );

  UnitTest(
      [&]()
      {
        const std::string path = write_cfg( "wrongtype.cfg", "Unexpected Main\n{\n}\n" );
        try
        {
          Clib::ConfigFile cf( path.c_str(), "Probe" );
          Clib::ConfigElem elem;
          cf.read( elem );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "an element type the file was not opened for throws" );

  // A closing brace with no element open.
  UnitTest(
      [&]()
      {
        const std::string path = write_cfg( "straybrace.cfg", "}\n" );
        try
        {
          Clib::ConfigFile cf( path.c_str(), "Probe" );
          Clib::ConfigElem elem;
          cf.read( elem );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "a stray closing brace throws" );

  // With no allowed types given, any element type is accepted.
  UnitTest(
      [&]()
      {
        const std::string path = write_cfg( "anytype.cfg", "Anything Main\n{\n  Key value\n}\n" );
        Clib::ConfigFile cf( path.c_str() );
        Clib::ConfigElem elem;
        bool got = cf.read( elem );
        return got && std::string( elem.type() ) == "Anything";
      },
      true, "a file with no declared types accepts any element" );

  // The name it was opened with is kept for error reporting, and the timestamp is real.
  UnitTest(
      [&]()
      {
        const std::string path = write_cfg( "named.cfg", "Probe Main\n{\n}\n" );
        Clib::ConfigFile cf( path.c_str(), "Probe" );
        return cf.filename() == path && cf.modified() > 0;
      },
      true, "a config file reports the name it was opened with" );
}

// An element with no source at all still answers, which is what StubConfigSource is for.
void test_stub_source()
{
  UnitTest(
      []()
      {
        Clib::StubConfigSource stub;
        Clib::ConfigElem elem;
        elem.set_source( &stub );
        elem.warn( "a warning with a stub source" );
        return true;
      },
      true, "a stub source takes a warning" );

  UnitTest(
      []()
      {
        Clib::StubConfigSource stub;
        Clib::ConfigElem elem;
        elem.set_source( &stub );
        try
        {
          elem.throw_prop_not_found( "Whatever" );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "a stub source still throws for a missing property" );
}
}  // namespace

void cfgfile_test()
{
  UnitTestDir dir( testdir );
  write_cfg( "probe.cfg", probe_body );

  test_typed_removers();
  test_reading_accessors();
  test_refusals();
  test_configfile_refusals();
  test_stub_source();
}
}  // namespace Pol::Testing
