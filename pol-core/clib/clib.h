/** @file
 *
 * @par History
 */


#pragma once

#include "pol_global_config.h"
#include <algorithm>
#include <fmt/format.h>
#include <limits>
#include <type_traits>

#ifdef __GNUC__
#include <strings.h>
#endif

#ifdef _MSC_VER
#include <string.h>
#endif

#ifndef __STDDEF_H
#include "stddef.h"
#endif
#include "rawtypes.h"
#include <ctime>

#define ms_to_clocks( ms ) ( ms * CLOCKS_PER_SEC / 1000 )
/**
 * CLib namespace is for functions that a general app may need,
 * not necessarily related to POL at all (string manipulation, for example)
 */
namespace Pol::Clib
{
/** make an always null terminated string in maxlen characters */
char* stracpy( char* dest, const char* src, size_t maxlen );

/** perform a function on all files meeting a file spec */
int forspec( const char* spec, void ( *func )( const char* pathname ) );

/* Compiler specific implementations for case insensitive compare... */
#ifdef __GNUC__
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#ifdef _MSC_VER
#define stricmp _stricmp
#define strnicmp _strnicmp
#endif

/// returns the current process size in bytes
size_t getCurrentMemoryUsage();


template <typename T>
void sanitize_upperlimit( T* value, const T max )
{
  if ( *value > max )
    *value = max;
}
template <typename T>
void sanitize_lowerlimit( T* value, const T min )
{
  if ( *value < min )
    *value = min;
}
template <typename T>
void sanitize_bounds( T* value, const T min, const T max )
{
  if ( *value > max )
    *value = max;
  else if ( *value < min )
    *value = min;
}

/**
 * Class to calculate online statistics: mean, variance and max ref Knuth
 */
class OnlineStatistics
{
public:
  OnlineStatistics() = default;
  void update( double value );
  double variance() const;
  double standard_deviation() const;
  double mean() const;
  u64 count() const;
  double max() const;
  double total() const;

private:
  u64 _count = 0;
  double _max = 0;
  double _mean = 0;
  double _m2 = 0;
  double _total = 0;
};


/// threadsafe version of localtime
inline std::tm localtime( const std::time_t& t )
{
  std::tm result;
#ifdef _MSC_VER
  localtime_s( &result, &t );
#else
  localtime_r( &t, &result );
#endif
  return result;
}

// convert number of type From to type To considering the limits of To
template <typename To, typename From>
inline To clamp_convert( From v )
{
  constexpr auto t_min = std::numeric_limits<To>::min();
  constexpr auto t_max = std::numeric_limits<To>::max();
  typedef std::common_type_t<From, To> common;
  // If To's range is fully contained within From's range
  if constexpr ( std::is_signed<To>::value == std::is_signed<From>::value &&
                 t_min >= std::numeric_limits<From>::min() &&
                 t_max <= std::numeric_limits<From>::max() )
  {
    return static_cast<To>(
        std::clamp( v, static_cast<From>( t_min ), static_cast<From>( t_max ) ) );
  }
  else if constexpr ( std::is_same<To, u64>::value )
  {
    // Handle unsigned 64-bit special case
    return v < 0 ? 0u : static_cast<u64>( v );
  }
  else if constexpr ( std::is_same<To, s64>::value && std::is_same<From, u64>::value )
  {
    // Handle unsigned-to-signed 64-bit conversion
    return v > static_cast<u64>( t_max ) ? static_cast<u64>( t_max ) : static_cast<s64>( v );
  }
  else if constexpr ( std::is_unsigned<common>::value && std::is_signed<To>::value )
  {
    // Handle unsigned common type to signed
    return static_cast<To>( std::clamp( static_cast<common>( v ), static_cast<common>( 0 ),
                                        static_cast<common>( t_max ) ) );
  }
  else if constexpr ( std::is_signed<From>::value && std::is_unsigned<To>::value )
  {
    // Handle signed to unsigned
    return v <= 0 ? 0u
                  : static_cast<To>( std::clamp( static_cast<common>( v ), static_cast<common>( 0 ),
                                                 static_cast<common>( t_max ) ) );
  }
  else
  {
    // Default case for clamping
    return static_cast<To>( std::clamp( static_cast<common>( v ), static_cast<common>( t_min ),
                                        static_cast<common>( t_max ) ) );
  }
}
}  // namespace Pol::Clib
template <>
struct fmt::formatter<Pol::Clib::OnlineStatistics> : fmt::formatter<std::string>
{
  fmt::format_context::iterator format( const Pol::Clib::OnlineStatistics& s,
                                        fmt::format_context& ctx ) const;
};
