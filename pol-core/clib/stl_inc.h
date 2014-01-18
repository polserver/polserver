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
#include <locale.h>
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
#include <mutex>

#include <unordered_map>
#include <unordered_set>

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

  using std::endl;

  using std::unordered_map;
  using std::unordered_set;


#ifdef _WIN32
#	pragma warning( pop )
#	pragma warning( disable: 4996 ) //'function': was declared deprecated
#	pragma warning( disable: 4100 ) //unreferenced formal parameter
#   pragma warning( disable: 4505 ) //unreferenced local function has been removed, vs seems to have problems with region virtuals
#   pragma warning( disable: 4351 ) //new behavior: elements of array 'array' will be default initialized 

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
