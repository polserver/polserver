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

template <class T, size_t N>
constexpr size_t arsize( T ( & )[N] )
{
  return N;
}

// Memory size estimation of std containers
// highly implementation specific, so just a rough guess
namespace
{
// std::map / std::multimap -- rb-tree based
template <typename M>
size_t _mapimp( const M& container )
{
  using K = typename M::key_type;
  using V = typename M::mapped_type;
  using Pair = std::pair<const K, V>;
  constexpr size_t node_size = 4 * sizeof( void* ) + sizeof( Pair );  // 3x void* + color
#ifdef _WIN32
  constexpr size_t overhead = sizeof( container ) + node_size;  // 16+sentinal
#else
  constexpr size_t overhead = sizeof( container );  // 48
#endif
  size_t size = overhead + ( node_size * container.size() );

  if constexpr ( std::is_same_v<K, std::string> && std::is_same_v<V, std::string> )
  {
    for ( const auto& p : container )
      size += p.first.capacity() + p.second.capacity();
  }
  else if constexpr ( std::is_same_v<K, std::string> )
  {
    for ( const auto& p : container )
      size += p.first.capacity();
  }
  else if constexpr ( std::is_same_v<V, std::string> )
  {
    for ( const auto& p : container )
      size += p.second.capacity();
  }
  return size;
}

template <typename M, typename Func>
size_t _mapimp( const M& container, Func f )
{
  using K = typename M::key_type;
  using Pair = std::pair<const K, typename M::mapped_type>;
  constexpr size_t node_size = 4 * sizeof( void* ) + sizeof( Pair );  // 3 void* + color
#ifdef _WIN32
  constexpr size_t overhead = sizeof( container ) + node_size;  // 16+sentinal
#else
  constexpr size_t overhead = sizeof( container );  // 48
#endif

  size_t size = overhead + ( node_size * container.size() );

  if constexpr ( std::is_same_v<K, std::string> )
  {
    for ( const auto& p : container )
      size += p.first.capacity() + f( p.second );
  }
  else
  {
    for ( const auto& p : container )
      size += f( p.second );
  }
  return size;
}

// std::unordered_map -- hash table, singly-linked nodes + bucket array
template <typename M>
size_t _unordered_mapimp( const M& container )
{
  using K = typename M::key_type;
  using V = typename M::mapped_type;
  using Pair = std::pair<const K, V>;

#ifdef _WIN32
  // MSVC: doubly-linked list internally
  constexpr size_t node_size = 2 * sizeof( void* ) + sizeof( Pair );
  constexpr size_t sentinal = node_size;
#else
  // libstdc++: singly-linked list
  constexpr size_t node_size = sizeof( void* ) + sizeof( Pair );  // void*
  constexpr size_t sentinal = 0;
#endif
  // bucket array overhead
  // sizeof(container) = libstdc++ 56 MSVC 64
  size_t size = sizeof( container ) + sentinal + ( container.bucket_count() * sizeof( void* ) );
  size += container.size() * node_size;

  if constexpr ( std::is_same_v<K, std::string> && std::is_same_v<V, std::string> )
  {
    for ( const auto& p : container )
      size += p.first.capacity() + p.second.capacity();
  }
  else if constexpr ( std::is_same_v<K, std::string> )
  {
    for ( const auto& p : container )
      size += p.first.capacity();
  }
  else if constexpr ( std::is_same_v<V, std::string> )
  {
    for ( const auto& p : container )
      size += p.second.capacity();
  }
  return size;
}

template <typename S>
size_t _unordered_setimp( const S& container )
{
  using T = typename S::value_type;
#ifdef _WIN32
  // MSVC: doubly-linked list internally
  constexpr size_t node_size = 2 * sizeof( void* ) + sizeof( T );
  constexpr size_t sentinal = node_size;
#else
  // libstdc++: singly-linked list
  constexpr size_t node_size = sizeof( void* ) + sizeof( T );  // void*
  constexpr size_t sentinal = 0;
#endif
  // sizeof(container) = libstdc++ 56 MSVC 64
  size_t size = sizeof( container ) + sentinal + container.bucket_count() * sizeof( void* );
  size += container.size() * node_size;

  if constexpr ( std::is_same_v<T, std::string> )
  {
    for ( const auto& t : container )
      size += t.capacity();
  }
  return size;
}

template <typename D>
size_t _dequeimp( const D& container )
{
  using T = typename D::value_type;
#ifdef _WIN32
  constexpr size_t block_bytes = sizeof( T ) <= 8 ? 16 : sizeof( T );
#else
  constexpr size_t block_bytes = sizeof( T ) <= 512 ? 512 : sizeof( T );
#endif
  constexpr size_t bpe = block_bytes / sizeof( T );
  const size_t blocks = ( container.size() + bpe - 1 ) / bpe;
  const size_t map_cost = std::max( size_t{ 8 }, blocks ) * sizeof( void* );
  // sizeof(container) = libstdc++ 80 MSVC 40
  return sizeof( container ) + blocks * block_bytes + map_cost;
}
}  // namespace

