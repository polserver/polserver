/** @file
 *
 * @par History
 */

#include <string>

#include "../polwww.h"
#include "testenv.h"

namespace Pol::Testing
{
using namespace Core;

namespace
{
void test_constant_time_equal()
{
#define T_CTEQ( expected, provided, res )                                    \
  UnitTest( []() { return constant_time_equal( expected, provided ); }, res, \
            "constant_time_equal( \"" expected "\", \"" provided "\" )" )

  T_CTEQ( "user:pass", "user:pass", true );
  T_CTEQ( "user:pass", "user:sass", false );  // same length, different content
  T_CTEQ( "user:pass", "user:pas", false );   // provided shorter
  T_CTEQ( "user:pas", "user:pass", false );   // provided longer
  T_CTEQ( "user:pass", "", false );
  T_CTEQ( "", "user:pass", false );
  T_CTEQ( "", "", true );
#undef T_CTEQ
}

void test_html_escape()
{
#define T_ESCAPE( input, res ) \
  UnitTest( []() { return html_escape( input ); }, res, "html_escape( \"" input "\" )" )

  T_ESCAPE( "&", "&amp;" );
  T_ESCAPE( "<", "&lt;" );
  T_ESCAPE( ">", "&gt;" );
  T_ESCAPE( "\"", "&quot;" );
  T_ESCAPE( "<script>alert(\"x&y\")</script>",
            "&lt;script&gt;alert(&quot;x&amp;y&quot;)&lt;/script&gt;" );
  T_ESCAPE( "plain text_1-2.html", "plain text_1-2.html" );
  T_ESCAPE( "", "" );
#undef T_ESCAPE
}

void test_decode_base64()
{
#define T_B64( input, res ) \
  UnitTest( []() { return decode_base64( input ); }, res, "decode_base64( \"" input "\" )" )

  T_B64( "", "" );
  T_B64( "cG9s", "pol" );
  T_B64( "cG9sY29yZTp0ZXN0", "polcore:test" );
  T_B64( "cA==", "p" );   // two pad chars
  T_B64( "cG8=", "po" );  // one pad char
  T_B64( "dXNlcjpwYXNz", "user:pass" );
#undef T_B64
}

void test_http_decodestr()
{
#define T_DECODE( input, res ) \
  UnitTest( []() { return http_decodestr( input ); }, res, "http_decodestr( \"" input "\" )" )

  T_DECODE( "", "" );
  T_DECODE( "a+b+c", "a b c" );
  T_DECODE( "a%2Fb", "a/b" );
  T_DECODE( "a%2fb", "a/b" );  // lowercase hex
  T_DECODE( "a%3d1%26b%3D2", "a=1&b=2" );
  T_DECODE( "plain", "plain" );
  // malformed escapes: returns what was decoded so far
  T_DECODE( "ab%zzcd", "ab" );
  T_DECODE( "ab%", "ab" );
  T_DECODE( "ab%4", "ab" );
#undef T_DECODE
}

void test_legal_pagename()
{
#define T_LEGAL( input, res ) \
  UnitTest( []() { return legal_pagename( input ); }, res, "legal_pagename( \"" input "\" )" )

  T_LEGAL( "/index.html", true );
  T_LEGAL( "/pkg/webserver/static.js", true );
  T_LEGAL( "/a_b-c/d1.htm", true );
  T_LEGAL( "", true );
  T_LEGAL( "/../pol.cfg", false );       // dot not followed by alnum
  T_LEGAL( "/a/..", false );             // trailing dots
  T_LEGAL( "/a.", false );               // trailing single dot
  T_LEGAL( "/bad%20name.html", false );  // '%' not in allowlist
  T_LEGAL( "/a b.html", false );         // space
  T_LEGAL( "/<script>.html", false );    // html metacharacters
  T_LEGAL( "/a&b.html", false );
  T_LEGAL( "/a\"b.html", false );
#undef T_LEGAL
}

void test_get_pagetype()
{
#define T_PAGETYPE( input, res ) \
  UnitTest( []() { return get_pagetype( input ); }, res, "get_pagetype( \"" input "\" )" )

  T_PAGETYPE( "/index.html", "html" );
  T_PAGETYPE( "/a.b/c.ecl", "ecl" );
  T_PAGETYPE( "/noextension", "" );
  T_PAGETYPE( "/dir.d/file", "" );  // last dot before last slash
  T_PAGETYPE( "", "" );
#undef T_PAGETYPE
}

void test_reason_phrase()
{
#define T_REASON( code, res ) \
  UnitTest( []() { return reasonPhrase( code ); }, res, "reasonPhrase( " #code " )" )

  T_REASON( 100, "Continue" );
  T_REASON( 200, "OK" );
  T_REASON( 301, "Moved Permanently" );
  T_REASON( 404, "Not Found" );
  T_REASON( 500, "Internal Server Error" );
  T_REASON( 999, "" );  // unknown code
#undef T_REASON
}
}  // namespace

void www_test()
{
  test_constant_time_equal();
  test_html_escape();
  test_decode_base64();
  test_http_decodestr();
  test_legal_pagename();
  test_get_pagetype();
  test_reason_phrase();
}

}  // namespace Pol::Testing
