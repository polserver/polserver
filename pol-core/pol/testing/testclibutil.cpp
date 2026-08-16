/** @file
 *
 * @par History
 */

#include <cstdio>
#include <fstream>
#include <string>

#include "clib/fileutil.h"
#include "clib/iohelp.h"
#include "clib/strutil.h"
#include "pol/testing/testenv.h"

// The clib helpers nothing else reaches. testmisc.cpp already covers the round-trippable half of
// strutil (splitnamevalue, the quoted-string pair, the iso sanitizer, the encoding conversions);
// what is left here is the arms that do not round trip, plus fileutil and iohelp, which no test
// touched at all.

namespace Pol::Testing
{
namespace
{
// A directory of our own under the working directory, removed again at the end, so that the file
// helpers work on something real rather than on whatever happens to be lying around.
const std::string testdir = unittest_path( "clibutil" );

std::string decoded( const std::string& in )
{
  std::string tmp = in;
  Clib::decodequotedstring( tmp );
  return tmp;
}

std::string sanitized( const std::string& in )
{
  std::string tmp = in;
  Clib::sanitizeUnicode( &tmp );
  return tmp;
}

// The escape arms that testmisc.cpp's round-trip helper cannot reach: a decoded newline does not
// encode back to "\n", and an unterminated escape has nothing to encode back from.
void test_decodequotedstring_escapes()
{
  UnitTest( []() { return decoded( "\"a\\nb\"" ); }, std::string( "a\nb" ),
            "an escaped n decodes to a newline" );
  UnitTest( []() { return decoded( "\"\\n\"" ); }, std::string( "\n" ),
            "a lone escaped newline decodes" );

  // A backslash as the last character before the closing quote: the escape reads the terminating
  // NUL and gives up, keeping what it had.
  UnitTest( []() { return decoded( "\"ab\\" ); }, std::string( "ab" ),
            "a trailing backslash ends the string" );
  UnitTest( []() { return decoded( "\"\\" ); }, std::string( "" ),
            "a string that is only a trailing backslash decodes to nothing" );

  // Anything else after a backslash is taken literally, which is how a quote gets in.
  UnitTest( []() { return decoded( "\"a\\\"b\"" ); }, std::string( "a\"b" ),
            "an escaped quote is literal" );
  UnitTest( []() { return decoded( "\"a\\\\b\"" ); }, std::string( "a\\b" ),
            "an escaped backslash is literal" );
  UnitTest( []() { return decoded( "\"a\\tb\"" ); }, std::string( "atb" ),
            "an unknown escape drops the backslash" );

  // The opening quote is skipped without being checked for, so an unquoted string loses its
  // first character. Its one caller (cfgfile.cpp:444) tests for the quote first, which is what
  // makes that a precondition rather than a defect -- pinned here so a future caller knows.
  UnitTest( []() { return decoded( "plain" ); }, std::string( "lain" ),
            "an unquoted string loses its first character" );

  // Empty is the one input it does guard against.
  UnitTest( []() { return decoded( "" ); }, std::string( "" ), "an empty string is untouched" );
}

// unicodeToCp1252's two fallbacks, which are the only arms of it that answer '?'.
void test_unicode_to_cp1252()
{
  // The C1 control block has no cp1252 meaning at all.
  UnitTest( []() { return Clib::unicodeToCp1252( 0x80 ); }, static_cast<uint8_t>( '?' ),
            "0x80 has no cp1252 form" );
  UnitTest( []() { return Clib::unicodeToCp1252( 0x9f ); }, static_cast<uint8_t>( '?' ),
            "0x9f has no cp1252 form" );

  // Above 0xff only the handful the switch names convert; everything else falls through.
  UnitTest( []() { return Clib::unicodeToCp1252( 0x2122 ); }, static_cast<uint8_t>( 153 ),
            "the trademark sign converts" );
  UnitTest( []() { return Clib::unicodeToCp1252( 0x0178 ); }, static_cast<uint8_t>( 159 ),
            "Y with diaeresis converts" );
  UnitTest( []() { return Clib::unicodeToCp1252( 0x4e00 ); }, static_cast<uint8_t>( '?' ),
            "a CJK codepoint has no cp1252 form" );

  // Below 0x80 and the Latin-1 upper half pass straight through.
  UnitTest( []() { return Clib::unicodeToCp1252( 0x41 ); }, static_cast<uint8_t>( 'A' ),
            "ascii passes through" );
  UnitTest( []() { return Clib::unicodeToCp1252( 0xe4 ); }, static_cast<uint8_t>( 0xe4 ),
            "latin-1 passes through" );
}

// sanitizeUnicode has two independent passes: repair invalid encoding, then strip control
// characters. Each is reached on its own.
void test_sanitize_unicode()
{
  UnitTest( []() { return sanitized( "plain ascii" ); }, std::string( "plain ascii" ),
            "valid text is untouched" );

  // A lone continuation byte is not valid utf8 and is replaced rather than dropped.
  UnitTest(
      []()
      {
        return sanitized( std::string( "a\x80"
                                       "b" ) ) !=
               std::string(
                   "a\x80"
                   "b" );
      },
      true, "invalid utf8 is repaired" );

  // The control characters the second pass removes, and the three whitespace ones it keeps.
  UnitTest(
      []()
      {
        return sanitized(
            std::string( "a\x01"
                         "b" ) );
      },
      std::string( "ab" ), "a control character is stripped" );
  UnitTest(
      []()
      {
        return sanitized(
            std::string( "a\x7f"
                         "b" ) );
      },
      std::string( "ab" ), "delete is stripped" );
  UnitTest( []() { return sanitized( "a\tb\nc\rd" ); }, std::string( "a\tb\nc\rd" ),
            "tab, newline and return survive" );
}

// normalized_dir_form and strip_one, the two path helpers the config readers lean on.
void test_path_helpers()
{
  UnitTest( []() { return Clib::normalized_dir_form( "a\\b" ); }, std::string( "a/b/" ),
            "backslashes become slashes and a separator is appended" );
  UnitTest( []() { return Clib::normalized_dir_form( "a\\b\\" ); }, std::string( "a/b/" ),
            "a trailing backslash is kept as a separator" );
  UnitTest( []() { return Clib::normalized_dir_form( "a/b/" ); }, std::string( "a/b/" ),
            "an already normalized path is unchanged" );
  UnitTest( []() { return Clib::normalized_dir_form( "" ); }, std::string( "/" ),
            "an empty path is the root" );

  // strip_one removes the last path element, and answers -1 when there is none to remove.
  UnitTest(
      []()
      {
        std::string d = "a/b/c";
        Clib::strip_one( d );
        return d;
      },
      std::string( "a/b" ), "strip_one removes the last element" );
  UnitTest(
      []()
      {
        std::string d = "noseparator";
        return Clib::strip_one( d );
      },
      -1, "strip_one refuses a path with no separator" );
  UnitTest(
      []()
      {
        std::string d = "";
        return Clib::strip_one( d );
      },
      -1, "strip_one refuses an empty path" );

  // GetFilePart is the name alone, whatever the path in front of it.
  UnitTest( []() { return Clib::GetFilePart( "a/b/file.txt" ); }, std::string( "file.txt" ),
            "GetFilePart takes the name" );
  UnitTest( []() { return Clib::GetFilePart( "file.txt" ); }, std::string( "file.txt" ),
            "GetFilePart of a bare name is the name" );

  // GetTrueName resolves the path first, and hands back what it was given when it cannot.
  UnitTest( []() { return Clib::GetTrueName( "no_such_file_here.txt" ); },
            std::string( "no_such_file_here.txt" ),
            "GetTrueName of a missing file answers the name it was given" );
}

// make_dir builds every missing level, and the file helpers answer about what it made.
void test_directory_helpers()
{
  const std::string nested = testdir + "/one/two";

  UnitTest( [&]() { return Clib::make_dir( nested.c_str() ); }, 0,
            "make_dir creates a nested path" );
  UnitTest( [&]() { return Clib::IsDirectory( nested.c_str() ); }, true,
            "the nested path is a directory" );

  // Making one that is already there succeeds without doing anything.
  UnitTest( [&]() { return Clib::make_dir( nested.c_str() ); }, 0,
            "make_dir on an existing path succeeds" );

  UnitTest( [&]() { return Clib::IsDirectory( ( testdir + "/nope" ).c_str() ); }, false,
            "a path that does not exist is not a directory" );

  // A real file, so that the size and existence helpers have something to answer about.
  const std::string file = testdir + "/probe.txt";
  {
    std::ofstream ofs( file );
    ofs << "0123456789";
  }
  UnitTest( [&]() { return Clib::FileExists( file ); }, true, "the file exists" );
  UnitTest( [&]() { return Clib::filesize( file.c_str() ); }, 10, "the file is ten bytes" );
  UnitTest( [&]() { return Clib::FileExists( testdir + "/missing.txt" ); }, false,
            "a missing file does not exist" );
  // A file that is not there and a file of zero bytes are indistinguishable: stat failing and
  // stat reporting an empty file both answer 0.
  UnitTest( [&]() { return Clib::filesize( ( testdir + "/missing.txt" ).c_str() ); }, 0,
            "a missing file has no size" );

  // GetTrueName of something real resolves to the name at the end of the canonical path.
  UnitTest( [&]() { return Clib::GetTrueName( file.c_str() ); }, std::string( "probe.txt" ),
            "GetTrueName resolves a real file" );

  UnitTest( [&]() { return Clib::GetFileTimestamp( file.c_str() ) > 0u; }, true,
            "the file has a timestamp" );

  Clib::RemoveFile( file );
  UnitTest( [&]() { return Clib::FileExists( file ); }, false, "the file is gone once removed" );
}

// open_file wraps the three stream types and turns a failure into an exception carrying the name.
void test_open_file()
{
  const std::string file = testdir + "/stream.txt";

  UnitTest(
      [&]()
      {
        std::string name = file;
        std::ofstream ofs;
        Clib::open_file( ofs, name, std::ios::out );
        bool ok = ofs.is_open();
        ofs << "written";
        return ok;
      },
      true, "open_file opens an ofstream" );

  UnitTest(
      [&]()
      {
        std::string name = file;
        std::ifstream ifs;
        Clib::open_file( ifs, name, std::ios::in );
        std::string content;
        ifs >> content;
        return content;
      },
      std::string( "written" ), "open_file opens an ifstream and reads back" );

  UnitTest(
      [&]()
      {
        std::string name = file;
        std::fstream fs;
        Clib::open_file( fs, name, std::ios::in | std::ios::out );
        return fs.is_open();
      },
      true, "open_file opens an fstream" );

  // A path whose directory does not exist cannot be opened. Whether the stream throws or merely
  // fails is a library decision, so what is pinned is that the file is not open either way and
  // that a thrown message names the file.
  UnitTest(
      [&]()
      {
        std::string name = testdir + "/no_such_dir/stream.txt";
        std::ofstream ofs;
        try
        {
          Clib::open_file( ofs, name, std::ios::out );
        }
        catch ( const std::runtime_error& ex )
        {
          return std::string( ex.what() ).find( name ) != std::string::npos;
        }
        return !ofs.is_open();
      },
      true, "open_file into a missing directory does not open a stream" );

  Clib::RemoveFile( file );
}

// ReadEntireFile reads from the current offset to the end, and an empty range allocates nothing.
void test_read_entire_file()
{
  const std::string file = testdir + "/whole.txt";
  {
    std::ofstream ofs( file, std::ios::binary );
    ofs << "abcdefgh";
  }

  UnitTest(
      [&]()
      {
        FILE* fp = fopen( file.c_str(), "rb" );
        auto buf = Clib::ReadEntireFile( fp );
        fclose( fp );
        return buf.size();
      },
      static_cast<size_t>( 8 ), "ReadEntireFile reads the whole file" );

  // Starting part way in reads only the remainder -- the length comes from the offset, not from
  // the size of the file.
  UnitTest(
      [&]()
      {
        FILE* fp = fopen( file.c_str(), "rb" );
        fseek( fp, 5, SEEK_SET );
        auto buf = Clib::ReadEntireFile( fp );
        fclose( fp );
        return buf.size();
      },
      static_cast<size_t>( 3 ), "ReadEntireFile reads from the current offset" );

  // At the end there is nothing left, and the empty case skips the read entirely.
  UnitTest(
      [&]()
      {
        FILE* fp = fopen( file.c_str(), "rb" );
        fseek( fp, 0, SEEK_END );
        auto buf = Clib::ReadEntireFile( fp );
        fclose( fp );
        return buf.empty();
      },
      true, "ReadEntireFile at the end reads nothing" );

  Clib::RemoveFile( file );
}
}  // namespace

void clibutil_test()
{
  UnitTestDir dir( testdir );

  test_decodequotedstring_escapes();
  test_unicode_to_cp1252();
  test_sanitize_unicode();
  test_path_helpers();
  test_directory_helpers();
  test_open_file();
  test_read_entire_file();
}
}  // namespace Pol::Testing
