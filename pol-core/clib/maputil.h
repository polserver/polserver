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

namespace Pol::Clib
{
template <typename T>
concept FlyWeightString = requires( T x ) {
  { x.get() } -> std::same_as<const std::string&>;
};

template <typename T>
concept CmpString = std::same_as<T, std::string> || std::same_as<T, std::string_view> ||
                    FlyWeightString<T> || std::same_as<T, const char*>;

struct ci_cmp_pred
{
  using is_transparent = void;

  struct mapper
  {
    static auto get( const std::string& x ) -> const std::string& { return x; };
    static auto get( std::string_view x ) -> std::string_view { return x; };
    static auto get( FlyWeightString auto const& x ) -> const std::string& { return x.get(); };
    static auto get( const char* x ) -> std::string_view { return { x }; }
  };

  inline static const auto icomp = []( char x, char y )
  { return std::tolower( x ) < std::tolower( y ); };

  bool operator()( CmpString auto const& x1, CmpString auto const& x2 ) const
  {
    return std::ranges::lexicographical_compare( mapper::get( x1 ), mapper::get( x2 ), icomp );
  }
};
}  // namespace Pol::Clib
