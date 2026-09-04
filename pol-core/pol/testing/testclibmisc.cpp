/** @file
 *
 * @par History
 */

#include <cstdint>
#include <exception>
#include <fstream>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "clib/binaryfile.h"
#include "clib/clib.h"
#include "clib/filecont.h"
#include "clib/fileutil.h"
#include "clib/passert.h"
#include "clib/streamsaver.h"
#include "clib/strset.h"
#include "clib/timer.h"
#include "pol/testing/testenv.h"

// The small clib classes that only the core's own C++ uses, so no script or shard test can reach
// them: BinaryFile, OnlineStatistics, StringSet, FileContents, StreamWriter, Timer, and the
// assertion handler itself.

namespace Pol::Testing
{
namespace
{
const std::string testdir = unittest_path( "clibmisc" );

// A file inside this test's own directory.
std::string testdir_file( const std::string& name )
{
  return testdir + "/" + name;
}

// A file of four little-endian u32 values, which is what the BinaryFile cases read back.
const std::vector<std::uint32_t> probe_values = { 0x11111111u, 0x22222222u, 0x33333333u,
                                                  0x44444444u };

void write_probe_file()
{
  std::ofstream ofs( testdir_file( "probe.bin" ), std::ios::binary );
  ofs.write( reinterpret_cast<const char*>( probe_values.data() ),
             static_cast<std::streamsize>( probe_values.size() * sizeof( std::uint32_t ) ) );
}

// Every BinaryFile accessor. Its preconditions are passerts rather than exceptions, so the cases
// below only ever call it on an open file.
void test_binaryfile()
{
  UnitTest(
      []()
      {
        Clib::BinaryFile bf( testdir_file( "probe.bin" ), std::ios::in );
        std::uint32_t val = 0;
        bf.Read( val );
        return val;
      },
      0x11111111u, "BinaryFile reads a value" );

  UnitTest(
      []()
      {
        Clib::BinaryFile bf( testdir_file( "probe.bin" ), std::ios::in );
        std::uint32_t vals[2] = { 0, 0 };
        bf.Read( vals, 2 );
        return vals[1];
      },
      0x22222222u, "BinaryFile reads an array" );

  // ReadVector sizes itself from the file, so it needs the length to divide evenly.
  UnitTest(
      []()
      {
        Clib::BinaryFile bf( testdir_file( "probe.bin" ), std::ios::in );
        std::vector<std::uint32_t> vals;
        bf.ReadVector( vals );
        return vals.size() == probe_values.size() && vals == probe_values;
      },
      true, "BinaryFile reads the whole file into a vector" );

  UnitTest(
      []()
      {
        Clib::BinaryFile bf( testdir_file( "probe.bin" ), std::ios::in );
        return static_cast<int>( bf.FileSize() );
      },
      16, "BinaryFile reports the file size" );

  // Seek is absolute, and FileSize leaves the position it found.
  UnitTest(
      []()
      {
        Clib::BinaryFile bf( testdir_file( "probe.bin" ), std::ios::in );
        bf.Seek( 8 );
        bf.FileSize();
        std::uint32_t val = 0;
        bf.Read( val );
        return val;
      },
      0x33333333u, "BinaryFile seeks and keeps its position across FileSize" );

  UnitTest(
      []()
      {
        Clib::BinaryFile bf( testdir_file( "probe.bin" ), std::ios::in );
        bf.Close();
        return true;
      },
      true, "BinaryFile closes" );

  UnitTest(
      []()
      {
        try
        {
          Clib::BinaryFile bf( testdir_file( "no_such_file.bin" ), std::ios::in );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "opening a missing binary file throws" );

  // Reading past the end is the only failure ReadBuffer itself reports.
  UnitTest(
      []()
      {
        Clib::BinaryFile bf( testdir_file( "probe.bin" ), std::ios::in );
        try
        {
          std::uint32_t vals[8] = {};
          bf.Read( vals, 8 );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "reading past the end of a binary file throws" );

  // A file whose length is not a multiple of the element size has no element count.
  UnitTest(
      []()
      {
        {
          std::ofstream ofs( testdir_file( "ragged.bin" ), std::ios::binary );
          ofs << "abcde";
        }
        Clib::BinaryFile bf( testdir_file( "ragged.bin" ), std::ios::in );
        try
        {
          std::vector<std::uint32_t> vals;
          bf.ReadVector( vals );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "a file that is not a whole number of elements throws" );

  UnitTest(
      []()
      {
        Clib::BinaryFile bf( testdir_file( "probe.bin" ), std::ios::in );
        return bf.sizeEstimate() >= sizeof( Clib::BinaryFile );
      },
      true, "BinaryFile estimates its own size" );
}

// The running mean/variance accumulator the profiling counters use.
void test_online_statistics()
{
  auto filled = []()
  {
    Clib::OnlineStatistics stats;
    for ( double v : { 2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0 } )
      stats.update( v );
    return stats;
  };

  UnitTest( [&]() { return filled().count(); }, static_cast<u64>( 8 ),
            "OnlineStatistics counts its updates" );
  UnitTest( [&]() { return filled().total(); }, 40.0, "OnlineStatistics totals its updates" );
  UnitTest( [&]() { return filled().mean(); }, 5.0, "OnlineStatistics means its updates" );
  UnitTest( [&]() { return filled().max(); }, 9.0, "OnlineStatistics keeps the largest" );

  // The textbook sample variance of that series is 32/7.
  UnitTest( [&]() { return filled().variance(); }, 32.0 / 7.0,
            "OnlineStatistics computes the sample variance" );
  UnitTest( [&]() { return filled().standard_deviation(); }, std::sqrt( 32.0 / 7.0 ),
            "OnlineStatistics computes the standard deviation" );

  // A sample of one has no spread to report, and none at all is not a division by zero.
  UnitTest(
      []()
      {
        Clib::OnlineStatistics stats;
        stats.update( 3.0 );
        return stats.variance();
      },
      0.0, "a single sample has no variance" );
  UnitTest( []() { return Clib::OnlineStatistics().variance(); }, 0.0,
            "an empty accumulator has no variance" );
  UnitTest( []() { return Clib::OnlineStatistics().mean(); }, 0.0,
            "an empty accumulator has no mean" );

  UnitTest( [&]() { return fmt::format( "{}", filled() ); },
            std::string( "mean: 5.00 max: 9.00 std: 2.14 count: 8 total: 40.00" ),
            "OnlineStatistics formats itself" );
}

// StringSet is a set of names with a packed string form; only its iterators are unreached.
void test_stringset()
{
  UnitTest(
      []()
      {
        Clib::StringSet set;
        set.readfrom( "beta alpha gamma" );
        std::string joined;
        for ( const auto& s : set )
          joined += s + ",";
        return joined;
      },
      std::string( "alpha,beta,gamma," ), "a string set iterates in sorted order" );

  // The const overloads are a separate pair of functions.
  UnitTest(
      []()
      {
        Clib::StringSet set;
        set.add( "one" );
        set.add( "two" );
        const Clib::StringSet& cset = set;
        return static_cast<int>( std::distance( cset.begin(), cset.end() ) );
      },
      2, "a const string set iterates too" );

  UnitTest(
      []()
      {
        Clib::StringSet set;
        set.readfrom( "alpha beta" );
        set.remove( "alpha" );
        return set.extract();
      },
      std::string( "beta " ), "a removed name is gone from the packed form" );
}

// FileContents reads a whole file into a string, and refuses one it cannot open.
void test_filecontents()
{
  UnitTest(
      []()
      {
        {
          std::ofstream ofs( testdir_file( "contents.txt" ) );
          ofs << "the whole file";
        }
        Clib::FileContents fc( testdir_file( "contents.txt" ).c_str() );
        return fc.str_contents();
      },
      std::string( "the whole file" ), "FileContents reads a file" );

  UnitTest(
      []()
      {
        Clib::FileContents fc( testdir_file( "contents.txt" ).c_str() );
        fc.set_contents( "replaced" );
        return std::string( fc.contents() );
      },
      std::string( "replaced" ), "FileContents takes replacement contents" );

  // Quietly, which is what the suppress flag is for.
  UnitTest(
      []()
      {
        try
        {
          Clib::FileContents fc( testdir_file( "no_such_file.txt" ).c_str(), true );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "a missing file is refused without a message" );

  UnitTest(
      []()
      {
        try
        {
          Clib::FileContents fc( testdir_file( "no_such_file.txt" ).c_str() );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "a missing file is refused with a message" );
}

// StreamWriter is the buffered writer the world save uses.
void test_streamwriter()
{
  UnitTest(
      []()
      {
        {
          Clib::StreamWriter sw( testdir_file( "written.txt" ) );
          sw.comment( "a bare comment" );
          sw.comment( "a formatted comment: {}", 42 );
          sw.begin( "Element" );
          sw.add<"Key">( "value" );
          sw.add<"Number">( 7 );
          sw.add<"Flag">( true );
          sw.end();
          sw.begin( "Element", "Named" );
          sw.end();
          sw.flush_close();
        }
        std::ifstream ifs( testdir_file( "written.txt" ) );
        return std::string( ( std::istreambuf_iterator<char>( ifs ) ),
                            std::istreambuf_iterator<char>() );
      },
      std::string( "# a bare comment\n"
                   "# a formatted comment: 42\n"
                   "Element\n{\n\tKey\tvalue\n\tNumber\t7\n\tFlag\t1\n}\n\n"
                   "Element Named\n{\n}\n\n" ),
      "StreamWriter writes what it was given" );

  // Closing twice is a no-op, which is what lets the destructor run after an explicit close.
  UnitTest(
      []()
      {
        Clib::StreamWriter sw( testdir_file( "closed_twice.txt" ) );
        sw.flush_close();
        sw.flush_close();
        return true;
      },
      true, "StreamWriter closes only once" );

  UnitTest(
      []()
      {
        try
        {
          Clib::StreamWriter sw( testdir_file( "no_such_dir/file.txt" ) );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "StreamWriter refuses a path it cannot open" );

  // Deferred writing, what pol.cfg WorldSaveDeferMB buys: the text a save formats is held until
  // write_deferred(), so the world runs again while the bytes are still in memory.
  auto file_text = []( const char* name )
  {
    std::ifstream ifs( testdir_file( name ) );
    return std::string( ( std::istreambuf_iterator<char>( ifs ) ),
                        std::istreambuf_iterator<char>() );
  };

  UnitTest(
      [&]()
      {
        Clib::StreamWriter::set_deferred_budget( 1 << 20 );
        std::string before;
        {
          Clib::StreamWriter sw( testdir_file( "deferred.txt" ) );
          sw.defer_writes();
          sw.append( "first\n" );
          sw.append( "second\n" );
          before = file_text( "deferred.txt" );
          sw.write_deferred();
          sw.flush_close();
        }
        Clib::StreamWriter::set_deferred_budget( 0 );
        return before + "|" + file_text( "deferred.txt" );
      },
      std::string( "|first\nsecond\n" ), "a deferred writer holds its text until write_deferred" );

  // A budget too small for what is handed over is what a shard owner will actually set, so the
  // eviction path matters as much as the holding one.
  UnitTest(
      [&]()
      {
        Clib::StreamWriter::set_deferred_budget( 8 );
        std::string over_budget;
        {
          Clib::StreamWriter sw( testdir_file( "evicted.txt" ) );
          sw.defer_writes();
          sw.append( "aaaaaaaaaa\n" );
          sw.append( "bbbbbbbbbb\n" );
          sw.append( "cccccccccc\n" );
          over_budget = file_text( "evicted.txt" );
          sw.write_deferred();
          sw.flush_close();
        }
        Clib::StreamWriter::set_deferred_budget( 0 );
        return over_budget + "|" + file_text( "evicted.txt" );
      },
      std::string( "aaaaaaaaaa\nbbbbbbbbbb\n|aaaaaaaaaa\nbbbbbbbbbb\ncccccccccc\n" ),
      "over the budget the oldest deferred block is written first" );

  // bytes_written() is what the save reports per file, counted when the file is handed the block
  // rather than when it reaches the disk.
  UnitTest(
      [&]()
      {
        Clib::StreamWriter::set_deferred_budget( 1 << 20 );
        Clib::StreamWriter sw( testdir_file( "deferred_count.txt" ) );
        sw.defer_writes();
        sw.append( "0123456789" );
        const auto counted = sw.bytes_written();
        sw.write_deferred();
        sw.flush_close();
        Clib::StreamWriter::set_deferred_budget( 0 );
        return counted;
      },
      size_t( 10 ), "a deferred block counts as written when the file is handed it" );
}

// The elapsed-time helper. Only its named form and the seconds/print accessors are unreached.
void test_timer()
{
  UnitTest(
      []()
      {
        Tools::Timer<Tools::SilentT> timer( "named timer" );
        timer.stop();
        return timer.ellapsed() >= 0;
      },
      true, "a named timer measures a span" );

  // Milliseconds and seconds are the same measurement in two units, and a span this short is
  // zero in both.
  UnitTest(
      []()
      {
        Tools::Timer<Tools::SilentT> timer( "seconds" );
        timer.stop();
        return timer.ellapsed_s() == timer.ellapsed() / 1000.0;
      },
      true, "a timer reports the same span in seconds" );

  UnitTest(
      []()
      {
        Tools::Timer<Tools::SilentT> timer( "printed" );
        timer.stop();
        timer.print();
        return true;
      },
      true, "a timer prints its span" );

  UnitTest(
      []()
      {
        Tools::HighPerfTimer timer;
        return timer.ellapsed().count() >= 0;
      },
      true, "the high resolution timer measures a span" );
}

// A failed assertion, with the abort and shutdown switches off so that it throws instead. The
// test shard runs with both on: abort ends the process, and shutdown signals the server to exit,
// which leaves the rest of the run waiting on a shutdown that never completes. They are restored
// before the case returns, whatever happens.
void test_passert()
{
  struct SwitchGuard
  {
    SwitchGuard() : abort( Clib::passert_abort ), shutdown( Clib::passert_shutdown )
    {
      Clib::passert_abort = false;
      Clib::passert_shutdown = false;
    }
    ~SwitchGuard()
    {
      Clib::passert_abort = abort;
      Clib::passert_shutdown = shutdown;
    }
    bool abort;
    bool shutdown;
  };

  UnitTest(
      []()
      {
        SwitchGuard guard;
        try
        {
          Clib::passert_failed( "1 == 2", "testclibmisc.cpp", 1 );
        }
        catch ( const std::exception& ex )
        {
          return std::string( ex.what() );
        }
        return std::string( "did not throw" );
      },
      std::string( "Assertion Failed: 1 == 2, testclibmisc.cpp, line 1" ),
      "a failed assertion throws, naming the expression" );

  UnitTest(
      []()
      {
        SwitchGuard guard;
        try
        {
          Clib::passert_failed( "1 == 2", "because", "testclibmisc.cpp", 2 );
        }
        catch ( const std::exception& ex )
        {
          return std::string( ex.what() );
        }
        return std::string( "did not throw" );
      },
      std::string( "Assertion Failed: 1 == 2 (because), testclibmisc.cpp, line 2" ),
      "a failed assertion carries its reason" );
}
}  // namespace

void clibmisc_test()
{
  UnitTestDir dir( testdir );
  write_probe_file();

  test_binaryfile();
  test_online_statistics();
  test_stringset();
  test_filecontents();
  test_streamwriter();
  test_timer();
  test_passert();
}
}  // namespace Pol::Testing
