/*
History
=======
2009/07/20 MuadDib: Removed calls and use of StackWalker for leak detection. vld is now used, much better.
                    Removed StackWalker.cpp/.h from the vcproj files also.

Notes
=======

*/

#include "stl_inc.h"

#include "clib.h"
#include "strexcpt.h"

#include "xmain.h"

#ifdef _WIN32
  #include <windows.h> // for GetModuleFileName
  #include <crtdbg.h>
#endif

unsigned long refptr_count;

static void parse_args( int argc, char *argv[] );

int main( int argc, char *argv[] )
{
    int exitcode = 0;

    try 
    {
        InstallStructuredExceptionHandler();
	// FIXME: 2008 Upgrades needed here? Make sure this still valid on 2008
#if defined(_WIN32) && defined(_DEBUG) && _MSC_VER >= 1300
        // on VS.NET, disable obnoxious heap checking
        int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        flags &= 0x0000FFFF; // set heap check frequency to 0
        _CrtSetDbgFlag( flags );
#endif
        parse_args( argc, argv );
        exitcode = xmain( argc, argv );
    }
    catch( const char *msg )
    {
        cerr << "Execution aborted due to: " << msg << endl;
        exitcode = 1;
    }
    catch( string& str )
    {
        cerr << "Execution aborted due to: " << str << endl;
        exitcode = 1;
    }       // egcs has some trouble realizing 'exception' should catch
    catch( runtime_error& re )   // runtime_errors, so...
    {
        cerr << "Execution aborted due to: " << re.what() << endl;
        exitcode = 1;
    }
    catch( exception& ex )
    {
        cerr << "Execution aborted due to: " << ex.what() << endl;
        exitcode = 1;
    }
    catch( int xn )
    {
        // Something that throws an integer is responsible for printing
        // its own error message.  
        // "throw 3" is meant as an alternative to exit(3).
        exitcode = xn;
    }
#ifndef _WIN32
    catch( ... )
    {
        cerr << "Execution aborted due to generic exception." << endl;
        exitcode = 2;
    }
#endif    
    return exitcode;
}

string xmain_exepath;
string xmain_exedir;

string fix_slashes( string pathname )
{
    string::size_type bslashpos;
    while (string::npos != (bslashpos = pathname.find( '\\')))
    {
        pathname.replace( bslashpos, 1, 1, '/' );
    }
    return pathname;
}

static void parse_args( int argc, char *argv[] )
{
	string exe_path;

#ifdef _WIN32
	// useless windows shell (cmd) usually doesn't tell us the whole path.  
	char module_path[ _MAX_PATH ];
	if (GetModuleFileName( NULL, module_path, sizeof module_path ))
		exe_path = module_path;
	else
		exe_path = argv[0];
#else
	// linux shells are generally more informative.
	exe_path = argv[0];
#endif

    xmain_exepath = fix_slashes( exe_path );
    xmain_exedir = fix_slashes( exe_path );

    string::size_type pos = xmain_exedir.find_last_of( "/" );
    if (pos != string::npos)
    {
        xmain_exedir.erase( pos );
        xmain_exedir += "/";
    }
}

