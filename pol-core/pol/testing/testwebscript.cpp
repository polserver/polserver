/** @file
 *
 * @par History
 */

#include <string>

#include "pol/testing/testenv.h"

// Both live in SourceFile.cpp and are declared there rather than in a header. They are the
// whole of the .hsr / .asp web page support, which nothing else exercises.
namespace Pol::Bscript::Compiler
{
bool is_web_script( const char* filename );
std::string preprocess_web_script( const std::string& input );
}  // namespace Pol::Bscript::Compiler

namespace Pol::Testing
{
using Bscript::Compiler::is_web_script;
using Bscript::Compiler::preprocess_web_script;

namespace
{
// Every page starts with this, whether or not it holds any code.
const std::string header = "use http;\n";

void test_is_web_script()
{
  UnitTest( []() { return is_web_script( "page.hsr" ); }, true, ".hsr is a web script" );
  UnitTest( []() { return is_web_script( "page.asp" ); }, true, ".asp is a web script" );
  UnitTest( []() { return is_web_script( "dir/sub/page.asp" ); }, true,
            "a full path is a web script" );

  UnitTest( []() { return is_web_script( "page.src" ); }, false, ".src is not a web script" );
  UnitTest( []() { return is_web_script( "page.htm" ); }, false, ".htm is not a web script" );
  UnitTest( []() { return is_web_script( "page.html" ); }, false, ".html is not a web script" );
  UnitTest( []() { return is_web_script( "page.inc" ); }, false, ".inc is not a web script" );
  UnitTest( []() { return is_web_script( "" ); }, false, "an empty name is not a web script" );

  // The extension is matched with its terminating NUL, so anything after it misses.
  UnitTest( []() { return is_web_script( "page.aspx" ); }, false, ".aspx is not a web script" );
  UnitTest( []() { return is_web_script( "page.hsrc" ); }, false, ".hsrc is not a web script" );
  UnitTest( []() { return is_web_script( "page.asp.txt" ); }, false,
            ".asp before another extension is not a web script" );
  UnitTest( []() { return is_web_script( "dir.asp/page.src" ); }, false,
            ".asp as a directory name is not a web script" );

  // Matched anywhere in the name, so a page may be named after another extension.
  UnitTest( []() { return is_web_script( "page.src.asp" ); }, true,
            ".asp still wins at the end of the name" );
}

void test_preprocess_plain_html()
{
  UnitTest( []() { return preprocess_web_script( "" ); }, header, "an empty page is just the use" );

  UnitTest( []() { return preprocess_web_script( "hello" ); },
            header + "WriteHtmlRaw( \"hello\");\n", "a page with no code is one write" );

  // \n is escaped into the generated string literal and \r is dropped entirely.
  UnitTest( []() { return preprocess_web_script( "a\r\nb" ); },
            header + "WriteHtmlRaw( \"a\\nb\");\n", "CR is dropped and LF is escaped" );

  // A double quote would otherwise end the generated literal.
  UnitTest( []() { return preprocess_web_script( "say \"hi\"" ); },
            header + "WriteHtmlRaw( \"say \\\"hi\\\"\");\n", "quotes are escaped" );
}

void test_preprocess_code_block()
{
  // <% %> emits its body as bare script, with no write around it.
  UnitTest( []() { return preprocess_web_script( "<% var x := 5; %>" ); }, header + " var x := 5; ",
            "a code block is emitted as script" );

  // The html on either side becomes a write, and the code keeps its place between them.
  UnitTest( []() { return preprocess_web_script( "a<% code %>b" ); },
            header + "WriteHtmlRaw( \"a\");\n code WriteHtmlRaw( \"b\");\n",
            "html around a code block becomes writes" );
}

void test_preprocess_emit_block()
{
  // <%= %> wraps its body in a write instead.
  UnitTest( []() { return preprocess_web_script( "<%= x %>" ); }, header + "WriteHtmlRaw(  x  );\n",
            "an emit block is wrapped in a write" );

  UnitTest( []() { return preprocess_web_script( "<b><%= x %></b>" ); },
            header + "WriteHtmlRaw( \"<b>\");\nWriteHtmlRaw(  x  );\nWriteHtmlRaw( \"</b>\");\n",
            "an emit block between html" );

  // The = is only an emit marker directly after <%.
  UnitTest( []() { return preprocess_web_script( "<% =x %>" ); }, header + " =x ",
            "an = after a space is ordinary code" );
}

void test_preprocess_unterminated()
{
  // A code block with no closing %> runs to the end of the page and is still emitted.
  UnitTest( []() { return preprocess_web_script( "a<% code" ); },
            header + "WriteHtmlRaw( \"a\");\n code", "an unterminated code block is emitted" );

  // An unterminated emit block loses its closing paren, which is a compile error downstream
  // rather than anything this function reports.
  UnitTest( []() { return preprocess_web_script( "<%= x" ); }, header + "WriteHtmlRaw(  x",
            "an unterminated emit block is left open" );

  // A trailing < or <% at the very end is not read past.
  UnitTest( []() { return preprocess_web_script( "a<" ); }, header + "WriteHtmlRaw( \"a<\");\n",
            "a trailing < is ordinary html" );
  UnitTest( []() { return preprocess_web_script( "a<%" ); }, header + "WriteHtmlRaw( \"a\");\n",
            "a trailing <% opens an empty code block" );
}

void test_preprocess_multiple_blocks()
{
  UnitTest( []() { return preprocess_web_script( "<% a %>a<% b %>" ); },
            header + " a WriteHtmlRaw( \"a\");\n b ", "several code blocks keep their order" );

  // A % inside a code block is only special when followed by >.
  UnitTest( []() { return preprocess_web_script( "<% a % b %>" ); }, header + " a % b ",
            "a bare % inside code is kept" );
}
}  // namespace

void webscript_test()
{
  test_is_web_script();
  test_preprocess_plain_html();
  test_preprocess_code_block();
  test_preprocess_emit_block();
  test_preprocess_unterminated();
  test_preprocess_multiple_blocks();
}
}  // namespace Pol::Testing
