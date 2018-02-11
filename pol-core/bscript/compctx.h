/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_COMPCTX_H
#define BSCRIPT_COMPCTX_H

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
  CompilerContext( const CompilerContext& );
  CompilerContext& operator=( const CompilerContext& );

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
}
}
#endif
