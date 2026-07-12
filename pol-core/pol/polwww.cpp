/** @file
 *
 * @par History
 * - 2005-08-22 Folko:     Add config/www.cfg, reloaded when select times out (all 5 seconds)
 * - 2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 * - 2008/12/31 Nando:     A call to Register() and some _endthread / pthread_exit were missing
 * - 2009/02/22 Nando:     send_binary() was sending the wrong size on sck.send().
 * - 2009/02/25 Nando:     removed a stray "\n" that was being sent at the end of send_binary(),
 *                         probably causing some corrupted images to be loaded in browsers.
 * - 2010/04/05 Shinigami: Transmit Pointer as Pointer and not Int as Pointer within
 * start_http_conn_thread and http_conn_thread_stub
 */


#include "polwww.h"

#include <ctype.h>
#include <errno.h>
#include <fmt/format.h>
#include <iosfwd>
#include <iterator>
#include <map>
#include <mutex>
#include <string>
#include <time.h>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/esignal.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/network/sockets.h"
#include "../clib/network/wnsckt.h"
#include "../clib/refptr.h"
#include "../clib/stlutil.h"
#include "../clib/threadhelp.h"
#include "../clib/timer.h"

#include "../plib/pkg.h"
#include "../plib/systemstate.h"

#include "globals/uvars.h"
#include "module/httpmod.h"
#include "module/uomod.h"
#include "network/sockio.h"
#include "polsem.h"
#include "scrdef.h"
#include "scrsched.h"
#include "scrstore.h"
#include "uoexec.h"

#ifdef _WIN32
#include <process.h>
#else
#include <pthread.h>
#endif


