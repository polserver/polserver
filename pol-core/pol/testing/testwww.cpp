/** @file
 *
 * @par History
 */

#include <string>

#include "pol/polwww.h"
#include "pol/testing/testenv.h"

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

  // 1xx informational
  T_REASON( 100, "Continue" );
  T_REASON( 101, "Switching Protocols" );
  T_REASON( 102, "Processing" );
  T_REASON( 103, "Early Hints" );

  // 2xx successful
  T_REASON( 200, "OK" );
  T_REASON( 201, "Created" );
  T_REASON( 202, "Accepted" );
  T_REASON( 203, "Non-Authoritative Information" );
  T_REASON( 204, "No Content" );
  T_REASON( 205, "Reset Content" );
  T_REASON( 206, "Partial Content" );
  T_REASON( 207, "Multi-Status" );
  T_REASON( 208, "Already Reported" );
  T_REASON( 226, "IM Used" );

  // 3xx redirection
  T_REASON( 300, "Multiple Choices" );
  T_REASON( 301, "Moved Permanently" );
  T_REASON( 302, "Found" );
  T_REASON( 303, "See Other" );
  T_REASON( 304, "Not Modified" );
  T_REASON( 305, "Use Proxy" );
  T_REASON( 307, "Temporary Redirect" );
  T_REASON( 308, "Permanent Redirect" );

  // 4xx client error
  T_REASON( 400, "Bad Request" );
  T_REASON( 401, "Unauthorized" );
  T_REASON( 402, "Payment Required" );
  T_REASON( 403, "Forbidden" );
  T_REASON( 404, "Not Found" );
  T_REASON( 405, "Method Not Allowed" );
  T_REASON( 406, "Not Acceptable" );
  T_REASON( 407, "Proxy Authentication Required" );
  T_REASON( 408, "Request Timeout" );
  T_REASON( 409, "Conflict" );
  T_REASON( 410, "Gone" );
  T_REASON( 411, "Length Required" );
  T_REASON( 412, "Precondition Failed" );
  T_REASON( 413, "Content Too Large" );
  T_REASON( 414, "URI Too Long" );
  T_REASON( 415, "Unsupported Media Type" );
  T_REASON( 416, "Range Not Satisfiable" );
  T_REASON( 417, "Expectation Failed" );
  T_REASON( 418, "I'm a teapot" );
  T_REASON( 421, "Misdirected Request" );
  T_REASON( 422, "Unprocessable Content" );
  T_REASON( 423, "Locked" );
  T_REASON( 424, "Failed Dependency" );
  T_REASON( 425, "Too Early" );
  T_REASON( 426, "Upgrade Required" );
  T_REASON( 428, "Precondition Required" );
  T_REASON( 429, "Too Many Requests" );
  T_REASON( 431, "Request Header Fields Too Large" );
  T_REASON( 451, "Unavailable For Legal Reasons" );

  // 5xx server error
  T_REASON( 500, "Internal Server Error" );
  T_REASON( 501, "Not Implemented" );
  T_REASON( 502, "Bad Gateway" );
  T_REASON( 503, "Service Unavailable" );
  T_REASON( 504, "Gateway Timeout" );
  T_REASON( 505, "HTTP Version Not Supported" );
  T_REASON( 506, "Variant Also Negotiates" );
  T_REASON( 507, "Insufficient Storage" );
  T_REASON( 508, "Loop Detected" );
  T_REASON( 510, "Not Extended" );
  T_REASON( 511, "Network Authentication Required" );

  // Anything the table does not name is an empty string, whichever class it falls in -- codes
  // the switch skips inside a range it otherwise covers, and codes outside every range.
  T_REASON( 0, "" );
  T_REASON( 104, "" );
  T_REASON( 306, "" );
  T_REASON( 420, "" );
  T_REASON( 509, "" );
  T_REASON( 512, "" );
  T_REASON( 999, "" );
  T_REASON( -1, "" );
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
