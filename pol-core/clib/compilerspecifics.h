/*
ATTENTION:
This header is part of the PCH
Remove the include in all StdAfx.h files or live with the consequences :)
*/

#ifndef CLIB_COMPILERSPECIFICS_H_
#define CLIB_COMPILERSPECIFICS_H_

// Fix for VS CodeAnalysis
// FIXME: disabled due to weird error with c++20
// #ifdef _MSC_VER
// #pragma warning( disable : 4467 )  // ATL attributes
// #include <CodeAnalysis/sourceannotations.h>
// #define passert_assume( x ) __analysis_assume( x )
// #else
#define passert_assume( x )
// #endif

// Note: POL_ANALYZER_NORETURN is just a hint for code analyzers (mostly clang-analyzer) and is not
//   used for code generation. I don't know of anything similar in the other analyzers and
// thought
//       of using POL_NORETURN here. Right now I'm not sure if this is needed at all.
#ifndef POL_ANALYZER_NORETURN
#ifndef __has_feature
#define __has_feature( x ) 0
#endif
#if __has_feature( attribute_analyzer_noreturn )
#define POL_ANALYZER_NORETURN __attribute__( ( analyzer_noreturn ) )
#else
#define POL_ANALYZER_NORETURN
#endif
#endif

#endif