template <typename T>
size_t memsize( const std::vector<T>& container )
{
  if constexpr ( std::is_same_v<T, std::string> )
  {
    size_t size = 3 * sizeof( void* );
    for ( const auto& t : container )
      size += t.capacity();
    size += ( container.capacity() - container.size() ) * sizeof( T );
    return size;
  }
  else
    return 3 * sizeof( void* ) + container.capacity() * sizeof( T );
}
template <typename T, class Func>
size_t memsize( const std::vector<T>& container, Func f )
{
  size_t size = 3 * sizeof( void* );
  for ( const auto& t : container )
    size += f( t );
  size += ( container.capacity() - container.size() ) * sizeof( T );
  return size;
}

template <typename T>
size_t memsize( const std::set<T>& container )
{
  constexpr size_t node_size = 4 * sizeof( void* ) + sizeof( T );  // 3x void* + color
#ifdef _WIN32
  constexpr size_t overhead = sizeof( container ) + node_size;  // 16 + sentinal
#else
  constexpr size_t overhead = sizeof( container );  // 48
#endif

  size_t size = overhead + container.size() * node_size;
  if constexpr ( std::is_same_v<T, std::string> )
  {
    for ( const auto& t : container )
      size += t.capacity();
  }
  return size;
}

template <typename K, typename V, typename C>
size_t memsize( const std::map<K, V, C>& container )
{
  return _mapimp( container );
}

template <typename K, typename V, typename C>
size_t memsize( const std::map<const K, V, C>& container )
{
  return _mapimp( container );
}

template <typename K, typename V, typename C, typename Func>
size_t memsize( const std::map<K, V, C>& container, Func f )
{
  return _mapimp( container, f );
}

template <typename K, typename V, typename C, typename Func>
size_t memsize( const std::map<const K, V, C>& container, Func f )
{
  return _mapimp( container, f );
}

template <typename K, typename V, typename C, class Func>
size_t memsize_keyvalue( const std::map<K, V, C>& container, Func f )
{
  constexpr size_t map_node_overhead = 4 * sizeof( void* );  // 3 void* + color
  size_t size = map_node_overhead * container.size();
  for ( const auto& p : container )
    size += f( p.first ) + f( p.second );
  return size;
}

template <typename T>
size_t memsize( const std::unordered_set<T>& container )
{
  return _unordered_setimp( container );
}

template <typename K, typename V, typename C>
size_t memsize( const std::unordered_map<K, V, C>& container )
{
  return _unordered_mapimp( container );
}

template <typename K, typename V, typename C>
size_t memsize( const std::multimap<K, V, C>& container )
{
  return _mapimp( container );
}

template <typename T>
size_t memsize( const std::deque<T>& container )
{
  return _dequeimp( container );
}

template <typename T, typename C>
size_t memsize( const std::queue<T, C>& container )
{
  // assuming we use the default imp of container a deque
  return _dequeimp( container );
}

template <typename T, typename K, typename C>
size_t memsize( const std::priority_queue<T, K, C>& container )
{
  // same as vector
  return 3 * sizeof( void* ) + container.size() * sizeof( T );
}

template <typename T>
size_t memsize( const std::list<T>& container )
{
  return 3 * sizeof( void* ) + container.size() * ( sizeof( T ) + 2 * sizeof( void* ) );
}
}  // namespace Pol::Clib
