/*
History
=======
2005-08-22 Folko:     Add config/www.cfg, reloaded when select times out (all 5 seconds)
2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
2008/12/31 Nando:     A call to Register() and some _endthread / pthread_exit were missing
2009/02/22 Nando:     send_binary() was sending the wrong size on sck.send().
2009/02/25 Nando:     removed a stray "\n" that was being sent at the end of send_binary(),
                      probably causing some corrupted images to be loaded in browsers.

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../bscript/berror.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/esignal.h"
#include "../clib/endian.h"
#include "../clib/fileutil.h"
#include "../clib/logfile.h"
#include "../clib/stlutil.h"
#include "../clib/strexcpt.h"
#include "../clib/strutil.h"
#include "../clib/threadhelp.h"
#include "../clib/wnsckt.h"

#include "../plib/pkg.h"

#include "polcfg.h"
#include "polsem.h"
#include "scrdef.h"
#include "scrsched.h"
#include "scrstore.h"
#include "sockio.h"
#include "uoexec.h"
#include "module/uomod.h"

#include "polwww.h"
#include "module/httpmod.h"

using namespace threadhelp;

static Package* wwwroot_pkg = NULL;
static map<string, string> mime_types;

void load_mime_config(void)
{
	static time_t last_load = 0;

	if(!FileExists("config/www.cfg")) {
		if(last_load) {
			mime_types.clear();
			last_load = 0;
		}
		mime_types["jpg"] = "image/jpeg";
		mime_types["jpeg"] = "image/jpeg";
		mime_types["gif"] = "image/gif";
		return;
	}

	try {
		ConfigFile cf("config/www.cfg");
		if(cf.modified() <= last_load) { // not modified
			return;
		}
		last_load = cf.modified();
		mime_types.clear();
		ConfigElem elem;
		while(cf.read(elem)) {
			string ext, mime;
			elem.remove_prop("Extension", &ext);
			elem.remove_prop("MIME", &mime);
			mime_types[ext] = mime;
		}
	} catch(...) {
		Log2("Error while parsing www.cfg\n");
	}
}

void config_web_server()
{
	for( Packages::iterator itr = packages.begin(); itr != packages.end(); ++itr )
	{
		Package* pkg = (*itr);
		if (pkg->provides_system_home_page())
		{
			if (wwwroot_pkg != NULL)
			{
				Log( "Package %s also provides a wwwroot, ignoring\n", pkg->desc().c_str() );
			}
			else
			{
				Log( "wwwroot package is %s\n", pkg->desc().c_str() );
				wwwroot_pkg = pkg;
			}
		}
	}
	load_mime_config();
}

// TODO: The http server is susceptible to DOS attacks
// TODO: limit access to localhost by default, probably

bool http_readline( Socket& sck, std::string& s )
{
	//cout << "{+RL}";
	bool res = false;
	s = "";
	unsigned char ch;
	while (sck.connected() && sck.recvbyte( &ch, 10000 ))
	{
		if (isprint( ch ))
		{
			s.append(1, ch );
			if (s.length() > 3000)
			{
				sck.close();
				break;// return false;
			}
		}
		else
		{
			// printf( "[%02.02x]", ch );
			if (ch == '\n')
			{
				res = true;
				break;
			   // return true;
			}
		}
	}
	//cout << "{-RL}";
	return res;
}
void http_writeline( Socket& sck, const std::string& s )
{
	sck.send( (void *) s.c_str(), s.length() );
	sck.send( "\n", 1 );
}

void http_forbidden( Socket& sck )
{
	http_writeline( sck, "HTTP/1.1 403 Forbidden" );
	http_writeline( sck, "Content-Type: text/html" );
	http_writeline( sck, "" );
	http_writeline( sck, "<HTML><HEAD><TITLE>403 Forbidden</TITLE></HEAD>" );
	http_writeline( sck, "<BODY><H1>Forbidden</H1>" );
	http_writeline( sck, "You are forbidden to access this server." );
	http_writeline( sck, "</BODY></HTML>" );
}

void http_forbidden( Socket& sck, const string& filename )
{
	http_writeline( sck, "HTTP/1.1 403 Forbidden" );
	http_writeline( sck, "Content-Type: text/html" );
	http_writeline( sck, "" );
	http_writeline( sck, "<HTML><HEAD><TITLE>403 Forbidden</TITLE></HEAD>" );
	http_writeline( sck, "<BODY><H1>Forbidden</H1>" );
	http_writeline( sck, "You are forbidden to access to " + filename + " on this server." );
	http_writeline( sck, "</BODY></HTML>" );
}

void http_not_authorized( Socket& sck, const string& filename )
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

void http_not_found( Socket& sck, const string& filename )
{
	http_writeline( sck, "HTTP/1.1 404 Not Found" );
	http_writeline( sck, "Content-Type: text/html" );
	http_writeline( sck, "" );
	http_writeline( sck, "<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>" );
	http_writeline( sck, "<BODY><H1>Not Found</H1>" );
	http_writeline( sck, "The requested URL " + filename + " was not found on this server." );
	http_writeline( sck, "</BODY></HTML>" );
}

void http_redirect( Socket& sck, const string& new_url )
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
// http_decodestr: turn all those %2F etc into what they represent
// rules:
//	'+'   ->   ' '
//	%HH   ->   (hex value)
//	other ->   itself
string http_decodestr( const string& s )
{
	string decoded;
	const char* t = s.c_str();
	while( t != NULL && *t != '\0')
	{
		if (*t == '+')
		{
			decoded.append( 1, ' ' );
			++t;
		}
		else if (*t != '%')
		{
			decoded.append( 1, *t );
			++t;
		}
		else
		{
			// if first is null terminator, won't look at second
			if (isxdigit(*(t+1)) && isxdigit(*(t+2))) 
			{
				char chH = *(t+1);
				char chL = *(t+2);
				t += 3;
				char ch = 0;
				if (isdigit( chH ))
					ch = (chH - '0') << 4;
				else
					ch = ((static_cast<char>(tolower(chH)) - 'a') + 10) << 4;
				
				if (isdigit(chL))
					ch |= (chL - '0');
				else
					ch |= (tolower(chL) - 'a') + 10;
				
				decoded.append( 1, ch );
			}
			else // garbage - just return what we have so far.
			{
				return decoded;
			}
		}
	}
	return decoded;
}

unsigned char cvt_8to6( char ch )
{
	if (ch >= 'A' && ch <= 'Z')
		return ch - 'A';
	else if (ch >= 'a' && ch <= 'z')
		return ch - 'a' + 26;
	else if (ch >= '0' && ch <= '9')
		return ch - '0' + 52;
	else if (ch == '+')
		return 62;
	else if (ch == '/')
		return 63;
	else if (ch == '=')
		return 0x40; // pad
	else
		return 0x80; // error
}
 
string decode_base64( const std::string& b64s )
{
	string s;
	const char* t = b64s.c_str();
	char c[4];
	char x[3];
	char b[4];
	while (*t)
	{  
		c[0] = *t++;
		c[1] = *t ? *t++ : '\0';
		c[2] = *t ? *t++ : '\0';
		c[3] = *t ? *t++ : '\0';

		b[0] = cvt_8to6( c[0] );
		b[1] = cvt_8to6( c[1] );
		b[2] = cvt_8to6( c[2] );
		b[3] = cvt_8to6( c[3] );

		x[0] = ((b[0] << 2) & 0xFC) | ((b[1] >> 4) & 0x03);
		x[1] = ((b[1] << 4) & 0xF0) | ((b[2] >> 2) & 0x0F);
		x[2] = ((b[2] << 6) & 0xC0) | ( b[3]	   & 0x3F);
		
		if (x[0]) s.append( 1, x[0] );
		if (x[1]) s.append( 1, x[1] );
		if (x[2]) s.append( 1, x[2] );
	}
	return s;
}

bool legal_pagename( const string& page )
{
	// make sure the page isn't going to go visit our hard disk
	for( const char* t = page.c_str(); *t; ++t )
	{
		char ch = *t;
		if (isalnum( ch ) || (ch == '/') || (ch == '_'))
		{
			continue;
		}
		else if ((ch == '.') && (isalnum( *(t+1) )))
		{
			continue;
		}
		else
		{
			return false;
		}
	}
	return true;
}

// get_pagetype: extract the extension, basically
string get_pagetype( const string& page )
{
	string::size_type lastslash = page.rfind( '/' );
	string::size_type dotpos = page.rfind( '.' );
	
	if (lastslash != string::npos && dotpos != string::npos && lastslash > dotpos)
		return "";

	if (dotpos != string::npos)
	{
		return page.substr( dotpos+1 );
	}
	else
	{
		return "";
	}
}

bool get_script_page_filename( const string& page, ScriptDef& sd )
{
	if (page.substr( 0, 5 ) == "/pkg/")
	{
		// cerr << "package page script: " << page << endl;
		string::size_type pkgname_end = page.find_first_of( '/', 5 );
		if (pkgname_end != string::npos)
		{
			string pkg_name = page.substr( 5, pkgname_end-5 );
			// cerr << "pkg name: " << pkg_name << endl;
			Package* pkg = find_package( pkg_name );
			if (pkg != NULL)
			{
				sd.quickconfig( pkg, "www/" + page.substr(pkgname_end+1) );
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		sd.quickconfig( "scripts/www" + page + ".ecl" );
		return true;
	}
}

// FIXME this is just ugly!  The HttpExecutorModule takes ownership of the
// socket, through the Socket copy-constructor..  But sometimes, after we've
// built it, we need to send data (on errors).
bool start_http_script( Socket& sck, 
						const string& page, 
						Package* pkg, const string& file_ecl, 
						const string& query_string )
{
	bool res = true;

	// cout << "ECL file! fire script" << endl;
	ScriptDef page_sd;
	if (pkg)
		page_sd.quickconfig( pkg, file_ecl );
	else
		page_sd.quickconfig( file_ecl );

	if (!page_sd.exists())
	{
		Log( "WebServer: not found: %s\n", page_sd.name().c_str() );
		http_not_found( sck, page );
		return false;
	}

	PolLock2 lck;

	ref_ptr<EScriptProgram> program = find_script2( page_sd, true, config.cache_interactive_scripts );
		  //find_script( filename, true, config.cache_interactive_scripts );
	if (program.get() == NULL)
	{
		cerr << "Error reading script " << page_sd.name() << endl;
		res = false;
		lck.unlock();
		http_not_found( sck, page );
		lck.lock();
	}
	else
	{
		UOExecutor* ex = create_script_executor();
		UOExecutorModule* uoemod = new UOExecutorModule( *ex );
		ex->addModule( uoemod );	
		HttpExecutorModule* hem = new HttpExecutorModule( *ex, sck );

		hem->read_query_string( query_string );
		hem->read_query_ip();

		ex->addModule( hem );

		if (!ex->setProgram( program.get() ))
		{
			lck.unlock();
			http_not_found( hem->sck_, page );
			lck.lock();
			delete ex;
			res = false;
		}
		else
		{
			http_writeline( hem->sck_, "HTTP/1.1 200 OK" );
			http_writeline( hem->sck_, "Content-Type: text/html" );
			http_writeline( hem->sck_, "" );
			ex->setDebugLevel( Executor::NONE );
			//std::cout << "scheduling executor" << std::endl;
			schedule_executor( ex );
		}
	}
	program.clear(); // do this so deletion happens while we're locked
	lck.unlock();
	return res;
}

string get_page_filename( const string& page )
{
	string filename = "scripts/www" + page;
	return filename;
}

bool decode_page( const string& ipage, 
				  Package** ppkg, 
				  string* pfilename,
				  string* ppagetype,
				  string* redirect_to )
{
	string page = ipage;
	Package* pkg = NULL;
	string filedir;
	string retdir;

	if (page.substr( 0, 5 ) == "/pkg/" )
	{
		// cerr << "package page: " << page << endl;
		string::size_type pkgname_end = page.find_first_of( '/', 5 );
		string pkgname;
		if (pkgname_end != string::npos)
		{
			pkgname = page.substr( 5, pkgname_end-5 );
			page = page.substr( pkgname_end );
		}
		else
		{
			pkgname = page.substr( 5 );
			page = "/";
		}

		// cerr << "pkg name: " << pkgname << endl;
		pkg = find_package( pkgname );
		if (pkg == NULL)
			return false;

		filedir = pkg->dir() + "www";
	}
	else
	{
		if (wwwroot_pkg != NULL)
		{
			filedir = wwwroot_pkg->dir() + "www";
			retdir = wwwroot_pkg->dir() + "www";
		}
		else
		{
			filedir = "scripts/www";
			retdir = "scripts/www";
		}
	}

	string filename = filedir + page;
	string fileret;
	
	string pagetype = get_pagetype( page );

	if (pagetype == "") // didn't specify, so assume it's a directory.
	{				   // have to redirect...
		page = ipage;
		if (page.empty() || page[ page.size()-1 ] != '/')
		{
			page += "/";
			//filename += "/";
		}
	
		string test;
		test = filename + "index.ecl";
		if (FileExists( test.c_str() ))
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
	
	if (pagetype == "") // didn't specify, so assume it's a directory.
	{				   // have to redirect...
		if (page.empty() || page[ page.size()-1 ] != '/')
		{
			page += "/";
			filename += "/";
		}
	
		string test;
		test = filename + "index.ecl";
		if (FileExists( test.c_str() ))
		{
			page += "index.ecl";
			filename += "index.ecl";
			pagetype = "ecl";
		}
		else
		{
			page += "index.htm";
			filename += "index.htm";
			pagetype = "htm";
		}
	}

	if (pkg)
	{
		if (pagetype == "ecl")
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

void send_html( Socket& sck, const string& page, const string& filename )
{
	ifstream ifs( filename.c_str() );
	if (ifs.is_open())
	{
		http_writeline( sck, "HTTP/1.1 200 OK" );
		http_writeline( sck, "Content-Type: text/html" );
		http_writeline( sck, "" );
		string t;
		while (getline(ifs,t))
		{
			http_writeline( sck, t );
		}
	}
	else
	{
		http_not_found( sck, page );
	}
}	

void send_binary( Socket& sck, const string& page, const string& filename, const string& content_type )
{
	//string filename = get_page_filename( page );
	unsigned int fsize = filesize(filename.c_str());
	ifstream ifs( filename.c_str(), ios::binary );
	if ( ifs.is_open() )
	{
		http_writeline( sck, "HTTP/1.1 200 OK" );
		http_writeline( sck, "Accept-Ranges: bytes" );
		http_writeline( sck, "Content-Length: " + decint( fsize ) );
		http_writeline( sck, "Content-Type: " + content_type );
		http_writeline( sck, "" );

		// Actual reading and outputting.
		char bfr[ 256 ];
		unsigned int cur_read = 0;
		while ( sck.connected() && ifs.good() && cur_read < fsize)
		{		
			ifs.read(bfr, sizeof(bfr));
			cur_read += static_cast<unsigned int>(ifs.gcount());
			sck.send(bfr, static_cast<unsigned int>(ifs.gcount()));  // This was sizeof bfr, which would send garbage... fixed -- Nando, 2009-02-22
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
	Socket sck( client_socket );
	string get;
	string auth;
	string tmpstr;
	string host;

	if (config.web_server_local_only)
	{
		if (!sck.is_local())
		{
			http_forbidden( sck );
			return;
		}
	}

	while (sck.connected() && http_readline( sck, tmpstr ))
	{
		if (config.web_server_debug)
			cout << "http(" << sck.handle() << "): '" << tmpstr << "'" << endl;
		if (tmpstr.empty()) break;
		if (strncmp( tmpstr.c_str(), "GET", 3 ) == 0)
			get = tmpstr;
		if (strncmp( tmpstr.c_str(), "Authorization:", 14 ) == 0)
			auth = tmpstr;
		if (strncmp( tmpstr.c_str(), "Host: ", 5 ) == 0)
			host = tmpstr.substr( 6 );
	}
	if (!sck.connected())
		return;
	

	// cout << "http-get: '" << get << "'" << endl;
	
	ISTRINGSTREAM is( get );
	
	string cmd;	 // GET, POST  (we only handle GET)
	string url;	 // The whole URL (xx.ecl?a=b&c=d)
	string proto;   // HTTP/1.1
	string page;	// xx.ecl
	string query_string;  // a=b&c=d

	is >> cmd >> url >> proto;
	
	if (config.web_server_debug)
	{
		cout << "http-cmd:   '" << cmd << "'" << endl;
		cout << "http-host:  '" << host << "'" << endl;
		cout << "http-url:   '" << url << "'" << endl;
		cout << "http-proto: '" << proto << "'" << endl;
	}
	
//	if (url == "/")
//		url = "/index.htm";
	
	// spliturl( url, page, params ); ??
	string::size_type ques = url.find( '?' );
	
	if (ques == string::npos)
	{
		page = url;
		query_string = "";
	}
	else
	{
		page = url.substr( 0, ques );
		query_string = url.substr( ques+1 );
	}

	if (config.web_server_debug)
	{
		cout << "http-page:   '" << page << "'" << endl;
		cout << "http-params: '" << query_string << "'" << endl;
		cout << "http-decode: '" << http_decodestr( query_string ) << "'" << endl;
	}

	if ( !config.web_server_password.empty() )
	{
		if ( !auth.empty() )
		{
			ISTRINGSTREAM is2( auth );
			string auth, type, coded_unpw, unpw;
			is2 >> auth >> type >> coded_unpw;
			unpw = decode_base64( coded_unpw );
			if (config.web_server_debug)
			{
				cout << "http-pw: '" << coded_unpw << "'" << endl;
				cout << "http-pw-decoded: '" << unpw << "'" << endl;
			}
			if (config.web_server_password != unpw)
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

	if ( !legal_pagename(page) )
	{
		// FIXME should probably be access denied
		http_forbidden( sck, page );
		return;
	}


	Package* pkg;
	string filename;
	string pagetype;
	string redirect_to;
	if ( !decode_page(page, &pkg, &filename, &pagetype, &redirect_to) )
	{
		http_not_found( sck, page );
		return;
	}
	if (!redirect_to.empty())
	{
		http_redirect( sck, /*"http://" + host +*/ redirect_to );
		return;
	}
	
	if (config.web_server_debug)
		cout << "Page type: " << pagetype << endl;
	
	if (pagetype == "ecl")
	{
		// Note it takes ownership of the socket
		start_http_script( sck, page, pkg, filename, query_string );
	}
	else if (pagetype == "htm" || pagetype == "html")
	{
		send_html( sck, page, filename );
	}
	else
	{
		string type = mime_types[pagetype];
		if(type.length() > 0) {
			send_binary(sck, page, filename, type);
		} else {
			// TODO: bitch to log file
			cout << "HTTP server: I can't handle pagetype '" << pagetype << "'" << endl;
		}
	}
}


