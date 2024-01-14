/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_COMPCTX_H
#define BSCRIPT_COMPCTX_H
#include "../clib/logfacility.h"
#include "compilercfg.h"

#include <iosfwd>
#include <iterator>
#include <string>

#include <fmt/format.h>

namespace Pol
{
namespace Bscript
{
/**
 * Represents a single compile context.
 *
 * A new compile context is created for every source file being compiled,
 * but a context can spawn itself many sub-contexts when needed
 */
class CompilerContext
{
public:
  CompilerContext();
  CompilerContext( const std::string& filename, int dbg_filenum, const char* s );
  CompilerContext( const CompilerContext& ) = default;
  CompilerContext& operator=( const CompilerContext& ) = default;

  void printOn( std::ostream& os ) const;

  void skipws();
  int skipcomments();

  /**
   * The code to be compiled, as null-terminated char sequence.
   * The pointer is moved forward when part of the string has been processed
   */
  const char* s;
  int line;
  std::string filename;

  const char* s_begin;

  int dbg_filenum;
  bool silence_unicode_warnings;  // tryLiteral gets executed multiple times flag used to ensure
                                  // that unicode warning gets only printed once
};

template <typename Str, typename... Args>
inline void compiler_warning( CompilerContext* ctx, Str const& format, Args&&... args )
{
  if ( compilercfg.DisplayWarnings || compilercfg.ErrorOnWarning )
  {
    std::string out = fmt::format( format, args... );

    if ( compilercfg.ErrorOnWarning )
      throw std::runtime_error( out );
    else
    {
      if ( ctx != nullptr )
        fmt::format_to( std::back_inserter( out ), "{}", *ctx );
      ERROR_PRINTLN( out );
    }
  }
}

template <typename Str, typename... Args>
inline void compiler_error( Str const& format, Args&&... args )
{
  ERROR_PRINTLN( format, args... );
}
}  // namespace Bscript
}  // namespace Pol
template <>
struct fmt::formatter<Pol::Bscript::CompilerContext> : fmt::formatter<std::string>
{
  fmt::format_context::iterator format( const Pol::Bscript::CompilerContext& c,
                                        fmt::format_context& ctx ) const;
};
#endif
