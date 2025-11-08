/** @file
 *
 * @par History
 */

#pragma once

#include <algorithm>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>

namespace Pol::Clib
{
template <typename T>
concept StdString = std::same_as<T, std::string> || std::same_as<T, std::string_view>;

template <typename T>
concept FlyWeightString = requires( T x ) {
  { x.get() } -> std::same_as<const std::string&>;
};

struct ci_cmp_pred
{
  using is_transparent = void;
  inline static const auto icomp = []( char x, char y )
  { return std::tolower( x ) < std::tolower( y ); };

  bool operator()( StdString auto const& x1, StdString auto const& x2 ) const
  {
    return std::ranges::lexicographical_compare( x1, x2, icomp );
  }
  bool operator()( StdString auto const& x1, const char* x2 ) const
  {
    return std::ranges::lexicographical_compare( x1, std::string_view{ x2 }, icomp );
  }
  bool operator()( const char* x1, StdString auto const& x2 ) const
  {
    return std::ranges::lexicographical_compare( std::string_view{ x1 }, x2, icomp );
  }

  bool operator()( FlyWeightString auto const& x1, FlyWeightString auto const& x2 ) const
  {
    return std::ranges::lexicographical_compare( x1.get(), x2.get(), icomp );
  }
  bool operator()( FlyWeightString auto const& x1, StdString auto const& x2 ) const
  {
    return std::ranges::lexicographical_compare( x1.get(), x2, icomp );
  }
  bool operator()( StdString auto const& x1, FlyWeightString auto const& x2 ) const
  {
    return std::ranges::lexicographical_compare( x1, x2.get(), icomp );
  }
  bool operator()( FlyWeightString auto const& x1, const char* x2 ) const
  {
    return std::ranges::lexicographical_compare( x1.get(), std::string_view{ x2 }, icomp );
  }
  bool operator()( const char* x1, FlyWeightString auto const& x2 ) const
  {
    return std::ranges::lexicographical_compare( std::string_view{ x1 }, x2.get(), icomp );
  }
};
}  // namespace Pol::Clib