namespace Pol::Core
{
using namespace threadhelp;

namespace
{
// guards gamestate.mime_types: the listener thread reloads it while worker
// threads look up page types
std::mutex mime_types_mutex;

std::string lookup_mime_type( const std::string& pagetype )
{
  std::lock_guard<std::mutex> guard( mime_types_mutex );
  auto itr = gamestate.mime_types.find( pagetype );
  return itr != gamestate.mime_types.end() ? itr->second : std::string();
}
}  // namespace

void load_mime_config()
{
  static time_t last_load = 0;

  std::map<std::string, std::string> mime_types;

  if ( !Clib::FileExists( "config/www.cfg" ) )
  {
    last_load = 0;
    mime_types["jpg"] = "image/jpeg";
    mime_types["jpeg"] = "image/jpeg";
    mime_types["gif"] = "image/gif";
    mime_types["png"] = "image/png";
    mime_types["js"] = "text/javascript";
    mime_types["ico"] = "image/x-icon";
    mime_types["css"] = "text/css";
    mime_types["json"] = "application/json";
    mime_types["svg"] = "image/svg+xml";
    mime_types["txt"] = "text/plain";
    mime_types["webp"] = "image/webp";
    mime_types["woff2"] = "font/woff2";
    mime_types["wasm"] = "application/wasm";
  }
  else
  {
    try
    {
      Clib::ConfigFile cf( "config/www.cfg" );
      if ( cf.modified() <= last_load )
      {  // not modified
        return;
      }
      last_load = cf.modified();
      Clib::ConfigElem elem;
      while ( cf.read( elem ) )
      {
        std::string ext, mime;
        elem.remove_prop( "Extension", &ext );
        elem.remove_prop( "MIME", &mime );
        mime_types[ext] = mime;
      }
    }
    catch ( ... )
    {
      POLLOG_ERRORLN( "Error while parsing www.cfg" );
      return;
    }
  }

  std::lock_guard<std::mutex> guard( mime_types_mutex );
  gamestate.mime_types.swap( mime_types );
}

void config_web_server()
{
  for ( auto pkg : Plib::systemstate.packages )
  {
    if ( pkg->provides_system_home_page() )
    {
      if ( gamestate.wwwroot_pkg == nullptr )
      {
        POLLOGLN( "wwwroot package is {}", pkg->desc() );
        gamestate.wwwroot_pkg = pkg;
      }
      else
      {
        POLLOGLN( "Package {} also provides a wwwroot, ignoring", pkg->desc() );
      }
    }
  }
  load_mime_config();
}

std::string html_escape( const std::string& s )
{
  std::string escaped;
  escaped.reserve( s.size() );
  for ( char ch : s )
  {
    switch ( ch )
    {
    case '&':
      escaped += "&amp;";
      break;
    case '<':
      escaped += "&lt;";
      break;
    case '>':
      escaped += "&gt;";
      break;
    case '"':
      escaped += "&quot;";
      break;
    default:
      escaped += ch;
    }
  }
  return escaped;
}

// Compares without early exit, so timing does not reveal how much of the
// password matched
bool constant_time_equal( const std::string& expected, const std::string& provided )
{
  unsigned int diff = expected.size() != provided.size() ? 1 : 0;
  for ( size_t i = 0; i < provided.size(); ++i )
  {
    unsigned char e = i < expected.size() ? static_cast<unsigned char>( expected[i] ) : 0;
    diff |= static_cast<unsigned int>( e ^ static_cast<unsigned char>( provided[i] ) );
  }
  return diff == 0;
}

namespace
{
// Builds the complete response and sends it with a single send() call
void http_send_response( Clib::Socket& sck, const std::string& status, const std::string& title,
                         const std::string& message, const std::string& extra_headers = "" )
{
  std::string body = fmt::format(
      "<HTML><HEAD><TITLE>{0}</TITLE></HEAD>\r\n<BODY><H1>{0}</H1>\r\n{1}\r\n</BODY></HTML>\r\n",
      title, message );

  std::string response;
  response.reserve( 160 + extra_headers.size() + body.size() );
  fmt::format_to( std::back_inserter( response ), "HTTP/1.1 {}\r\n", status );
  response += extra_headers;
  response += "Content-Type: text/html\r\n";
  fmt::format_to( std::back_inserter( response ), "Content-Length: {}\r\n", body.size() );
  response += "Connection: close\r\n";
  response += "\r\n";
  response += body;
  sck.send( response.data(), static_cast<unsigned int>( response.size() ) );
}
}  // namespace

void http_forbidden( Clib::Socket& sck )
{
  http_send_response( sck, "403 Forbidden", "Forbidden",
                      "You are forbidden to access this server." );
}

void http_forbidden( Clib::Socket& sck, const std::string& filename )
{
  http_send_response(
      sck, "403 Forbidden", "Forbidden",
      fmt::format( "You are forbidden to access to {} on this server.", html_escape( filename ) ) );
}

void http_not_authorized( Clib::Socket& sck, const std::string& /*filename*/ )
{
  http_send_response( sck, "401 Unauthorized", "Unauthorized",
                      "You are not authorized to access that page.",
                      "WWW-Authenticate: Basic realm=\"pol\"\r\n" );
}

void http_internal_error( Clib::Socket& sck, const std::string& filename )
{
  http_send_response( sck, "500 Internal Server Error", "Internal Server Error",
                      fmt::format( "The requested URL {} caused an internal server error.",
                                   html_escape( filename ) ) );
}

void http_not_found( Clib::Socket& sck, const std::string& filename )
{
  http_send_response( sck, "404 Not Found", "Not Found",
                      fmt::format( "The requested URL {} was not found on this server.",
                                   html_escape( filename ) ) );
}

void http_bad_request( Clib::Socket& sck )
{
  http_send_response( sck, "400 Bad Request", "Bad Request",
                      "The server could not understand the request." );
}

void http_method_not_allowed( Clib::Socket& sck )
{
  http_send_response( sck, "405 Method Not Allowed", "Method Not Allowed",
                      "Only GET requests are supported by this server.", "Allow: GET\r\n" );
}

void http_redirect( Clib::Socket& sck, const std::string& new_url )
{
  http_send_response(
      sck, "301 Moved Permanently", "Moved Permanently",
      fmt::format( "The requested URL has been moved to {}", html_escape( new_url ) ),
      fmt::format( "Location: {}\r\n", new_url ) );
}

std::string reasonPhrase( int code )
{
  switch ( code )
  {
  // ####### 1xx - Informational #######
  case 100:
    return "Continue";
  case 101:
    return "Switching Protocols";
  case 102:
    return "Processing";
  case 103:
    return "Early Hints";

  // ####### 2xx - Successful #######
  case 200:
    return "OK";
  case 201:
    return "Created";
  case 202:
    return "Accepted";
  case 203:
    return "Non-Authoritative Information";
  case 204:
    return "No Content";
  case 205:
    return "Reset Content";
  case 206:
    return "Partial Content";
  case 207:
    return "Multi-Status";
  case 208:
    return "Already Reported";
  case 226:
    return "IM Used";

  // ####### 3xx - Redirection #######
  case 300:
    return "Multiple Choices";
  case 301:
    return "Moved Permanently";
  case 302:
    return "Found";
  case 303:
    return "See Other";
  case 304:
    return "Not Modified";
  case 305:
    return "Use Proxy";
  case 307:
    return "Temporary Redirect";
  case 308:
    return "Permanent Redirect";

  // ####### 4xx - Client Error #######
  case 400:
    return "Bad Request";
  case 401:
    return "Unauthorized";
  case 402:
    return "Payment Required";
  case 403:
    return "Forbidden";
  case 404:
    return "Not Found";
  case 405:
    return "Method Not Allowed";
  case 406:
    return "Not Acceptable";
  case 407:
    return "Proxy Authentication Required";
  case 408:
    return "Request Timeout";
  case 409:
    return "Conflict";
  case 410:
    return "Gone";
  case 411:
    return "Length Required";
  case 412:
    return "Precondition Failed";
  case 413:
    return "Content Too Large";
  case 414:
    return "URI Too Long";
  case 415:
    return "Unsupported Media Type";
  case 416:
    return "Range Not Satisfiable";
  case 417:
    return "Expectation Failed";
  case 418:
    return "I'm a teapot";
  case 421:
    return "Misdirected Request";
  case 422:
    return "Unprocessable Content";
  case 423:
    return "Locked";
  case 424:
    return "Failed Dependency";
  case 425:
    return "Too Early";
  case 426:
    return "Upgrade Required";
  case 428:
    return "Precondition Required";
  case 429:
    return "Too Many Requests";
  case 431:
    return "Request Header Fields Too Large";
  case 451:
    return "Unavailable For Legal Reasons";

  // ####### 5xx - Server Error #######
  case 500:
    return "Internal Server Error";
  case 501:
    return "Not Implemented";
  case 502:
    return "Bad Gateway";
  case 503:
    return "Service Unavailable";
  case 504:
    return "Gateway Timeout";
  case 505:
    return "HTTP Version Not Supported";
  case 506:
    return "Variant Also Negotiates";
  case 507:
    return "Insufficient Storage";
  case 508:
    return "Loop Detected";
  case 510:
    return "Not Extended";
  case 511:
    return "Network Authentication Required";

  default:
    return std::string();
  }
}

// http_decodestr: turn all those %2F etc into what they represent
// rules:
//  '+'   ->   ' '
//  %HH   ->   (hex value)
//  other ->   itself
std::string http_decodestr( const std::string& s )
{
  std::string decoded;
  const char* t = s.c_str();
  while ( t != nullptr && *t != '\0' )
  {
    if ( *t == '+' )
    {
      decoded.append( 1, ' ' );
      ++t;
    }
    else if ( *t != '%' )
    {
      decoded.append( 1, *t );
      ++t;
    }
    else
    {
      // if first is null terminator, won't look at second
      if ( isxdigit( static_cast<unsigned char>( *( t + 1 ) ) ) &&
           isxdigit( static_cast<unsigned char>( *( t + 2 ) ) ) )
      {
        unsigned char chH = *( t + 1 );
        unsigned char chL = *( t + 2 );
        t += 3;
        char ch = 0;
        if ( isdigit( chH ) )
          ch = ( chH - '0' ) << 4;
        else
          ch = ( ( static_cast<char>( tolower( chH ) ) - 'a' ) + 10 ) << 4;

        if ( isdigit( chL ) )
          ch |= ( chL - '0' );
        else
          ch |= ( tolower( chL ) - 'a' ) + 10;

        decoded.append( 1, ch );
      }
      else  // garbage - just return what we have so far.
      {
        return decoded;
      }
    }
  }
  return decoded;
}

unsigned char cvt_8to6( char ch )
{
  if ( ch >= 'A' && ch <= 'Z' )
    return ch - 'A';
  if ( ch >= 'a' && ch <= 'z' )
    return ch - 'a' + 26;
  if ( ch >= '0' && ch <= '9' )
    return ch - '0' + 52;
  if ( ch == '+' )
    return 62;
  if ( ch == '/' )
    return 63;
  if ( ch == '=' )
    return 0x40;  // pad
  return 0x80;    // error
}

std::string decode_base64( const std::string& b64s )
{
  std::string s;
  const char* t = b64s.c_str();
  char c[4];
  char x[3];
  char b[4];
  while ( *t )
  {
    c[0] = *t++;
    c[1] = *t ? *t++ : '\0';
    c[2] = *t ? *t++ : '\0';
    c[3] = *t ? *t++ : '\0';

    b[0] = cvt_8to6( c[0] );
    b[1] = cvt_8to6( c[1] );
    b[2] = cvt_8to6( c[2] );
    b[3] = cvt_8to6( c[3] );

    x[0] = ( ( b[0] << 2 ) & 0xFC ) | ( ( b[1] >> 4 ) & 0x03 );
    x[1] = ( ( b[1] << 4 ) & 0xF0 ) | ( ( b[2] >> 2 ) & 0x0F );
    x[2] = ( ( b[2] << 6 ) & 0xC0 ) | ( b[3] & 0x3F );

    if ( x[0] )
      s.append( 1, x[0] );
    if ( x[1] )
      s.append( 1, x[1] );
    if ( x[2] )
      s.append( 1, x[2] );
  }
  return s;
}

bool legal_pagename( const std::string& page )
{
  // make sure the page isn't going to go visit our hard disk
  for ( const char* t = page.c_str(); *t; ++t )
  {
    unsigned char ch = *t;
    if ( isalnum( ch ) || ( ch == '/' ) || ( ch == '_' ) || ( ch == '-' ) )
    {
      continue;
    }
    if ( ( ch == '.' ) && ( isalnum( static_cast<unsigned char>( *( t + 1 ) ) ) ) )
    {
      continue;
    }

    return false;
  }
  return true;
}

// get_pagetype: extract the extension, basically
std::string get_pagetype( const std::string& page )
{
  std::string::size_type lastslash = page.rfind( '/' );
  std::string::size_type dotpos = page.rfind( '.' );

  if ( lastslash != std::string::npos && dotpos != std::string::npos && lastslash > dotpos )
    return "";

  if ( dotpos != std::string::npos )
  {
    return page.substr( dotpos + 1 );
  }

  return "";
}

// FIXME this is just ugly!  The HttpExecutorModule takes ownership of the
// socket, through the Socket copy-constructor..  But sometimes, after we've
// built it, we need to send data (on errors).
bool start_http_script( Clib::Socket& sck, const std::string& page, Plib::Package* pkg,
                        const std::string& file_ecl, const std::string& query_string )
{
  bool res = true;

  ScriptDef page_sd;
  if ( pkg )
    page_sd.quickconfig( pkg, file_ecl );
  else
    page_sd.quickconfig( file_ecl );

  if ( !page_sd.exists() )
  {
    POLLOGLN( "WebServer: not found: {}", page_sd.name() );
    http_not_found( sck, page );
    return false;
  }

  PolLock2 lck;

  ref_ptr<Bscript::EScriptProgram> program =
      find_script2( page_sd, true, Plib::systemstate.config.cache_interactive_scripts );
  // find_script( filename, true, config.cache_interactive_scripts );
  if ( program.get() == nullptr )
  {
    ERROR_PRINTLN( "Error reading script {}", page_sd.name() );
    res = false;
    lck.unlock();
    http_not_found( sck, page );
    lck.lock();
  }
  else
  {
    UOExecutor* ex = create_script_executor();
    Module::UOExecutorModule* uoemod = new Module::UOExecutorModule( *ex );
    ex->addModule( uoemod );
    Module::HttpExecutorModule* hem = new Module::HttpExecutorModule( *ex, std::move( sck ) );

    hem->read_query_string( query_string );
    hem->read_query_ip();

    ex->addModule( hem );

    if ( !ex->setProgram( program.get() ) )
    {
      lck.unlock();
      http_not_found( hem->sck_, page );
      lck.lock();
      delete ex;
      res = false;
    }
    else
    {
      // Sending of HTTP status line and headers moved to module function
      ex->setDebugLevel( Bscript::Executor::NONE );
      schedule_executor( ex );
    }
  }
  program.clear();  // do this so deletion happens while we're locked
  lck.unlock();
  return res;
}

std::string get_page_filename( const std::string& page )
{
  std::string filename = "scripts/www" + page;
  return filename;
}

bool decode_page( const std::string& ipage, Plib::Package** ppkg, std::string* pfilename,
                  std::string* ppagetype, std::string* redirect_to )
{
  std::string page = ipage;
  Plib::Package* pkg = nullptr;
  std::string filedir;
  std::string retdir;

  if ( page.substr( 0, 5 ) == "/pkg/" )
  {
    // cerr << "package page: " << page << endl;
    std::string::size_type pkgname_end = page.find_first_of( '/', 5 );
    std::string pkgname;
    if ( pkgname_end != std::string::npos )
    {
      pkgname = page.substr( 5, pkgname_end - 5 );
      page = page.substr( pkgname_end );
    }
    else
    {
      pkgname = page.substr( 5 );
      page = "/";
    }

    // cerr << "pkg name: " << pkgname << endl;
    pkg = Plib::find_package( pkgname );
    if ( pkg == nullptr )
      return false;

    filedir = pkg->dir() + "www";
  }
  else
  {
    if ( gamestate.wwwroot_pkg != nullptr )
    {
      filedir = gamestate.wwwroot_pkg->dir() + "www";
      retdir = gamestate.wwwroot_pkg->dir() + "www";
    }
    else
    {
      filedir = "scripts/www";
      retdir = "scripts/www";
    }
  }

  std::string filename = filedir + page;

  std::string pagetype = get_pagetype( page );

  if ( pagetype.empty() )  // didn't specify, so assume it's a directory.
  {                        // have to redirect...
    page = ipage;
    if ( page.empty() || page[page.size() - 1] != '/' )
    {
      page += "/";
      // filename += "/";
    }

    std::string test;
    test = filename + "index.ecl";
    if ( Clib::FileExists( test.c_str() ) )
    {
      page += "index.ecl";
    }
    else
    {
      page += "index.htm";
    }
    *redirect_to = page;
    return true;
  }

  if ( pkg )
  {
    if ( pagetype == "ecl" )
    {
      // uh, I don't get this:
      retdir = "www";
    }
    else
    {
      retdir = pkg->dir() + "www";
    }
  }

  *ppkg = pkg;
  *pfilename = retdir + page;
  *ppagetype = pagetype;
  return true;
}

void send_binary( Clib::Socket& sck, const std::string& page, const std::string& filename,
                  const std::string& content_type )
{
  unsigned int fsize = Clib::filesize( filename.c_str() );
  std::ifstream ifs( filename.c_str(), std::ios::binary );
  if ( !ifs.is_open() ) {
    http_not_found( sck, page );
    return;
  }

  std::string headers = fmt::format(
      "HTTP/1.1 200 OK\r\n"
      "Accept-Ranges: bytes\r\n"
      "Content-Length: {}\r\n"
      "Content-Type: {}\r\n"
      "Connection: close\r\n"
      "\r\n",
      fsize, content_type );
  sck.send( headers.data(), static_cast<unsigned int>( headers.size() ) );

  // Actual reading and outputting.
  char bfr[32768];
  unsigned int cur_read = 0;
  while ( sck.connected() && ifs.good() && cur_read < fsize )
  {
    ifs.read( bfr, sizeof( bfr ) );
    cur_read += static_cast<unsigned int>( ifs.gcount() );
    sck.send( bfr, static_cast<unsigned int>( ifs.gcount() ) ); 
  }
}

void send_html( Clib::Socket& sck, const std::string& page, const std::string& filename )
{
  send_binary( sck, page, filename, "text/html" );
}

void http_func( SOCKET client_socket )
{
  Clib::Socket sck( client_socket );
  Clib::SocketLineReader lineReader( sck, 5, 3000,
                                     false );  // we take care of disconnecting at timeout

  std::string request_line;
  std::string auth;
  std::string tmpstr;
  std::string host;

  if ( Plib::systemstate.config.web_server_local_only && !sck.is_local() )
  {
    http_forbidden( sck );
    return;
  }

  bool timed_out = false;
  bool first_line = true;
  unsigned int header_count = 0;
  const unsigned int max_header_count = 64;
  const auto max_request_time = std::chrono::seconds( 10 );
  Tools::HighPerfTimer requestTimer;
  while ( sck.connected() && lineReader.read( tmpstr, &timed_out ) )
  {
    if ( Plib::systemstate.config.web_server_debug )
      INFO_PRINTLN( "http({}): '{}'", sck.handle(), tmpstr );
    if ( tmpstr.empty() )
      break;
    if ( ++header_count > max_header_count || requestTimer.ellapsed() > max_request_time )
    {
      INFO_PRINTLN( "HTTP connection {} exceeded request limits", sck.getpeername() );
      http_bad_request( sck );
      return;
    }
    if ( first_line )
    {
      request_line = tmpstr;
      first_line = false;
      continue;
    }
    if ( strncmp( tmpstr.c_str(), "Authorization:", 14 ) == 0 )
      auth = tmpstr;
    else if ( strncmp( tmpstr.c_str(), "Host: ", 6 ) == 0 )
      host = tmpstr.substr( 6 );
  }

  if ( timed_out )
  {
    INFO_PRINTLN( "HTTP connection {} timed out", sck.getpeername() );
    sck.close();
  }

  if ( !sck.connected() )
    return;

  if ( Plib::systemstate.config.web_server_debug )
  {
    INFO_PRINTLN( "[{} msec] finished reading header",
                  double( requestTimer.ellapsed().count() / 1000.0 ) );
  }

  ISTRINGSTREAM is( request_line );

  std::string cmd;           // GET, POST  (we only handle GET)
  std::string url;           // The whole URL (xx.ecl?a=b&c=d)
  std::string proto;         // HTTP/1.1
  std::string page;          // xx.ecl
  std::string query_string;  // a=b&c=d

  is >> cmd >> url >> proto;

  if ( Plib::systemstate.config.web_server_debug )
  {
    INFO_PRINTLN(
        "http-cmd:   '{}'\n"
        "http-host:  '{}'\n"
        "http-url:   '{}'\n"
        "http-proto: '{}'",
        cmd, host, url, proto );
  }

  if ( cmd.empty() || url.empty() )
  {
    http_bad_request( sck );
    return;
  }
  if ( cmd != "GET" )
  {
    http_method_not_allowed( sck );
    return;
  }

  //  if (url == "/")
  //    url = "/index.htm";

  // spliturl( url, page, params ); ??
  std::string::size_type ques = url.find( '?' );

  if ( ques == std::string::npos )
  {
    page = url;
    query_string = "";
  }
  else
  {
    page = url.substr( 0, ques );
    query_string = url.substr( ques + 1 );
  }

  if ( Plib::systemstate.config.web_server_debug )
  {
    INFO_PRINTLN(
        "http-page:   '{}'\n"
        "http-params: '{}'\n"
        "http-decode: '{}'",
        page, query_string, http_decodestr( query_string ) );
  }

  if ( !Plib::systemstate.config.web_server_password.empty() )
  {
    if ( !auth.empty() )
    {
      ISTRINGSTREAM is2( auth );
      std::string _auth, type, coded_unpw, unpw;
      is2 >> _auth >> type >> coded_unpw;
      unpw = decode_base64( coded_unpw );
      if ( Plib::systemstate.config.web_server_debug )
      {
        INFO_PRINTLN(
            "http-pw: '{}'\n"
            "http-pw-decoded: '{}'",
            coded_unpw, unpw );
      }
      if ( !constant_time_equal( Plib::systemstate.config.web_server_password, unpw ) )
      {
        http_not_authorized( sck, url );
        return;
      }
    }
    else
    {
      http_not_authorized( sck, url );
      return;
    }
  }

  if ( !legal_pagename( page ) )
  {
    // FIXME should probably be access denied
    http_forbidden( sck, page );
    return;
  }


  Plib::Package* pkg = nullptr;
  std::string filename;
  std::string pagetype;
  std::string redirect_to;
  if ( !decode_page( page, &pkg, &filename, &pagetype, &redirect_to ) )
  {
    http_not_found( sck, page );
    return;
  }
  if ( !redirect_to.empty() )
  {
    http_redirect( sck, /*"http://" + host +*/ redirect_to );
    return;
  }

  if ( Plib::systemstate.config.web_server_debug )
    INFO_PRINTLN( "Page type: {}", pagetype );

  if ( pagetype == "ecl" )
  {
    // Note it takes ownership of the socket
    start_http_script( sck, page, pkg, filename, query_string );
  }
  else if ( pagetype == "htm" || pagetype == "html" )
  {
    send_html( sck, page, filename );
  }
  else
  {
    std::string type = lookup_mime_type( pagetype );
    if ( !type.empty() )
    {
      send_binary( sck, page, filename, type );
    }
    else
    {
      POLLOG_INFOLN( "HTTP server: I can't handle pagetype '{}'", pagetype );
      http_internal_error( sck, page );
    }
  }
}


#ifdef _WIN32
void init_http_thread_support() {}
#else
pthread_attr_t http_attr;
void init_http_thread_support()
{
  pthread_attr_init( &http_attr );
  pthread_attr_setdetachstate( &http_attr, PTHREAD_CREATE_DETACHED );
}
#endif

void http_thread()
{
  config_web_server();
  init_http_thread_support();

  if ( Plib::systemstate.config.web_server_local_only )
    INFO_PRINTLN( "Listening for HTTP requests on 127.0.0.1:{} (local only)",
                  Plib::systemstate.config.web_server_port );
  else
    INFO_PRINTLN( "Listening for HTTP requests on port {}",
                  Plib::systemstate.config.web_server_port );

  Clib::Socket listen_sck;
  if ( !listen_sck.listen( Plib::systemstate.config.web_server_port,
                           Plib::systemstate.config.web_server_local_only ) )
  {
    ERROR_PRINTLN( "Unable to listen on HTTP port {}",
                   Plib::systemstate.config.web_server_port );
    return;  // webserver off, but the rest of the server keeps running
  }

  {
    Pol::threadhelp::TaskThreadPool worker_threads( 4, "http" );
    while ( !Clib::exit_signalled )
    {
      if ( !listen_sck.has_incoming_data( 5000 ) )
      {
        load_mime_config();  // hot-reload the MIME config while idle
        continue;
      }

      if ( Plib::systemstate.config.web_server_debug )
        INFO_PRINTLN( "Accepting connection.." );

      Clib::Socket client;
      try
      {
        if ( !listen_sck.accept( &client ) || !client.connected() )
          continue;
        // Always disable Nagle on webserver clients; config.disable_nagle gates UO clients only.
        client.disable_nagle();
      }
      catch ( std::exception& ex )
      {
        POLLOG_ERRORLN( "HTTP server: failed to set socket options: {}", ex.what() );
        continue;  // client closes via RAII
      }

      struct sockaddr client_addr = client.peer_address();
      INFO_PRINTLN( "HTTP client connected from {}", Network::AddressToString( &client_addr ) );

      // Transfer the handle out of the stack Socket; http_func re-wraps it into a Clib::Socket.
      SOCKET client_socket = client.release_handle();
      worker_threads.push( [client_socket]() { http_func( client_socket ); } );
    }
  }  // wait for the worker threads to finish before cleanup

  gamestate.mime_types.clear();  // cleanup on exit
}

void start_http_server()
{
  threadhelp::start_thread( http_thread, "HTTP" );
}
}  // namespace Pol::Core
