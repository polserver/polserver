/** @file
 *
 * @par History
 *
 * @note ATTENTION
 * This header is part of the PCH
 * Remove the include in all StdAfx.h files or live with the consequences :)
 */


#pragma once

#include <cstring>
#include <deque>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Pol::Clib
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

inline int stringicmp( const std::string& str1, const std::string& str2 )
{
#ifdef _WIN32
  return _stricmp( str1.c_str(), str2.c_str() );
#else
  return strcasecmp( str1.c_str(), str2.c_str() );
#endif
}
inline int stringicmp( const std::string& str1, const char* str2 )
{
#ifdef _WIN32
  return _stricmp( str1.c_str(), str2 );
#else
  return strcasecmp( str1.c_str(), str2 );
#endif
}
inline int stringicmp( const char* str1, const std::string& str2 )
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

template <class T, size_t N>
constexpr size_t arsize( T ( & )[N] )
{
  return N;
}

// Memory size estimation of std containers
// highly implementation specific, so just a rough guess
template <typename T>
size_t memsize( const std::vector<T>& container )
{
  return 3 * sizeof( void* ) + container.capacity() * sizeof( T );
}
template <typename T>
size_t memsize( const std::set<T>& container )
{
  return 3 * sizeof( void* ) + container.size() * sizeof( T ) + 3 * sizeof( void* );
}
namespace
{
template <typename M>
size_t _mapimp( const M& container )
{
  using K = typename M::key_type;
  using V = typename M::mapped_type;
  if constexpr ( std::is_same_v<K, std::string> && std::is_same_v<V, std::string> )
  {
    size_t size = ( ( sizeof( void* ) * 3 + 1 ) / 2 ) * container.size();
    for ( const auto& p : container )
      size += p.first.capacity() + +p.second.capacity();
    return size;
  }
  if constexpr ( std::is_same_v<K, std::string> )
  {
    size_t size = ( sizeof( V ) + ( sizeof( void* ) * 3 + 1 ) / 2 ) * container.size();
    for ( const auto& p : container )
      size += p.first.capacity();
    return size;
  }
  return ( sizeof( K ) + sizeof( V ) + ( sizeof( void* ) * 3 + 1 ) / 2 ) * container.size();
}
}  // namespace
template <typename K, typename V, typename C>
size_t memsize( const std::map<K, V, C>& container )
{
  return _mapimp( container );
}
template <typename T>
size_t memsize( const std::unordered_set<T>& container )
{
  return 3 * sizeof( void* ) + container.size() * sizeof( T ) + 3 * sizeof( void* );
}
template <typename K, typename V, typename C>
size_t memsize( const std::unordered_map<K, V, C>& container )
{
  return _mapimp( container );
}
template <typename K, typename V, typename C>
size_t memsize( const std::multimap<K, V, C>& container )
{
  return _mapimp( container );
}
template <typename T>
size_t memsize( const std::deque<T>& container )
{
  // roughly like a list
  return 3 * sizeof( void* ) + container.size() * ( sizeof( T ) + 2 * sizeof( void* ) );
}
template <typename T, typename C>
size_t memsize( const std::queue<T, C>& container )
{
  // assuming we use the default imp of container a deque
  return 3 * sizeof( void* ) + container.size() * ( sizeof( T ) + 2 * sizeof( void* ) );
}
template <typename T, typename K, typename C>
size_t memsize( const std::priority_queue<T, K, C>& container )
{
  return 3 * sizeof( void* ) + container.size() * sizeof( T );
}
template <typename T>
size_t memsize( const std::list<T>& container )
{
  return 3 * sizeof( void* ) + container.size() * ( sizeof( T ) + 2 * sizeof( void* ) );
}
}  // namespace Pol::Clib

