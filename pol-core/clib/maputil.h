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

#include "clib.h"

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

struct ci_cmp_pred
{
  // allow different types tag
  using is_transparent = void;

  struct converter
  {
    static auto get( const std::string& x ) -> std::string_view { return { x }; };
    static auto get( std::string_view x ) -> std::string_view { return x; };
    static auto get( FlyWeightString auto const& x ) -> std::string_view { return { x.get() }; };
    static auto get( const char* x ) -> std::string_view { return { x }; }
  };

  bool operator()( CmpString auto const& x1, CmpString auto const& x2 ) const
  {
    auto sv1 = converter::get( x1 );
    auto sv2 = converter::get( x2 );
    // benchmark showed that strnicmp is faster then lexicographical_compare, even if we have to
    // create string_views. my guess is that it gets completly optimized away and what is left is
    // one comparison vs two comparisons per iteration
    return strnicmp( sv1.data(), sv2.data(), std::min( sv1.size(), sv2.size() ) );
  }
};
}  // namespace Pol::Clib
