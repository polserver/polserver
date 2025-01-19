/** @file
 *
 * @par History
 */


#pragma once

#include "pol_global_config.h"
#include <algorithm>
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
  OnlineStatistics();
  void update( double value );
  double variance() const;
  double mean() const;
  u64 count() const;
  double max() const;

private:
  u64 _count;
  double _max;
  double _mean;
  double _m2;
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

// convert number of type U to type T considering the limits of T
template <typename T, typename U>
inline T clamp_convert( U v )
{
  // find in compile time the common type from both types to call clamp without overflowing

  constexpr auto t_min = std::numeric_limits<T>::min();
  constexpr auto t_max = std::numeric_limits<T>::max();
  // easy case T min max is contained in U eg from int to short
  if constexpr ( t_min >= std::numeric_limits<U>::min() && t_max <= std::numeric_limits<U>::max() )
    return static_cast<T>( std::clamp( v, static_cast<U>( t_min ), static_cast<U>( t_max ) ) );

  // for 64bit this would not work
  static_assert( !( ( std::is_same<U, u64>::value || std::is_same<U, s64>::value ) &&
                    ( std::is_same<T, u64>::value || std::is_same<T, s64>::value ) ) );
  // common_type will not use 64bit integer
  if constexpr ( ( std::is_same<U, u32>::value || std::is_same<T, u32>::value ) &&
                 ( std::is_same<U, s32>::value || std::is_same<T, s32>::value ) )
    return static_cast<T>(
        std::clamp( static_cast<s64>( v ), static_cast<s64>( t_min ), static_cast<s64>( t_max ) ) );


  typedef std::common_type_t<U, T> common;
  return static_cast<T>( std::clamp( static_cast<common>( v ), static_cast<common>( t_min ),
                                     static_cast<common>( t_max ) ) );
}
}  // namespace Pol::Clib
