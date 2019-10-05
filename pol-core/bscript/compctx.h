/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_COMPCTX_H
#define BSCRIPT_COMPCTX_H
#include "../clib/logfacility.h"
#include "compilercfg.h"

#include <iosfwd>
#include <string>

#include <format/format.h>

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
  void printOn( fmt::Writer& writer ) const;
  void printOnShort( std::ostream& os ) const;
  void printOnShort( fmt::Writer& writer ) const;

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

inline std::ostream& operator<<( std::ostream& os, const CompilerContext& ctx )
{
  ctx.printOn( os );
  return os;
}

inline fmt::Writer& operator<<( fmt::Writer& writer, const CompilerContext& ctx )
{
  ctx.printOn( writer );
  return writer;
}

namespace
{
inline void rec_write( fmt::Writer& /*w*/ ) {}
template <typename T, typename... Targs>
inline void rec_write( fmt::Writer& w, T&& value, Targs&&... Fargs )
{
  w << value;
  rec_write( w, std::forward<Targs>( Fargs )... );
}
}  // namespace

template <typename... Args>
inline void compiler_warning( CompilerContext* ctx, Args&&... args )
{
  if ( compilercfg.DisplayWarnings || compilercfg.ErrorOnWarning )
  {
    fmt::Writer w;
    rec_write( w, std::forward<Args>( args )... );

    if ( compilercfg.ErrorOnWarning )
      throw std::runtime_error( w.str() );
    else
    {
      if ( ctx != nullptr )
        w << *ctx;
      ERROR_PRINT << w.c_str();
    }
  }
}

template <typename... Args>
inline void compiler_error( Args&&... args )
{
  fmt::Writer w;
  rec_write( w, std::forward<Args>( args )... );
  ERROR_PRINT << w.str();
}
}  // namespace Bscript
}  // namespace Pol
#endif
