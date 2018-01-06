#ifndef POL_GLOBAL_CONFIG_WIN_H
#define POL_GLOBAL_CONFIG_WIN_H

// #define USE_BOOST_REGEX
// #define DEBUG_FLYWEIGHT
// #define MEMORYLEAK
// #define PERGON
// #define ESCRIPT_PROFILE
// #define GIT_REVISION
// #define HAVE_OPENSSL
// #define HAVE_MYSQL
// #define ENABLE_BENCHMARK

#define POL_VERSION_MAJOR 99
#define POL_VERSION_MINOR 0
#define POL_VERSION_PATCH 0
#define POL_VERSION_STR "99.0.0"
#define POL_VERSION_NAME "Break Everything Even Rudder"
#define POL_VERSION_ID "99.0.1 Break Everything Even Rudder"
#define POL_BUILD_DATE __DATE__
#define POL_BUILD_TIME __TIME__

#ifdef PERGON
#define POL_BUILD_TARGET_TAG_PERGON " - Pergon"
#else
#define POL_BUILD_TARGET_TAG_PERGON ""
#endif

#ifdef MEMORYLEAK
#define POL_BUILD_TARGET_TAG_MEMLEAK " - MemoryLeak"
#else
#define POL_BUILD_TARGET_TAG_MEMLEAK ""
#endif

#ifdef ESCRIPT_PROFILE
#define POL_BUILD_TARGET_TAG_PROFILE " - EscriptProfile"
#else
#define POL_BUILD_TARGET_TAG_PROFILE ""
#endif

#ifdef GIT_REVISION
#define POL_REVISION_TAG " (Rev. " GIT_REVISION ")"
#else
#define POL_REVISION_TAG ""
#endif

#if ( ARCH_BITS == 64 )
#define POL_64 " - 64bit"
#else
#define POL_64 ""
#endif

#if defined( LINUX )
#define POL_BUILD_TARGET                                                 \
  ( "UBUNTU" POL_BUILD_TARGET_TAG_PERGON "" POL_BUILD_TARGET_TAG_MEMLEAK \
    "" POL_BUILD_TARGET_TAG_PROFILE "" POL_64 "" POL_REVISION_TAG )
#elif defined( WINDOWS )
#if ( _MSC_VER <= 1700 )
#define POL_BUILD_TARGET                                                 \
  ( "VS2012" POL_BUILD_TARGET_TAG_PERGON "" POL_BUILD_TARGET_TAG_MEMLEAK \
    "" POL_BUILD_TARGET_TAG_PROFILE "" POL_64 "" POL_REVISION_TAG )
#elif ( _MSC_VER <= 1800 )
#define POL_BUILD_TARGET                                                 \
  ( "VS2013" POL_BUILD_TARGET_TAG_PERGON "" POL_BUILD_TARGET_TAG_MEMLEAK \
    "" POL_BUILD_TARGET_TAG_PROFILE "" POL_64 "" POL_REVISION_TAG )
#elif ( _MSC_VER <= 1900 )
#define POL_BUILD_TARGET                                                 \
  ( "VS2015" POL_BUILD_TARGET_TAG_PERGON "" POL_BUILD_TARGET_TAG_MEMLEAK \
    "" POL_BUILD_TARGET_TAG_PROFILE "" POL_64 "" POL_REVISION_TAG )
#elif ( _MSC_VER <= 2000 )
#define POL_BUILD_TARGET                                                 \
  ( "VS2017" POL_BUILD_TARGET_TAG_PERGON "" POL_BUILD_TARGET_TAG_MEMLEAK \
    "" POL_BUILD_TARGET_TAG_PROFILE "" POL_64 "" POL_REVISION_TAG )
#else
#define POL_BUILD_TARGET                                                    \
  ( "[UNKNOWN]" POL_BUILD_TARGET_TAG_PERGON "" POL_BUILD_TARGET_TAG_MEMLEAK \
    "" POL_BUILD_TARGET_TAG_PROFILE "" POL_64 "" POL_REVISION_TAG )
#endif
#else
#define POL_BUILD_TARGET                                                    \
  ( "[UNKNOWN]" POL_BUILD_TARGET_TAG_PERGON "" POL_BUILD_TARGET_TAG_MEMLEAK \
    "" POL_BUILD_TARGET_TAG_PROFILE "" POL_64 "" POL_REVISION_TAG )
#endif

#endif /* POL_GLOBAL_CONFIG_WIN_H */
