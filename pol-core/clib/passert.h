/*
History
=======
2005/01/17 Shinigami: added passert*_r, same as passert* with additional param "reason"
2005/09/16 Shinigami: scripts_thread_script* moved to passert

Notes
=======

*/

#ifndef CLIB_PASSERT_H
#define CLIB_PASSERT_H

#include "compilerspecifics.h"

#ifndef INC_PASSERT
#	define INC_PASSERT 0
#endif

#ifndef INC_PASSERT_PARANOID
#	ifdef NDEBUG
#		define INC_PASSERT_PARANOID 0
#	else
#		define INC_PASSERT_PARANOID INC_PASSERT
#	endif
#endif
namespace Pol {
  namespace Clib {
    extern bool passert_disabled;

    // what to do on an assertion failure:
    extern bool passert_dump_stack;
    extern bool passert_shutdown;
    extern bool passert_abort;
    extern bool passert_nosave;
    extern bool passert_shutdown_due_to_assertion;

    extern std::string scripts_thread_script;
    extern unsigned scripts_thread_scriptPC;

    //#if !defined(INC_PASSERT) && defined(NDEBUG)
    //#define INC_PASSERT 0
    //#elif !defined(INC_PASSERT) && !defined(NDEBUG)
    //#define INC_PASSERT 1
    //#endif

#undef  passert

	void force_backtrace(bool complete = false);

    POL_NORETURN void passert_failed( const char *expr, const char *file, unsigned line );
    POL_NORETURN void passert_failed( const char *expr, const std::string& reason, const char *file, unsigned line );
  }
#if  INC_PASSERT

#define passert(exp) (void)( Clib::passert_disabled || (exp) || (Clib::passert_failed(#exp, __FILE__, __LINE__), 0) ); passert_assume(exp)
#define passert_r(exp, reason) (void)( Clib::passert_disabled || (exp) || (Clib::passert_failed(#exp, reason, __FILE__, __LINE__), 0) ); passert_assume(exp)

#else

#define passert(exp)     ((void)0)

#endif

#define passert_always(exp) (void)( (exp) || (Clib::passert_failed(#exp, __FILE__, __LINE__), 0) ); passert_assume(exp)
#define passert_always_r(exp, reason) (void)( (exp) || (Clib::passert_failed(#exp, reason, __FILE__, __LINE__), 0) ); passert_assume(exp)

#if  INC_PASSERT_PARANOID

#define passert_paranoid(exp) (void)( (exp) || (Clib::passert_failed(#exp, __FILE__, __LINE__), 0) ); passert_assume(exp)
#define passert_paranoid_r(exp, reason) (void)( (exp) || (Clib::passert_failed(#exp, reason, __FILE__, __LINE__), 0) ); passert_assume(exp)

#else

#define passert_paranoid(exp)     ((void)0)

#endif
}
#endif
