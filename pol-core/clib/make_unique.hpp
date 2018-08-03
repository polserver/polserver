#pragma once

#include <memory>

namespace Pol
{
namespace Clib
{
template <typename T, typename... Args>
std::unique_ptr<T> make_unique( Args&&... args )
{
  return std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
}
}  // namespace Clib
}  // namespace Pol
