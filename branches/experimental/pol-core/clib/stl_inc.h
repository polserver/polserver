/*
History
=======
2008/03/01 Shinigami: VS2005 will not use STLport, but VS2003 will use it
2009/09/01 MuadDib:   VS2005/2008 Support added with STLPort 5.1.2

Notes
=======

*/

#ifndef CLIB_STL_INC_H
#define CLIB_STL_INC_H

#ifdef _WIN32
//Set to _USE_STLPORT_WIN32 to 0 to use MSCVs STL.
#	if defined(_MSC_VER) && (_MSC_VER <= 1500) // up to VS2003
#		define _USE_STLPORT_WIN32 1
#       if (_MSC_VER >= 1400) // 2005 and higher
#			include <cctype>
#			include <cstring>
#           include <iosfwd>
#			include <locale>
#       endif
#	else // Above 2008
#		define _USE_STLPORT_WIN32 0
#	endif
#	pragma warning( disable: 4786 )//'identifier' : identifier was truncated to 'number' characters in the debug information
#	pragma warning( push, 4 )
#endif

// Minimum Windows OS: Win98/NT4
// Minimum IE: 4.0
#ifdef _WIN32
#	define _WIN32_WINDOWS      0x0410
#	define WINVER              0x0400
#	define _WIN32_IE           0x0400
#endif

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <utility> // pair
#include <vector>

#ifdef __GNUC__
  #ifndef USE_STLPORT
    #if __GNUC__ < 3
      #include <hash_map.h>
      #include <hash_set.h>

      namespace SGI { using ::hash_map; using ::hash_set; }; // inherit globals
    #else
       #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3 )
         #include <tr1/unordered_map>
         #include <tr1/unordered_set>
       #else     
         #include <ext/hash_map>
         #include <ext/hash_set>
         #if __GNUC_MINOR__ == 0
           namespace SGI = std;               // GCC 3.0
         #else
           namespace SGI = ::__gnu_cxx;       // GCC 3.1 and later
         #endif
       #endif
    #endif
  #else
    #include <hash_map>
    #include <hash_set>
    namespace SGI = std;
  #endif
#else      // ...  there are other compilers, right?
  namespace SGI = std;
  #include <hash_map>
  #include <hash_set>
#endif

#if defined (_WIN32) && (_USE_STLPORT_WIN32 == 1)
	using stlport::auto_ptr;
	using stlport::basic_string;
	using stlport::bind2nd;
	using stlport::binder2nd;
	using stlport::char_traits;
	using stlport::dec;
	using stlport::equal_to;
	using stlport::filebuf;
	using stlport::find;
	using stlport::for_each;
	using stlport::fstream;
	using stlport::getline;
	using stlport::hex;
	using stlport::hash_map;
	using stlport::hash_set;
	using stlport::ios;
	using stlport::ifstream;
	using stlport::istream;
	using stlport::istringstream;
	using stlport::less;
	using stlport::logic_error;
	using stlport::list;
	using stlport::make_pair;
	using stlport::map;
	using stlport::multimap;
	using stlport::not_equal_to;
	using stlport::ofstream;
	using stlport::ostream;
	using stlport::ostringstream;
	using stlport::pair;
	using stlport::priority_queue;
	using stlport::queue;
	using stlport::remove_if;
	using stlport::reverse;
	using stlport::runtime_error;
	using stlport::set;
	using stlport::setw;
	using stlport::sort;
	using stlport::stack;
	using stlport::streambuf;
	using stlport::streamsize;
	using stlport::string;
	using stlport::swap;
	using stlport::vector;

	using stlport::cerr;
	using stlport::cout;
	using stlport::endl;

#	if (_MSC_VER >= 1400) // 2005 and up
		using stlport::ctype;
		using stlport::exception;
		using stlport::locale;
#	endif

#else
	// GCC
	// we will try libstdc++ on linux
	using std::auto_ptr;
	using std::basic_string;
	using std::bind2nd;
	using std::binder2nd;
	using std::char_traits;
	using std::dec;
	using std::equal_to;
	using std::exception;
	using std::filebuf;
	using std::find;
	using std::for_each;
	using std::fstream;
	using std::getline;
	using std::hex;
	using std::ios;
	using std::ifstream;
	using std::istream;
	using std::istringstream;
	using std::less;
	using std::list;
	using std::logic_error;
	using std::make_pair;
	using std::map;
	using std::multimap;
	using std::not_equal_to;
	using std::ofstream;
	using std::ostream;
	using std::ostringstream;
	using std::pair;
	using std::priority_queue;
	using std::queue;
	using std::remove_if;
	using std::reverse;
	using std::runtime_error;
	using std::set;
	using std::setw;
	using std::sort;
	using std::stack;
	using std::streambuf;
	using std::streamsize;
	using std::string;
	using std::swap;
	using std::vector;

	using std::cerr;
	using std::cout;
	using std::endl;

#	ifdef _WIN32
#		if defined(_MSC_VER) && (_MSC_VER <= 1500) // up to VS2008
			using std::hash_map;
			using std::hash_set;
#		else // VS2005 ff.
			using stdext::hash_map;
			using stdext::hash_set;
#		endif
#	else
#     if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3 )
		using std::tr1::unordered_map;
		using std::tr1::unordered_set;
#     else
		using SGI::hash_map;
		using SGI::hash_set;
#     endif
#	endif

#endif

#ifdef _WIN32
#	pragma warning( pop )
#	pragma warning( disable: 4996 ) //'function': was declared deprecated
#	pragma warning( disable: 4100 ) //unreferenced formal parameter
#   pragma warning( disable: 4505 ) //unreferenced local function has been removed, vs seems to have problems with region virtuals

#	if 0
		// this is useful for finding memory leaks
#		include <crtdbg.h>
#		include "../bscript/bobject.h"
#		ifdef _DEBUG
#			define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
#		else
#			define DEBUG_CLIENTBLOCK
#		endif // _DEBUG

#		ifdef _DEBUG
#			define new DEBUG_CLIENTBLOCK
#		endif

		//#define _CRTDBG_MAP_ALLOC 1
#	endif


#endif

#endif
