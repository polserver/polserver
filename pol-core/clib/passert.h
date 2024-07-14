/** @file
 *
 * @par History
 * - 2005/01/17 Shinigami: added passert*_r, same as passert* with additional param "reason"
 * - 2005/09/16 Shinigami: scripts_thread_script* moved to passert
 *
 * @warning This header is part of the PCH\n
 * Remove the include in all StdAfx.h files or live with the consequences :)
 */

#ifndef CLIB_PASSERT_H
#define CLIB_PASSERT_H

#include "compilerspecifics.h"
#include "weakptr.h"
#include <string>

#ifndef INC_PASSERT
#define INC_PASSERT 0
#endif

#ifndef INC_PASSERT_PARANOID
#ifdef NDEBUG
#define INC_PASSERT_PARANOID 0
#else
#define INC_PASSERT_PARANOID INC_PASSERT
#endif
#endif
namespace Pol
{
namespace Bscript
{
class Executor;
}
namespace Clib
{
// what to do on an assertion failure:
extern bool passert_dump_stack;
extern bool passert_shutdown;
extern bool passert_abort;
extern bool passert_nosave;
extern bool passert_shutdown_due_to_assertion;

extern std::string scripts_thread_script;
extern unsigned scripts_thread_scriptPC;
extern weak_ptr<Pol::Bscript::Executor> scripts_thread_exec_wptr;

//#if !defined(INC_PASSERT) && defined(NDEBUG)
//#define INC_PASSERT 0
//#elif !defined(INC_PASSERT) && !defined(NDEBUG)
//#define INC_PASSERT 1
//#endif

#undef passert

void force_backtrace( bool complete = false );

[[noreturn]] void passert_failed( const char* expr, const char* file, unsigned line );
[[noreturn]] void passert_failed( const char* expr, const std::string& reason, const char* file,
                                  unsigned line );
}  // namespace Clib
#if INC_PASSERT

/**
 * Works the same way as assert(), but is not stripped at non-debug compilation time.
 * Use this for non-crucial assertions.
 */
#define passert( exp )                                                          \
  (void)( ( exp ) || ( Clib::passert_failed( #exp, __FILE__, __LINE__ ), 0 ) ); \
  passert_assume( exp )
/** Just like passert(), but allows to specify a reason as additional parameter */
#define passert_r( exp, reason )                                                        \
  (void)( ( exp ) || ( Clib::passert_failed( #exp, reason, __FILE__, __LINE__ ), 0 ) ); \
  passert_assume( exp )

#else

#define passert( exp ) ( (void)0 )

#endif

/**
 * Works the same way as assert(), but is guaranteed to be never stripped.
 * Use this for critical assertions that can't be ignored.
 */
#define passert_always( exp )                                                   \
  (void)( ( exp ) || ( Clib::passert_failed( #exp, __FILE__, __LINE__ ), 0 ) ); \
  passert_assume( exp )
/** Just like passert_always(), but allows to specify a reason as additional parameter */
#define passert_always_r( exp, reason )                                                 \
  (void)( ( exp ) || ( Clib::passert_failed( #exp, reason, __FILE__, __LINE__ ), 0 ) ); \
  passert_assume( exp )

#if INC_PASSERT_PARANOID

/**
 * This works the same way as assert(), but is guaranteed to be stripped at non-debug compilation
 * time
 * Use this for trivial assertions: "I've proved it's impossible, but ..."
 */
#define passert_paranoid( exp )                                                 \
  (void)( ( exp ) || ( Clib::passert_failed( #exp, __FILE__, __LINE__ ), 0 ) ); \
  passert_assume( exp )
/** Just like passert_paranoid(), but allows to specify a reason as additional parameter */
#define passert_paranoid_r( exp, reason )                                               \
  (void)( ( exp ) || ( Clib::passert_failed( #exp, reason, __FILE__, __LINE__ ), 0 ) ); \
  passert_assume( exp )

#else

#define passert_paranoid( exp ) ( (void)0 )

#endif
}  // namespace Pol
#endif
