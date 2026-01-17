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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <iosfwd>
#include <string>
#include <time.h>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/esignal.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/network/sockets.h"
#include "../clib/network/wnsckt.h"
#include "../clib/passert.h"
#include "../clib/refptr.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
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


#ifdef _MSC_VER
#pragma warning( disable : 4127 )  // conditional expression is constant (needed because of FD_SET)
#endif


namespace Pol::Core
{
using namespace threadhelp;

void load_mime_config()
{
  static time_t last_load = 0;

  if ( !Clib::FileExists( "config/www.cfg" ) )
  {
    if ( last_load )
    {
      gamestate.mime_types.clear();
      last_load = 0;
    }
    gamestate.mime_types["jpg"] = "image/jpeg";
    gamestate.mime_types["jpeg"] = "image/jpeg";
    gamestate.mime_types["gif"] = "image/gif";
    gamestate.mime_types["png"] = "image/png";
    gamestate.mime_types["js"] = "text/javascript";
    gamestate.mime_types["ico"] = "image/x-icon";
    return;
  }

  try
  {
    Clib::ConfigFile cf( "config/www.cfg" );
    if ( cf.modified() <= last_load )
    {  // not modified
      return;
    }
    last_load = cf.modified();
    gamestate.mime_types.clear();
    Clib::ConfigElem elem;
    while ( cf.read( elem ) )
    {
      std::string ext, mime;
      elem.remove_prop( "Extension", &ext );
      elem.remove_prop( "MIME", &mime );
      gamestate.mime_types[ext] = mime;
    }
  }
  catch ( ... )
  {
    POLLOG_ERRORLN( "Error while parsing www.cfg" );
  }
}

void config_web_server()
{
  for ( Plib::Packages::iterator itr = Plib::systemstate.packages.begin();
        itr != Plib::systemstate.packages.end(); ++itr )
  {
    Plib::Package* pkg = ( *itr );
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

void http_writeline( Clib::Socket& sck, const std::string& s )
{
  sck.send( (void*)s.c_str(), static_cast<unsigned int>( s.length() ) );
  sck.send( "\r\n", 2 );
}

void http_forbidden( Clib::Socket& sck )
{
  http_writeline( sck, "HTTP/1.1 403 Forbidden" );
  http_writeline( sck, "Content-Type: text/html" );
  http_writeline( sck, "" );
  http_writeline( sck, "<HTML><HEAD><TITLE>403 Forbidden</TITLE></HEAD>" );
  http_writeline( sck, "<BODY><H1>Forbidden</H1>" );
  http_writeline( sck, "You are forbidden to access this server." );
  http_writeline( sck, "</BODY></HTML>" );
}

void http_forbidden( Clib::Socket& sck, const std::string& filename )
{
  http_writeline( sck, "HTTP/1.1 403 Forbidden" );
  http_writeline( sck, "Content-Type: text/html" );
  http_writeline( sck, "" );
  http_writeline( sck, "<HTML><HEAD><TITLE>403 Forbidden</TITLE></HEAD>" );
  http_writeline( sck, "<BODY><H1>Forbidden</H1>" );
  http_writeline( sck, "You are forbidden to access to " + filename + " on this server." );
  http_writeline( sck, "</BODY></HTML>" );
}

void http_not_authorized( Clib::Socket& sck, const std::string& /*filename*/ )
{
  http_writeline( sck, "HTTP/1.1 401 Unauthorized" );
  http_writeline( sck, "WWW-Authenticate: Basic realm=\"pol\"" );
  http_writeline( sck, "Content-Type: text/html" );
  http_writeline( sck, "" );
  http_writeline( sck, "<HTML><HEAD><TITLE>401 Unauthorized</TITLE></HEAD>" );
  http_writeline( sck, "<BODY><H1>Unauthorized</H1>" );
  http_writeline( sck, "You are not authorized to access that page." );
  http_writeline( sck, "</BODY></HTML>" );
}

void http_internal_error( Clib::Socket& sck, const std::string& filename )
{
  http_writeline( sck, "HTTP/1.1 500 Internal Sever Error" );
  http_writeline( sck, "Content-Type: text/html" );
  http_writeline( sck, "" );
  http_writeline( sck, "<HTML><HEAD><TITLE>500 Internal Server Error</TITLE></HEAD>" );
  http_writeline( sck, "<BODY><H1>Internal Server Error</H1>" );
  http_writeline( sck, "The requested URL " + filename + " caused an internal server error." );
  http_writeline( sck, "</BODY></HTML>" );
}

void http_not_found( Clib::Socket& sck, const std::string& filename )
{
  http_writeline( sck, "HTTP/1.1 404 Not Found" );
  http_writeline( sck, "Content-Type: text/html" );
  http_writeline( sck, "" );
  http_writeline( sck, "<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>" );
  http_writeline( sck, "<BODY><H1>Not Found</H1>" );
  http_writeline( sck, "The requested URL " + filename + " was not found on this server." );
  http_writeline( sck, "</BODY></HTML>" );
}

void http_redirect( Clib::Socket& sck, const std::string& new_url )
{
  // cerr << "http: redirecting to " << new_url << endl;


  http_writeline( sck, "HTTP/1.1 301 Moved Permanently" );
  http_writeline( sck, "Location: " + new_url );
  http_writeline( sck, "" );
  http_writeline( sck, "<HTML><HEAD><TITLE>301 Moved Permanently</TITLE></HEAD>" );
  http_writeline( sck, "<BODY><H1>Moved Permanently</H1>" );
  http_writeline( sck, "The requested URL has been moved to " + new_url );
  http_writeline( sck, "</BODY></HTML>" );
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
      if ( isxdigit( *( t + 1 ) ) && isxdigit( *( t + 2 ) ) )
      {
        char chH = *( t + 1 );
        char chL = *( t + 2 );
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
  else if ( ch == '/' )
    return 63;
  else if ( ch == '=' )
    return 0x40;  // pad
  else
    return 0x80;  // error
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
    char ch = *t;
    if ( isalnum( ch ) || ( ch == '/' ) || ( ch == '_' ) || ( ch == '-' ) )
    {
      continue;
    }
    if ( ( ch == '.' ) && ( isalnum( *( t + 1 ) ) ) )
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

bool get_script_page_filename( const std::string& page, ScriptDef& sd )
{
  if ( page.substr( 0, 5 ) == "/pkg/" )
  {
    // cerr << "package page script: " << page << endl;
    auto pkgname_end = page.find_first_of( '/', 5 );
    if ( pkgname_end != std::string::npos )
    {
      std::string pkg_name = page.substr( 5, pkgname_end - 5 );
      // cerr << "pkg name: " << pkg_name << endl;
      Plib::Package* pkg = Plib::find_package( pkg_name );
      if ( pkg != nullptr )
      {
        sd.quickconfig( pkg, "www/" + page.substr( pkgname_end + 1 ) );
        return true;
      }

      return false;
    }

    return false;
  }

  sd.quickconfig( "scripts/www" + page + ".ecl" );
  return true;
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

void send_html( Clib::Socket& sck, const std::string& page, const std::string& filename )
{
  std::ifstream ifs( filename.c_str() );
  if ( ifs.is_open() )
  {
    http_writeline( sck, "HTTP/1.1 200 OK" );
    http_writeline( sck, "Content-Type: text/html" );
    http_writeline( sck, "" );
    std::string t;
    while ( getline( ifs, t ) )
    {
      http_writeline( sck, t );
    }
  }
  else
  {
    http_not_found( sck, page );
  }
}

void send_binary( Clib::Socket& sck, const std::string& page, const std::string& filename,
                  const std::string& content_type )
{
  // string filename = get_page_filename( page );
  unsigned int fsize = Clib::filesize( filename.c_str() );
  std::ifstream ifs( filename.c_str(), std::ios::binary );
  if ( ifs.is_open() )
  {
    http_writeline( sck, "HTTP/1.1 200 OK" );
    http_writeline( sck, "Accept-Ranges: bytes" );
    http_writeline( sck, "Content-Length: " + Clib::tostring( fsize ) );
    http_writeline( sck, "Content-Type: " + content_type );
    http_writeline( sck, "" );

    // Actual reading and outputting.
    char bfr[256];
    unsigned int cur_read = 0;
    while ( sck.connected() && ifs.good() && cur_read < fsize )
    {
      ifs.read( bfr, sizeof( bfr ) );
      cur_read += static_cast<unsigned int>( ifs.gcount() );
      sck.send( bfr, static_cast<unsigned int>( ifs.gcount() ) );  // This was sizeof bfr, which
                                                                   // would send garbage... fixed --
                                                                   // Nando, 2009-02-22
    }
    // -------------
  }
  else
  {
    http_not_found( sck, page );
  }
}

void http_func( SOCKET client_socket )
{
  Clib::Socket sck( client_socket );
  Clib::SocketLineReader lineReader( sck, 5, 3000,
                                     false );  // we take care of disconnecting at timeout

  std::string get;
  std::string auth;
  std::string tmpstr;
  std::string host;

  if ( Plib::systemstate.config.web_server_local_only && !sck.is_local() )
  {
    http_forbidden( sck );
    return;
  }

  bool timed_out = false;
  Tools::HighPerfTimer requestTimer;
  while ( sck.connected() && lineReader.read( tmpstr, &timed_out ) )
  {
    if ( Plib::systemstate.config.web_server_debug )
      INFO_PRINTLN( "http({}): '{}'", sck.handle(), tmpstr );
    if ( tmpstr.empty() )
      break;
    if ( strncmp( tmpstr.c_str(), "GET", 3 ) == 0 )
      get = tmpstr;
    if ( strncmp( tmpstr.c_str(), "Authorization:", 14 ) == 0 )
      auth = tmpstr;
    if ( strncmp( tmpstr.c_str(), "Host: ", 5 ) == 0 )
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

  ISTRINGSTREAM is( get );

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
      if ( Plib::systemstate.config.web_server_password != unpw )
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
    std::string type = gamestate.mime_types[pagetype];
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

void test_decode( const char* page, bool result_expected, Plib::Package* pkg_expected,
                  const char* filename_expected, const char* pagetype_expected,
                  const char* redirect_to_expected )
{
  Plib::Package* pkg = nullptr;
  std::string filename;
  std::string pagetype;
  std::string redirect_to;
  bool result;

  result = decode_page( page, &pkg, &filename, &pagetype, &redirect_to );
  passert_always( result == result_expected );
  if ( result )
  {
    assert( redirect_to == redirect_to_expected );
    (void)redirect_to_expected;
    if ( redirect_to.empty() )
    {
      passert_always( pkg == pkg_expected );
      passert_always( filename == filename_expected );
      passert_always( pagetype == pagetype_expected );
    }
  }
}

void test_decode()
{
  /*
      lock();
      if (find_package( "testwww" ))
      {
      test_decode( "/", true, nullptr, "scripts/www/index.htm", "htm" );
      test_decode( "/pkg/testwww1",
      true, find_package( "testwww1" ), "pkg/test/testwww1/www/index.htm", "htm", "" );
      test_decode( "/pkg/testwww1/noexist.ecl",
      true, find_package( "testwww1" ), "www/noexist.ecl", "ecl", "" );
      test_decode( "/pkg/testwww3",
      true, find_package( "testwww3" ), "www/index.ecl", "ecl" );
      }
      unlock();
      */
}


void http_thread()
{
  test_decode();


  config_web_server();
  init_http_thread_support();

  // if (1)
  INFO_PRINTLN( "Listening for HTTP requests on port {}",
                Plib::systemstate.config.web_server_port );

  SOCKET http_socket = Network::open_listen_socket( Plib::systemstate.config.web_server_port );
  if ( http_socket == INVALID_SOCKET )
  {
    ERROR_PRINTLN( "Unable to listen on socket: {}", http_socket );
    return;
  }
  fd_set listen_fd;
  struct timeval listen_timeout = { 0, 0 };

  Pol::threadhelp::TaskThreadPool worker_threads( 2, "http" );  // two threads should be enough
  while ( !Clib::exit_signalled )
  {
    int nfds = 0;
    FD_ZERO( &listen_fd );

    FD_SET( http_socket, &listen_fd );
#ifndef _WIN32
    nfds = http_socket + 1;
#endif

    int res;
    do
    {
      listen_timeout.tv_sec = 5;
      listen_timeout.tv_usec = 0;
      res = select( nfds, &listen_fd, nullptr, nullptr, &listen_timeout );
    } while ( res < 0 && !Clib::exit_signalled && socket_errno == SOCKET_ERRNO( EINTR ) );

    if ( res <= 0 )
    {
      load_mime_config();
      continue;
    }

    if ( FD_ISSET( http_socket, &listen_fd ) )
    {
      if ( Plib::systemstate.config.web_server_debug )
        INFO_PRINTLN( "Accepting connection.." );

      struct sockaddr client_addr;  // inet_addr
      socklen_t addrlen = sizeof client_addr;
      SOCKET client_socket = accept( http_socket, &client_addr, &addrlen );
      if ( client_socket == INVALID_SOCKET )
        return;

      Network::apply_socket_options( client_socket );

      std::string addrstr = Network::AddressToString( &client_addr );
      INFO_PRINTLN( "HTTP client connected from {}", addrstr );

      worker_threads.push(
          [=]() { http_func( client_socket ); } );  // copy socket into queue to keep it valid
    }
  }

  gamestate.mime_types.clear();  // cleanup on exit

#ifdef _WIN32
  closesocket( http_socket );
#else
  close( http_socket );
#endif
}

void start_http_server()
{
  threadhelp::start_thread( http_thread, "HTTP" );
}
}  // namespace Pol::Core
