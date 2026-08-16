/** @file
 *
 * @par History
 */

#include <algorithm>
#include <string>

#include "bscript/executor.h"
#include "pol/testing/testenv.h"


namespace Pol::Testing
{
namespace
{
const std::string escript_stack =
    "at check_target (ai/guard.src:88)\n"
    "at on_tick (ai/guard.src:41)\n"
    "at <program> (ai/guard.src:7)";

const std::string native_stack =
    " 0# Pol::Core::UOExecutorModule::mf_ApplyDamage at uomod.cpp:1234\n"
    " 1# Pol::Bscript::Executor::execInstr at executor.cpp:3761\n";

std::string report( const std::string& escript, const std::string& native )
{
  return Bscript::format_execution_error( "ai/guard.ecl", 4711, 1234, "Scripts",
                                          "std::runtime_error: boom", escript, native );
}

// A stack made of `count` identical frames, to drive the caps.
std::string repeated_frames( size_t count )
{
  std::string result;
  for ( size_t i = 0; i < count; ++i )
    result += "at frame (x.src:1)\n";
  return result;
}

size_t line_count( const std::string& s )
{
  return static_cast<size_t>( std::count( s.begin(), s.end(), '\n' ) );
}

bool contains( const std::string& haystack, const std::string& needle )
{
  return haystack.find( needle ) != std::string::npos;
}
}  // namespace

void execution_error_format_test()
{
#define T_FMT( expr, expected, msg ) UnitTest( []() { return ( expr ); }, expected, " " msg )

  // The whole layout, asserted exactly once. Everything below this picks at a
  // single property; this is the test that notices a change in the shape.
  T_FMT( report( escript_stack, native_stack ),
         std::string( "Exception in ai/guard.ecl (pid 4711) PC 1234, thread Scripts\n"
                      "  std::runtime_error: boom\n"
                      "\n"
                      "eScript stack (innermost first):\n"
                      "  at check_target (ai/guard.src:88)\n"
                      "  at on_tick (ai/guard.src:41)\n"
                      "  at <program> (ai/guard.src:7)\n"
                      "\n"
                      "Native stack (throw site, innermost first):\n"
                      "   0# Pol::Core::UOExecutorModule::mf_ApplyDamage at uomod.cpp:1234\n"
                      "   1# Pol::Bscript::Executor::execInstr at executor.cpp:3761\n" ),
         "full report matches the documented layout" );

  // The script block comes first: it is the answer for most readers and the one
  // that survives log truncation.
  T_FMT( report( escript_stack, native_stack ).find( "eScript stack" ) <
             report( escript_stack, native_stack ).find( "Native stack" ),
         true, "script block precedes native block" );

  // Innermost first in both, i.e. neither list is reversed on the way in.
  T_FMT( report( escript_stack, native_stack ).find( "check_target" ) <
             report( escript_stack, native_stack ).find( "<program>" ),
         true, "eScript frames stay innermost-first" );
  T_FMT( report( escript_stack, native_stack ).find( "mf_ApplyDamage" ) <
             report( escript_stack, native_stack ).find( "execInstr" ),
         true, "native frames stay innermost-first" );

  // An absent stack drops its heading. Empty native traces are the normal case
  // on macOS (no from_exception there) and possible anywhere getTrace fails, so
  // a dangling heading would be a heading readers see routinely.
  T_FMT( contains( report( escript_stack, "" ), "Native stack" ), false,
         "empty native trace omits its heading" );
  T_FMT( contains( report( escript_stack, "" ), "eScript stack" ), true,
         "...without taking the script block with it" );
  T_FMT( contains( report( "", native_stack ), "eScript stack" ), false,
         "empty script stack omits its heading" );
  T_FMT( contains( report( "", "" ), "boom" ), true, "message survives both stacks missing" );

  // A trailing newline is what boost's to_string produces; it must not become a
  // blank indented frame.
  T_FMT( contains( report( escript_stack, native_stack ), "\n  \n" ), false,
         "trailing newline in a stack yields no blank frame" );

  // Caps. eScript recursion is unbounded and script-controlled.
  T_FMT( contains( report( repeated_frames( 30 ), "" ), "more)" ), false,
         "30 eScript frames are reported in full" );
  T_FMT( contains( report( repeated_frames( 34 ), "" ), "... (4 more)" ), true,
         "beyond the cap the remainder is counted, not dropped silently" );
  // 30 frames + 1 count line + header(1) + what(1) + blank(1) + heading(1).
  T_FMT( line_count( report( repeated_frames( 500 ), "" ) ), static_cast<size_t>( 35 ),
         "a runaway eScript stack cannot flood the log" );
  T_FMT( contains( report( "", repeated_frames( 120 ) ), "... (20 more)" ), true,
         "the native block is capped too" );

  // pid 0 is a plain Executor -- runecl, the compiler's tests -- and printing
  // "(pid 0)" would read as a real process.
  T_FMT( contains( Bscript::format_execution_error( "x.ecl", 0, 7, "Main", "e", "", "" ), "(pid" ),
         false, "a pidless executor prints no pid" );
  T_FMT( contains( Bscript::format_execution_error( "x.ecl", 0, 7, "", "e", "", "" ), "thread" ),
         false, "an unnamed thread prints no thread" );

#undef T_FMT
}
}  // namespace Pol::Testing
