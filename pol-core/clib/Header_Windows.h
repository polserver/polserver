#ifndef POL_CORE_CLIB_HEADER_WINDOWS_H
#define POL_CORE_CLIB_HEADER_WINDOWS_H

#if defined( WINDOWS )

// activate some additional definitions within Windows includes
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma warning( push )
#pragma warning( disable : 4091 )  // unused typedef

#define _WINSOCKAPI_

#define _IMAGEHLP_

#include <winsock2.h>  // has to be included before Windows.h
#include <windows.h>
#include <ws2tcpip.h>
#include <process.h>
#include <psapi.h>
#include <eh.h>
#include <dbghelp.h>
#include <crtdbg.h>
#include <Shellapi.h>

#pragma warning( pop ) // unused typedef in dbghelp.h

//#define snprintf _snprintf

#define MAXDRIVE _MAX_DRIVE
#define MAXDIR _MAX_DIR
#define MAXFILE _MAX_FNAME
#define MAXPATH _MAX_PATH
#define MAXEXT _MAX_EXT

#define ssize_t SSIZE_T

#pragma warning( push )
#pragma warning( disable : 4005 )  // redefinition
#pragma warning( disable : 4127 )  // conditional expression is constant (needed because of FD_SET)
#pragma warning( disable : 4996 )  // unsafe strerror, deprecation warnings for getcwd, chdir
#pragma warning( disable : 4786 )
#pragma warning( disable : 4091 )

#pragma warning( disable : 4251 )

#endif  // defined(Windows)

#endif /* POL_CORE_CLIB_HEADER_WINDOWS_H */
