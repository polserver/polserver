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

struct ci_cmp_pred
{
  using is_transparent = void;

  bool operator()( StdString auto const& x1, StdString auto const& x2 ) const
  {
    return std::ranges::lexicographical_compare( x1, x2 );
  }
  bool operator()( const std::string& x1, const char* x2 ) const
  {
    return std::ranges::lexicographical_compare( x1, std::string_view{ x2 } );
  }
  bool operator()( const char* x1, const std::string& x2 ) const
  {
    return std::ranges::lexicographical_compare( std::string_view{ x1 }, x2 );
  }
};
}  // namespace Pol::Clib
