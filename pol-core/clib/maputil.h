/** @file
 *
 * @par History
 */

#pragma once

#include <algorithm>
#include <concepts>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>

#include "clib/clib.h"
#include "clib/rawtypes.h"

namespace Pol::Clib
{
template <typename T>
concept FlyWeightString = requires( T x ) {
  { x.get() } -> std::same_as<const std::string&>;
};
template <typename T>
concept CharArray = std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>;

template <typename T>
concept CmpString = std::same_as<T, std::string> || std::same_as<T, std::string_view> ||
                    FlyWeightString<T> || std::same_as<T, const char*> || CharArray<T>;

/// The string_view behind any of the shapes CmpString accepts.
struct ci_converter
{
  static auto get( const std::string& x ) -> std::string_view { return { x }; };
  static auto get( std::string_view x ) -> std::string_view { return x; };
  static auto get( FlyWeightString auto const& x ) -> std::string_view { return { x.get() }; };
  static auto get( const char* x ) -> std::string_view { return { x }; }
};

struct ci_cmp_pred
{
  // allow different types tag
  using is_transparent = void;

  bool operator()( CmpString auto const& x1, CmpString auto const& x2 ) const
  {
    auto sv1 = ci_converter::get( x1 );
    auto sv2 = ci_converter::get( x2 );
    // benchmark showed that strnicmp is faster then lexicographical_compare, even if we have to
    // create string_views. my guess is that it gets completly optimized away and what is left is
    // one comparison vs two comparisons per iteration
    auto res = strnicmp( sv1.data(), sv2.data(), std::min( sv1.size(), sv2.size() ) );
    if ( res != 0 )
      return res < 0;
    if ( sv1.size() == sv2.size() )
      return res < 0;
    if ( sv1.size() > sv2.size() )
      return false;
    return true;
  }
};

/// Case-insensitive FNV-1a over ASCII, not std::hash: that one hashes raw bytes, and these names
/// are matched regardless of case. constexpr, which std::hash also is not, so a literal name at
/// the call site is hashed at compile time and a lookup only compares numbers. Never returns 0,
/// which leaves that value free for a sentinel -- ConfigProps marks a removed slot with it.
constexpr u32 ci_hash( std::string_view s )
{
  u32 h = 2166136261u;
  for ( char c : s )
  {
    const char lowered = ( c >= 'A' && c <= 'Z' ) ? static_cast<char>( c + ( 'a' - 'A' ) ) : c;
    h ^= static_cast<u32>( static_cast<unsigned char>( lowered ) );
    h *= 16777619u;
  }
  return h != 0 ? h : 1u;
}

inline bool ci_equal( std::string_view a, std::string_view b )
{
  return a.size() == b.size() && strnicmp( a.data(), b.data(), a.size() ) == 0;
}

/// The two halves an unordered container needs, where an ordered one takes ci_cmp_pred alone.
struct ci_hash_pred
{
  // allow different types tag
  using is_transparent = void;

  size_t operator()( CmpString auto const& x ) const { return ci_hash( ci_converter::get( x ) ); }
};

struct ci_eq_pred
{
  // allow different types tag
  using is_transparent = void;

  bool operator()( CmpString auto const& x1, CmpString auto const& x2 ) const
  {
    return ci_equal( ci_converter::get( x1 ), ci_converter::get( x2 ) );
  }
};
}  // namespace Pol::Clib
