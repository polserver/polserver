/** @file
 *
 * @par History
 *
 * @note ATTENTION
 * This header is part of the PCH
 * Remove the include in all StdAfx.h files or live with the consequences :)
 */


#ifndef STLUTIL_H
#define STLUTIL_H

#include "Header_Windows.h"
#include "compilerspecifics.h"
#include <cstring>
#include <sstream>

namespace Pol
{
namespace Clib
{
template <class T>
void delete_all( T& coll )
{
  while ( !coll.empty() )
  {
    delete coll.back();
    coll.pop_back();
  }
}

template <class D, class S>
inline D implicit_cast( const S& s )
{
  return s;  // fails unless S can be converted to D implicitly
}

template <class D, class S>
inline D explicit_cast( const S& s )
{
  return static_cast<D>( s );
}

template <class S1, class S2>
inline int stringicmp( const S1& str1, const S2& str2 )
{
#ifdef _WIN32
  return _stricmp( str1.c_str(), str2.c_str() );
#else
  return strcasecmp( str1.c_str(), str2.c_str() );
#endif
}
template <class S1, const char*>
inline int stringicmp( const S1& str1, const char* str2 )
{
#ifdef _WIN32
  return _stricmp( str1.c_str(), str2 );
#else
  return strcasecmp( str1.c_str(), str2 );
#endif
}
template <const char*, class S2>
inline int stringicmp( const char* str1, const S2& str2 )
{
#ifdef _WIN32
  return _stricmp( str1, str2.c_str() );
#else
  return strcasecmp( str1, str2.c_str() );
#endif
}

#define ISTRINGSTREAM std::istringstream

#define OSTRINGSTREAM std::ostringstream
#define OSTRINGSTREAM_STR( x ) x.str()

template <class T>
std::string tostring( const T& v )
{
  OSTRINGSTREAM os;
  os << v;
  return OSTRINGSTREAM_STR( os );
}

template <class T, size_t N>
CONSTEXPR size_t arsize( T ( & )[N] )
{
  return N;
}
}
}

#endif