void http_conn_thread_stub2( void* arg )
{
	SOCKET client_socket = reinterpret_cast<SOCKET>(arg);

	http_func( client_socket );
}

#ifdef _WIN32

#include <process.h>
void init_http_thread_support()
{
}
unsigned __stdcall http_conn_thread_stub( void *arg )
{
	InstallStructuredExceptionHandler();

	int sck = (int)(reinterpret_cast<SOCKET>(arg));
	threadmap.Register( threadhelp::thread_pid(), string("http_conn_thread:")+decint(sck) ); // Was missing (Nando -- 12-31-2008)
	
	threadhelp::run_thread( http_conn_thread_stub2, arg );

	_endthreadex(0); // missing (Nando -- 12-31)
	return 0;
}

void start_http_conn_thread( SOCKET client_socket )
{
	// Code Analyze: C6001
	//	unsigned threadid;
	unsigned threadid = 0;
	HANDLE h;

	h = (HANDLE) _beginthreadex( NULL, 
								 0, 
								 http_conn_thread_stub, 
								 reinterpret_cast<void*>(client_socket), 
								 0, 
								 &threadid );
	if (h == 0) {
		Log( "error in start_http_conn_thread: %d %d \"%s\" \"%s\" %d %d\n",
				 errno, _doserrno, strerror( errno ), strerror( _doserrno ), http_conn_thread_stub, client_socket);
		cerr << "start_http_conn_thread error: " << strerror(errno) << endl;

		dec_child_thread_count();
	}
	else
		CloseHandle( h );
}
#else

