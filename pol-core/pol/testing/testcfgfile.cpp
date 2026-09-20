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

// The same, byte for byte: on Windows the stream above turns every \n into a \r\n of its own, so
// a test that cares which line endings the file really has cannot use it.
std::string write_cfg_raw( const std::string& name, const std::string& body )
{
  const std::string path = testdir + "/" + name;
  std::ofstream ofs( path, std::ios::binary );
  ofs << body;
  return path;
}

// Reads one element out of a file written for the occasion. The element is disowned from the file
// before that goes out of scope, since none of the callers below asks it to report an error.
Clib::ConfigElem read_one( const std::string& path )
{
  Clib::ConfigFile cf( path.c_str(), "Probe" );
  Clib::ConfigElem elem;
  cf.read( elem );
  elem.set_source( static_cast<const Clib::ConfigSource*>( nullptr ) );
  return elem;
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

// The order properties come back in, which is not the order the file lists them in.
void test_property_order()
{
  // remove_first_prop answers smallest name first, case-insensitively. Menu entries are built in
  // that order and read_movecost() interpolates over it, so it is load-bearing rather than
  // incidental.
  UnitTest(
      [&]()
      {
        auto elem = read_one( write_cfg( "order.cfg",
                                         "Probe Main\n"
                                         "{\n"
                                         "  zebra 1\n"
                                         "  Alpha 2\n"
                                         "  mango 3\n"
                                         "}\n" ) );
        std::string seen, name, value;
        while ( elem.remove_first_prop( &name, &value ) )
        {
          if ( !seen.empty() )
            seen += ",";
          seen += name + "=" + value;
        }
        return seen;
      },
      std::string( "Alpha=2,mango=3,zebra=1" ),
      "remove_first_prop walks the properties in name order" );

  // A repeated name is read one at a time, and has to come back in the order the file listed it:
  // that is how a CProp, a Coordinate and an ACCTMATCH are all read.
  UnitTest(
      [&]()
      {
        auto elem = read_one( write_cfg( "dupes.cfg",
                                         "Probe Main\n"
                                         "{\n"
                                         "  Item first\n"
                                         "  Item second\n"
                                         "  Item third\n"
                                         "}\n" ) );
        std::string seen, value;
        while ( elem.remove_prop( "Item", &value ) )
          seen += value + " ";
        return seen;
      },
      std::string( "first second third " ), "a repeated property comes back in file order" );

  // Removing by name and draining in order are mixed freely by the config readers.
  UnitTest(
      [&]()
      {
        auto elem = read_one( write_cfg( "mixed.cfg",
                                         "Probe Main\n"
                                         "{\n"
                                         "  bravo 2\n"
                                         "  alpha 1\n"
                                         "  charlie 3\n"
                                         "}\n" ) );
        std::string value;
        elem.remove_prop( "alpha", &value );  // the one remove_first_prop would have said first
        std::string seen, name;
        while ( elem.remove_first_prop( &name, &value ) )
          seen += name + " ";
        return seen;
      },
      std::string( "bravo charlie " ),
      "a property removed by name is skipped by remove_first_prop" );

  UnitTest(
      [&]()
      {
        auto elem = read_one( write_cfg( "haveprop.cfg", "Probe Main\n{\n  Key value\n}\n" ) );
        // Case-insensitive, and it leaves what it finds in place.
        const bool before = elem.has_prop( "KEY" ) && elem.has_prop( "key" );
        std::string value;
        return before && elem.remove_prop( "Key", &value ) && !elem.has_prop( "Key" );
      },
      true, "has_prop finds a property without removing it" );
}

// The reader hands out lines from a buffer of its own, so where a line falls in that buffer, and
// how the file ends, both need pinning down.
void test_line_reading()
{
  UnitTest(
      [&]()
      {
        const std::string huge( 200000, 'x' );  // far longer than the read buffer
        auto elem = read_one(
            write_cfg( "huge.cfg", "Probe Main\n{\n  Big " + huge + "\n  After yes\n}\n" ) );
        return elem.remove_string( "Big" ) == huge && elem.remove_string( "After" ) == "yes";
      },
      true, "a value longer than the read buffer is read whole" );

  // Enough properties to cross several buffer refills, checked at both ends and in the middle.
  UnitTest(
      [&]()
      {
        std::string body = "Probe Main\n{\n";
        for ( int i = 0; i < 20000; ++i )
          body += fmt::format( "  Pad{} {}\n", i, i );
        body += "}\n";
        auto elem = read_one( write_cfg( "straddle.cfg", body ) );
        return elem.remove_int( "Pad0" ) == 0 && elem.remove_int( "Pad9999" ) == 9999 &&
               elem.remove_int( "Pad19999" ) == 19999;
      },
      true, "properties either side of a buffer refill are all read" );

  UnitTest(
      [&]()
      {
        // No newline after the closing brace, which is where the reader runs out of file.
        auto elem = read_one( write_cfg_raw( "nonewline.cfg", "Probe Main\n{\n  Key value\n}" ) );
        return elem.remove_string( "Key" ) == "value";
      },
      true, "an element whose file ends without a newline is read" );

  UnitTest(
      [&]()
      {
        auto elem =
            read_one( write_cfg_raw( "crlf.cfg", "Probe Main\r\n{\r\n  Key value\r\n}\r\n" ) );
        return elem.remove_string( "Key" ) == "value";
      },
      true, "a carriage return is trimmed rather than kept in the value" );

  UnitTest(
      [&]()
      {
        auto elem = read_one( write_cfg_raw( "lf.cfg", "Probe Main\n{\n  Key value\n}\n" ) );
        return elem.remove_string( "Key" ) == "value";
      },
      true, "a file with bare newlines is read the same way" );

  // Blank lines and comments between properties, which every config file in the tree has.
  UnitTest(
      [&]()
      {
        auto elem = read_one( write_cfg( "comments.cfg",
                                         "Probe Main\n"
                                         "{\n"
                                         "  // a comment\n"
                                         "\n"
                                         "  Key value\n"
                                         "  # another comment\n"
                                         "}\n" ) );
        std::string name, value;
        return elem.remove_string( "Key" ) == "value" && !elem.remove_first_prop( &name, &value );
      },
      true, "comments and blank lines are not properties" );
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
  test_property_order();
  test_line_reading();
  test_refusals();
  test_configfile_refusals();
  test_stub_source();
}
}  // namespace Pol::Testing
