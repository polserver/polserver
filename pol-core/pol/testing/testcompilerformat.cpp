/** @file
 *
 * @par History
 */

#include <optional>
#include <stdexcept>
#include <string>

#include "bscript/compiler/file/SourceLocation.h"
#include "pol/testing/testcompileranalysis.h"
#include "pol/testing/testenv.h"

// Compiler::to_formatted_string(), which vscode-escript calls to format a document or a selection.
// ecompile -F never passes a range, so the range handling is reached only from here.

namespace Pol::Testing
{
using namespace Bscript::Compiler;
using namespace CompilerAnalysis;

namespace
{
std::string format_text( const std::string& name, const std::string& body, bool is_module,
                         std::optional<Range> range )
{
  Analysis a;
  const auto path = a.add_source( name, body );
  Compiler compiler( a.loader, a.em_cache, a.inc_cache, a.profile );
  return compiler.to_formatted_string( path, is_module, range );
}

// Two `#` mark the range, as in vscode-escript's format-srcs. The positions are what the extension
// passes: a 1-based line and a 0-based character. The marks are removed from the source.
std::pair<std::string, std::optional<Range>> take_range( const std::string& marked )
{
  std::optional<Position> start;
  std::optional<Position> end;
  std::string source;
  size_t line = 1;
  size_t character = 0;
  for ( char c : marked )
  {
    if ( c == '#' )
    {
      ( start ? end : start ) = Position{ line, character, 0 };
      continue;
    }
    source += c;
    if ( c == '\n' )
    {
      ++line;
      character = 0;
    }
    else
      ++character;
  }
  if ( start && end )
    return { source, Range( *start, *end ) };
  return { source, std::nullopt };
}

struct RangeCase
{
  const char* name;
  const char* marked;
  const char* expected;
};

// vscode-escript's native/test/format-srcs/fmt01-fmt09, which pin the six ways a format-off
// block can meet the range.
const RangeCase range_cases[] = {
    { "whole file",  //
      "// Format whole file.\nVAR a;\nVAR b;\nVAR c;",
      "// Format whole file.\nvar a;\nvar b;\nvar c;\n" },
    { "range inside a format-off block",
      "VAR a;\n// format-off\n#VAR b;#\nVAR c;\n// format-on\nVAR d;\nVAR e;",
      "VAR a;\n// format-off\nVAR b;\nVAR c;\n// format-on\nVAR d;\nVAR e;" },
    { "format-off block starts inside the range",
      "VAR a;\n#VAR b;\n// format-off\nVAR c;#\nVAR d;\n// format-on\nVAR e;\nVAR f;",
      "VAR a;\nvar b;\n// format-off\nVAR c;\nVAR d;\n// format-on\nVAR e;\nVAR f;" },
    { "format-off block before the range",
      "VAR a;\nVAR b;\n// format-off\nVAR c;\nVAR d;\n// format-on\n#VAR e;#\nVAR f;",
      "VAR a;\nVAR b;\n// format-off\nVAR c;\nVAR d;\n// format-on\nvar e;\nVAR f;" },
    { "format-off block inside the range",
      "VAR a;\n#VAR b;\n// format-off\nVAR c;\nVAR d;\n// format-on\nVAR e#;\nVAR f;",
      "VAR a;\nvar b;\n// format-off\nVAR c;\nVAR d;\n// format-on\nvar e;\nVAR f;" },
    { "format-off block ends inside the range",
      "VAR a;\nVAR b;\n// format-off\nVAR c;\n#VAR d;\n// format-on\nVAR e;#\nVAR f;",
      "VAR a;\nVAR b;\n// format-off\nVAR c;\nVAR d;\n// format-on\nvar e;\nVAR f;" },
    { "format-off block after the range",
      "#VAR a;#\nVAR b;\n// format-off\nVAR c;\nVAR d;\n// format-on\nVAR e;\nVAR f;",
      "var a;\nVAR b;\n// format-off\nVAR c;\nVAR d;\n// format-on\nVAR e;\nVAR f;" },
    { "range on the last line",  //
      "VAR a;\nVAR b;\n#VAR f;#", "VAR a;\nVAR b;\nvar f;\n" },
    { "range on the first line", "#VAR a;# // first line\nVAR b;\nVAR c;",
      "var a; // first line\nVAR b;\nVAR c;" },
};

void test_range_cases()
{
  for ( const auto& c : range_cases )
  {
    auto [source, range] = take_range( c.marked );
    UnitTest( [&]() { return format_text( "range.src", source, false /* is_module */, range ); },
              std::string( c.expected ), std::string( "to_formatted_string: " ) + c.name );
  }
}

// Only the marked line is formatted. The lines around it keep their double spaces, which shows
// the range was applied.
void test_module_range()
{
  auto [source, range] = take_range( "Foo(  a );\n#Bar(  b );#\nBaz(  c );\n" );
  UnitTest( [&]() { return format_text( "range.em", source, true /* is_module */, range ); },
            std::string( "Foo(  a );\nBar( b );\nBaz(  c );\n" ),
            "to_formatted_string: range in a module" );
}

// The text after a range keeps its final newline. The fmt cases above all end without one.
void test_range_keeps_final_newline()
{
  auto [source, range] = take_range( "VAR a;\n#VAR b;#\nVAR c;\n" );
  UnitTest( [&]() { return format_text( "newline.src", source, false /* is_module */, range ); },
            std::string( "VAR a;\nvar b;\nVAR c;\n" ),
            "to_formatted_string: range keeps the final newline after it" );
}

// A format-off block that runs to the end of the file gets the final newline only when the whole
// file is formatted. With a range, the text after the range is left exactly as it was.
void test_format_off_to_eof()
{
  const std::string source = "VAR a;\n// format-off\nVAR b;";
  UnitTest( [&]() { return format_text( "eof.src", source, false /* is_module */, std::nullopt ); },
            std::string( "var a;\n// format-off\nVAR b;\n" ),
            "to_formatted_string: format-off to EOF, whole file, newline added" );
  UnitTest(
      [&]()
      {
        return format_text( "eof.src", source, false /* is_module */,
                            Range( Position{ 1, 0, 0 }, Position{ 1, 6, 0 } ) );
      },
      std::string( "var a;\n// format-off\nVAR b;" ),
      "to_formatted_string: format-off to EOF, range, no newline added" );
}

void test_parse_error_throws()
{
  UnitTest(
      [&]() -> std::string
      {
        try
        {
          format_text( "broken.src", "program p(\n", false /* is_module */, std::nullopt );
          return "no exception";
        }
        catch ( const std::runtime_error& ex )
        {
          return std::string( ex.what() ).empty() ? "empty message" : "message";
        }
      },
      std::string( "message" ), "to_formatted_string: a parse error throws its message" );
}
}  // namespace

void compiler_format_test()
{
  ScopedCompilerConfig cfg;

  test_range_cases();
  test_module_range();
  test_range_keeps_final_newline();
  test_format_off_to_eof();
  test_parse_error_throws();
}
}  // namespace Pol::Testing
