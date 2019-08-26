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

/**
 * use instead of c-style (type) casts.
 *
 * Specify what you want to cast to, and
 * what you think you're casting from.
 */

#define ms_to_clocks( ms ) ( ms * CLOCKS_PER_SEC / 1000 )
namespace Pol
{
/**
 * CLib namespace is for functions that a general app may need,
 * not necessarily related to POL at all (string manipulation, for example)
 */
namespace Clib
{
/** how many file handles are available? */
int favail( void );

/** make a space-padded string */
char* strpcpy( char* dest, const char* src, size_t maxlen );

/** make an always null terminated string in maxlen characters */
char* stracpy( char* dest, const char* src, size_t maxlen );


/** do first strlen(b) chars of a and b match? match(s, "tag") */
int match( char* a, char* b );

/* strip leading, trailing spaces, trim multile whitespace to single space */
char* strip( char* s );

#ifdef __STDIO_H /* well there you go, not portable... why not standard the idempotent mechanism? \
                  */
/** open a unique file in a directory, no sharing allowed, "w+b" */
FILE* uniqfile( char* directory, char* pathname );
#endif
/** perform a function on all files meeting a file spec */
int forspec( const char* spec, void ( *func )( const char* pathname ) );

/**
 * make a char** array suitable for call to exec() or spawn()
 *
 * @return the argc in *pargc.
 */
char** decompose1( char* cmdline, int* pargc );

/** remove all occurrences of  characters not in allowed from s */
void legalize( char* s, const char* allowed );

/** how many words in s? nondestructive, assumes s is strip()ed */
int nwords( const char* s );

/** return nbits binary representation of val, in static buffer */
char* binary( unsigned int val, int nbits );

/** take a string, increment alphas, 'ZZZZ' converts to 'AAAA', etc */
void incStr( char* str );


/* Compiler specific implementations for case insensitive compare... */
#ifdef __GNUC__
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#ifdef _MSC_VER
#define stricmp _stricmp
#define strnicmp _strnicmp
#endif

#ifdef __cplusplus
#define PARAMS ...
#else
#define PARAMS
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
