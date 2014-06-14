#ifndef CLIB_COMPILERSPECIFICS_H_
#define CLIB_COMPILERSPECIFICS_H_

// Fix for VS CodeAnalysis
#ifdef _MSC_VER
#  include <CodeAnalysis/sourceannotations.h>
#  define passert_assume(x) __analysis_assume(x)
#else
#  define passert_assume(x)
#endif



// Defines noreturn attribute using VC++ specifics, C++11-way for modern compilers. :P
// Note: noreturn can be used for code-generation and is supposed to produce more optimized code
#ifndef POL_NORETURN
#	ifdef _MSC_VER
#		define POL_NORETURN _declspec(noreturn)
#	else
#       ifdef __GNUC__
#           define POL__GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#           if POL__GCC_VERSION <= 407
#		        define POL_NORETURN __attribute__((noreturn))
#           else
#   		    define POL_NORETURN [[noreturn]]
#           endif
#           undef POL__GCC_VERSION
#       else
#   		define POL_NORETURN [[noreturn]]
#       endif
#	endif
#endif

// Note: POL_ANALYZER_NORETURN is just a hint for code analyzers (mostly clang-analyzer) and is not
//		 used for code generation. I don't know of anything similar in the other analyzers and thought
//       of using POL_NORETURN here. Right now I'm not sure if this is needed at all.
#ifndef POL_ANALYZER_NORETURN
#	ifndef __has_feature
#		define __has_feature(x) 0
#	endif
#	if __has_feature(attribute_analyzer_noreturn)
#		define POL_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
#	else
#		define POL_ANALYZER_NORETURN
#	endif
#endif

#endif