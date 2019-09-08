/** @file
 *
 * @par History
 */


#ifndef CLIB_CLIB_H
#define CLIB_CLIB_H

#include "pol_global_config.h"

#ifndef __STDDEF_H
#include "stddef.h"
#endif
#include "rawtypes.h"
#include <ctime>

#define ms_to_clocks( ms ) ( ms * CLOCKS_PER_SEC / 1000 )
namespace Pol
{
/**
 * CLib namespace is for functions that a general app may need,
 * not necessarily related to POL at all (string manipulation, for example)
 */
namespace Clib
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
}  // namespace Clib
}  // namespace Pol

#endif  // CLIB_CLIB_H
