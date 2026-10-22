/** @file
 *
 * @par History
 */

#include <string>
#include <vector>

#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/SemanticTokens.h"
#include "pol/testing/testcompileranalysis.h"
#include "pol/testing/testenv.h"

// The semantic tokens and the hidden-token lookup that vscode-escript uses for highlighting and
// for the doc comments it shows on hover. Only Compiler::analyze() builds them.

namespace Pol::Testing
{
using namespace Bscript::Compiler;
using namespace CompilerAnalysis;

namespace
{
// The line and column numbers matter: the cases below ask about positions in it.
const char* const token_fixture =
    "use uo;\n"                                        // 1
    "// line comment\n"                                // 2
    "/* block\n"                                       // 3
    "   comment */\n"                                  // 4
    "program tokens( arg )\n"                          // 5
    "  var x := 0x1F + 1.5;\n"                         // 6
    "  var s := \"plain\";\n"                          // 7
    "  var multi := \"first\n"                         // 8
    "second\";\n"                                      // 9
    "  var re := /ab+c/i;\n"                           // 10
    "  var t := true;\n"                               // 11
    "  foreach item in ( array{ 1 } )\n"               // 12
    "    print( item.length() + s.size );\n"           // 13
    "  endforeach\n"                                   // 14
    "  print( $\"v {x}\" + arg + multi + re + t );\n"  // 15
    "endprogram\n"                                     // 16
    "class Shape()\n"                                  // 17
    "endclass\n";                                      // 18

std::string type_name( SemanticTokenType type )
{
  switch ( type )
  {
  case SemanticTokenType::NAMESPACE:
    return "namespace";
  case SemanticTokenType::TYPE:
    return "type";
  case SemanticTokenType::PARAMETER:
    return "parameter";
  case SemanticTokenType::VARIABLE:
    return "variable";
  case SemanticTokenType::PROPERTY:
    return "property";
  case SemanticTokenType::FUNCTION:
    return "function";
  case SemanticTokenType::KEYWORD:
    return "keyword";
  case SemanticTokenType::STRING:
    return "string";
  case SemanticTokenType::NUMBER:
    return "number";
  case SemanticTokenType::REGEXP:
    return "regexp";
  case SemanticTokenType::OPERATOR:
    return "operator";
  default:
    return "other";
  }
}

// "<type>/<length>" of every token starting at the position, so a failure shows them all.
std::string token_at( const SemanticTokens& tokens, size_t line, size_t column )
{
  std::string result;
  for ( const auto& token : tokens )
  {
    if ( token.line_number == line && token.character_column == column )
    {
      if ( !result.empty() )
        result += ",";
      result += type_name( token.type ) + "/" + std::to_string( token.length );
      if ( !token.modifiers.empty() && token.modifiers.front() == SemanticTokenModifier::READONLY )
        result += "/readonly";
    }
  }
  return result;
}

void test_semantic_tokens()
{
  auto a = analyze_src( "tokens.src", token_fixture );
  UnitTest( [&]() { return a->reporter.diagnostics.size(); }, size_t{ 0 },
            "token fixture has no diagnostics" );
  if ( !a->workspace )
  {
    UnitTest::inc_failures();
    UnitTest::report_failure( "token fixture", "no workspace", "a workspace" );
    return;
  }
  const auto& tokens = a->workspace->tokens;
  auto at = [&]( size_t line, size_t column ) { return token_at( tokens, line, column ); };

  // From the lexer
  UnitTest( [&]() { return at( 1, 1 ); }, std::string( "keyword/3" ), "token: keyword" );
  UnitTest( [&]() { return at( 6, 9 ); }, std::string( "operator/2" ), "token: operator" );
  UnitTest( [&]() { return at( 6, 12 ); }, std::string( "number/4" ), "token: hex number" );
  UnitTest( [&]() { return at( 6, 19 ); }, std::string( "number/3" ), "token: float number" );
  UnitTest( [&]() { return at( 7, 12 ); }, std::string( "string/7" ), "token: string" );
  UnitTest( [&]() { return at( 8, 16 ); }, std::string( "string/6" ),
            "token: multi-line string, first line" );
  UnitTest( [&]() { return at( 9, 1 ); }, std::string( "string/7" ),
            "token: multi-line string, second line" );
  UnitTest( [&]() { return at( 10, 13 ); }, std::string( "regexp/7" ), "token: regexp literal" );
  UnitTest( [&]() { return at( 11, 12 ); }, std::string( "variable/4/readonly" ),
            "token: boolean literal" );
  UnitTest( [&]() { return at( 15, 10 ); }, std::string( "string/2" ),
            "token: interpolated string start" );
  UnitTest( [&]() { return at( 2, 1 ); }, std::string( "" ), "token: comments get none" );

  // From the parse tree
  UnitTest( [&]() { return at( 1, 5 ); }, std::string( "namespace/2" ), "token: used module" );
  UnitTest( [&]() { return at( 5, 9 ); }, std::string( "function/6" ), "token: program name" );
  UnitTest( [&]() { return at( 5, 17 ); }, std::string( "parameter/3" ),
            "token: program parameter" );
  UnitTest( [&]() { return at( 6, 7 ); }, std::string( "variable/1" ),
            "token: variable declaration" );
  UnitTest( [&]() { return at( 12, 11 ); }, std::string( "variable/4" ),
            "token: foreach variable" );
  UnitTest( [&]() { return at( 13, 5 ); }, std::string( "function/5" ), "token: function call" );
  UnitTest( [&]() { return at( 13, 12 ); }, std::string( "variable/4" ),
            "token: variable reference" );
  UnitTest( [&]() { return at( 13, 17 ); }, std::string( "function/6" ), "token: method call" );
  UnitTest( [&]() { return at( 13, 30 ); }, std::string( "property/4" ), "token: member" );
  UnitTest( [&]() { return at( 17, 7 ); }, std::string( "type/5" ), "token: class name" );
}

void test_crlf_string_tokens()
{
  auto a = analyze_src( "crlf.src", "var multi := \"first\r\nsecond\";\r\nprint( multi );\r\n" );
  if ( !a->workspace )
  {
    UnitTest::inc_failures();
    UnitTest::report_failure( "crlf fixture", "no workspace", "a workspace" );
    return;
  }
  const auto& tokens = a->workspace->tokens;
  UnitTest( [&]() { return token_at( tokens, 1, 14 ); }, std::string( "string/6" ),
            "token: CRLF multi-line string, first line" );
  UnitTest( [&]() { return token_at( tokens, 2, 1 ); }, std::string( "string/7" ),
            "token: CRLF multi-line string, second line without the CR" );
}

std::string comments_of( const std::vector<antlr4::Token*>& hidden )
{
  std::string result;
  for ( auto* token : hidden )
  {
    const auto text = token->getText();
    if ( text.starts_with( "/" ) )
      result += ( result.empty() ? "" : "|" ) + text;
  }
  return result;
}

void test_hidden_tokens()
{
  auto a = analyze_src( "hidden.src", token_fixture );
  if ( !a->workspace || !a->workspace->source )
  {
    UnitTest::inc_failures();
    UnitTest::report_failure( "hidden-token fixture", "no source", "a source" );
    return;
  }
  auto& source = *a->workspace->source;
  const std::string both = "// line comment|/* block\n   comment */";

  UnitTest( [&]() { return comments_of( source.get_hidden_tokens_before( Position{ 5, 1, 0 } ) ); },
            both, "get_hidden_tokens_before: the comments above a position" );
  UnitTest( [&]() { return comments_of( source.get_hidden_tokens_before( Position{ 5, 4, 0 } ) ); },
            both, "get_hidden_tokens_before: a position inside the token" );
  UnitTest(
      [&]()
      {
        for ( auto* token : source.get_all_tokens() )
          if ( token->getLine() == 5 && token->getCharPositionInLine() == 0 )
            return comments_of( source.get_hidden_tokens_before( token->getTokenIndex() ) );
        return std::string( "no token" );
      },
      both, "get_hidden_tokens_before: by token index" );
  UnitTest( [&]() { return comments_of( source.get_hidden_tokens_before( Position{ 6, 3, 0 } ) ); },
            std::string( "" ), "get_hidden_tokens_before: no comment above" );
  UnitTest( [&]() { return source.get_hidden_tokens_before( Position{ 99, 1, 0 } ).size(); },
            size_t{ 0 }, "get_hidden_tokens_before: no token at the position" );
}
}  // namespace

void compiler_tokens_test()
{
  ScopedCompilerConfig cfg;

  test_semantic_tokens();
  test_crlf_string_tokens();
  test_hidden_tokens();
}
}  // namespace Pol::Testing
