/** @file
 *
 * @par History
 */

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "pol/testing/testcompileranalysis.h"
#include "pol/testing/testenv.h"

// vscode-escript analyses a document on every keystroke, so Compiler::analyze() has to survive
// every half-typed prefix of a valid script. This feeds the testsuite's scripts in one word at a
// time, the way the extension's long "Actively typing sources" test does, and fails on an
// exception. A crash ends the whole unit test run.
//
// The cost grows with the square of a script's length. By default only the escript scripts of up
// to MAX_DEFAULT_WORDS words are typed. POLCORE_TEST_TYPING=escript types every escript script,
// and POLCORE_TEST_TYPING=all adds testsuite/pol. Both take hours on slow hardware, so they name
// each script and print each failure as it happens.

namespace Pol::Testing
{
using namespace Bscript::Compiler;
using namespace CompilerAnalysis;
namespace fs = std::filesystem;

namespace
{
constexpr size_t MAX_DEFAULT_WORDS = 100;

// The words of a script, and every line break as a word of its own, so that typing them back
// separated by spaces keeps the line structure.
std::vector<std::string> words_of( const std::string& text )
{
  std::vector<std::string> words;
  std::string word;
  for ( char c : text )
  {
    if ( c == ' ' || c == '\t' || c == '\r' || c == '\n' )
    {
      if ( !word.empty() )
        words.push_back( std::move( word ) );
      word.clear();
      if ( c == '\n' )
        words.emplace_back( "\n" );
    }
    else
      word += c;
  }
  if ( !word.empty() )
    words.push_back( std::move( word ) );
  return words;
}

std::string read_file( const fs::path& path )
{
  std::ifstream ifs( path, std::ios::binary );
  std::stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

// One workspace's worth of caches, kept across the keystrokes as the extension keeps them.
struct Typist
{
  Typist() : em_cache( loader, profile ), inc_cache( loader, profile )
  {
    em_cache.configure( 25 );
    inc_cache.configure( 50 );
  }
  MemoryLoader loader;
  Bscript::Compiler::Profile profile;
  SourceFileCache em_cache;
  SourceFileCache inc_cache;

  // Returns the failure, or an empty string. The script keeps its real path, so that its
  // includes resolve, and the loader serves the typed prefix in place of the file.
  std::string type( const fs::path& file )
  {
    const auto path = file.string();
    const auto words = words_of( read_file( file ) );
    std::string text;
    for ( size_t i = 0; i < words.size(); ++i )
    {
      text += " " + words[i];
      loader.sources[path] = text;
      DiagnosticReporter reporter;
      Report report( reporter );
      Compiler compiler( loader, em_cache, inc_cache, profile );
      try
      {
        compiler.analyze( path, report, false /* is_module */, true /* continue_on_error */ );
      }
      catch ( const std::exception& ex )
      {
        loader.sources.erase( path );
        return fmt::format( "{} [word {}]: {}", path, i + 1, ex.what() );
      }
    }
    loader.sources.erase( path );
    return {};
  }
};

std::vector<fs::path> scripts_under( const fs::path& dir, size_t max_words )
{
  std::vector<fs::path> scripts;
  for ( const auto& entry : fs::recursive_directory_iterator( dir ) )
  {
    if ( !entry.is_regular_file() || entry.path().extension() != ".src" )
      continue;
    if ( max_words && words_of( read_file( entry.path() ) ).size() > max_words )
      continue;
    scripts.push_back( entry.path() );
  }
  std::ranges::sort( scripts );
  return scripts;
}
}  // namespace

void compiler_typing_test()
{
  ScopedCompilerConfig cfg;

  // The test shard's ecompile.cfg names <repo>/pol-core/support/scripts as ModuleDirectory.
  std::error_code ec;
  const auto testsuite = fs::canonical(
      fs::path( Bscript::compilercfg.ModuleDirectory ) / ".." / ".." / ".." / "testsuite", ec );
  if ( ec || !fs::is_directory( testsuite / "escript" ) )
  {
    UnitTest::inc_failures();
    UnitTest::report_failure( "typing: testsuite", Bscript::compilercfg.ModuleDirectory,
                              "a ModuleDirectory inside the repository" );
    return;
  }

  const char* env = std::getenv( "POLCORE_TEST_TYPING" );
  const std::string mode = env ? env : "";
  const bool full = mode == "escript" || mode == "all";
  std::vector<fs::path> scripts =
      scripts_under( testsuite / "escript", full ? 0 : MAX_DEFAULT_WORDS );
  if ( mode == "all" )
  {
    auto pol_scripts = scripts_under( testsuite / "pol", 0 );
    scripts.insert( scripts.end(), pol_scripts.begin(), pol_scripts.end() );
  }

  const auto start = std::chrono::steady_clock::now();
  std::vector<std::string> failures;
  Typist typist;
  for ( const auto& script : scripts )
  {
    if ( full )
      INFO_PRINTLN( "    typing {}", script.string() );
    auto failure = typist.type( script );
    if ( failure.empty() )
      continue;
    if ( full )
      INFO_PRINTLN( "    typing failed: {}", failure );
    failures.push_back( std::move( failure ) );
  }
  const auto seconds =
      std::chrono::duration_cast<std::chrono::seconds>( std::chrono::steady_clock::now() - start )
          .count();
  INFO_PRINTLN( "    typed {} scripts in {}s", scripts.size(), seconds );

  std::string joined;
  for ( const auto& failure : failures )
    joined += "\n      " + failure;
  UnitTest( [&]() { return joined; }, std::string(),
            "typing: analyze() survives every prefix of the test scripts" );
}
}  // namespace Pol::Testing