#include <pthread.h>
pthread_attr_t http_attr;
void init_http_thread_support()
{
	pthread_attr_init( &http_attr );
	pthread_attr_setdetachstate( &http_attr, PTHREAD_CREATE_DETACHED );
}

void* http_conn_thread_stub( void *arg )
{
	int sck = (int)(reinterpret_cast<SOCKET>(arg));
	threadmap.Register( threadhelp::thread_pid(), string("http_conn_thread:")+decint(sck) );

	run_thread( http_conn_thread_stub2, arg );

	pthread_exit(NULL); // Missing (Nando -- 12-31-2008)
	return NULL;
}
void start_http_conn_thread( SOCKET client_socket )
{
	pthread_t th;
	int res = pthread_create( &th, 
							  &http_attr, 
							  http_conn_thread_stub, 
							  reinterpret_cast<void*>(client_socket) );
	if (res) // Turley 06-26-2009: removed passert_always so pol doesnt crash
	{
		Log( "Failed to create worker thread for http (res = %d)\n", res );
		dec_child_thread_count();
	}
}

#endif

void test_decode( const char* page,
				  bool result_expected,
				  Package* pkg_expected,
				  const char* filename_expected,
				  const char* pagetype_expected,
				  const char* redirect_to_expected )
{
	Package* pkg;
	string filename;
	string pagetype;
	string redirect_to;
	bool result;

	result = decode_page( page, &pkg, &filename, &pagetype, &redirect_to );
	passert_always( result == result_expected );
	if (result)
	{
		assert( redirect_to == redirect_to_expected );
		if (redirect_to.empty())
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
		test_decode( "/", true, NULL, "scripts/www/index.htm", "htm" );
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


void http_thread(void)
{
	test_decode();


	config_web_server();
	init_http_thread_support();

	//if (1)
	{
		PolLock lck;
		cout << "Listening for HTTP requests on port " << config.web_server_port << endl;
	}
	SOCKET http_socket = open_listen_socket( config.web_server_port );
	if (http_socket < 0)
	{
		PolLock lck;
		cerr << "Unable to listen on socket: " << http_socket << endl;
		return;
	}
	fd_set listen_fd;
	struct timeval listen_timeout = { 0, 0 };

	while (!exit_signalled)
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
			// printf( "HTTP Listen!\n" );
			res = select( nfds, &listen_fd, NULL, NULL, &listen_timeout );
		} while (res < 0 && !exit_signalled && socket_errno == SOCKET_ERRNO(EINTR));
		
		if (res <= 0) {
			load_mime_config();
			continue;
		}

		if ( FD_ISSET( http_socket, &listen_fd ))
		{
			if (config.web_server_debug)
				cout << "Accepting connection.." << endl;

			struct sockaddr client_addr; // inet_addr
			socklen_t addrlen = sizeof client_addr;
			SOCKET client_socket = accept( http_socket, &client_addr, &addrlen );
			if (client_socket < 0)
				return;

			apply_socket_options( client_socket );

			string addrstr = AddressToString( &client_addr );
			cout << "HTTP client connected from " << addrstr << endl;

			threadhelp::inc_child_thread_count();
			start_http_conn_thread( client_socket );
		}
	}
	mime_types.clear(); // cleanup on exit
}

void start_http_server()
{
	threadhelp::start_thread( http_thread, "HTTP" );
}
