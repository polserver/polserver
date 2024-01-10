/** @file
 *
 * @par History
 */


#include "compctx.h"

#include <cstring>
#include <ctype.h>
#include <format/format.h>

namespace Pol
{
namespace Bscript
{
namespace
{
/**
 * Skips to the end of the line (or to the end of the file, whathever comes first)
 */
int eatToEndOfLine( CompilerContext& ctx )
{
  const char* t = ctx.s;

  while ( *t && ( *t != '\r' ) && ( *t != '\n' ) )
    t++;

  ctx.s = t;
  return 0;
}

/**
 * Skips to the end of a multiline comment, supports nested comments
 *
 * @return 0 on success
 */
int eatToCommentEnd( CompilerContext& ctx )
{
  CompilerContext tmp( ctx );

  while ( tmp.s[0] )
  {
    if ( strncmp( tmp.s, "*/", 2 ) == 0 )
    {
      tmp.s += 2;
      ctx = tmp;
      return 0;
    }
    else if ( strncmp( tmp.s, "/*", 2 ) == 0 )  // nested comment
    {
      tmp.s += 2;
      int res = eatToCommentEnd( tmp );
      if ( res )
        return res;
    }
    else if ( strncmp( tmp.s, "//", 2 ) == 0 )  // nested eol-comment
    {
      int res = eatToEndOfLine( tmp );
      if ( res )
        return res;
    }
    if ( tmp.s[0] == '\n' )
      ++tmp.line;
    tmp.s++;
  }
  return -1;
}
}  // namespace

CompilerContext::CompilerContext()
    : s( nullptr ),
      line( 1 ),
      filename( "" ),
      s_begin( nullptr ),
      dbg_filenum( 0 ),
      silence_unicode_warnings( false )
{
}

CompilerContext::CompilerContext( const std::string& filename, int dbg_filenum, const char* s )
    : s( s ),
      line( 1 ),
      filename( filename ),
      s_begin( s ),
      dbg_filenum( dbg_filenum ),
      silence_unicode_warnings( false )
{
}
/**
 * Skips whitespaces. Moves the pointer forward until a non-whitespace is found
 */
void CompilerContext::skipws()
{
  while ( isspace( s[0] ) )
  {
    if ( s[0] == '\n' )
      ++line;
    s++;
  }
}

int CompilerContext::skipcomments()
{
  CompilerContext tctx( *this );
  for ( ;; )
  {
    while ( tctx.s[0] && isspace( tctx.s[0] ) )
    {
      // FIXME: if (tctx.s[0] == '\t')
      // FIXME:     contains_tabs = true;
      if ( tctx.s[0] == '\n' )
        ++tctx.line;
      tctx.s++;  // wow neat, already take care of newlines
    }

    if ( !tctx.s[0] )
    {
      ( *this ) = tctx;
      return 1;
    }

    // look for comments
    if ( strncmp( tctx.s, "/*", 2 ) == 0 )  // got a start of comment
    {
      int res;
      tctx.s += 2;
      res = eatToCommentEnd( tctx );
      if ( res )
        return res;
    }
    else if ( strncmp( tctx.s, "//", 2 ) == 0 )  // comment, one line only
    {
      int res;
      res = eatToEndOfLine( tctx );
      if ( res )
        return res;
    }
    else
    {
      break;
    }
  }

  ( *this ) = tctx;

  return 0;
}

void CompilerContext::printOn( std::ostream& os ) const
{
  os << "File: " << filename << ", Line " << line << std::endl;
}

void CompilerContext::printOn( fmt::Writer& writer ) const
{
  writer << "File: " << filename << ", Line " << line << "\n";
}
}  // namespace Bscript
}  // namespace Pol

fmt::format_context::iterator fmt::formatter<Pol::Bscript::CompilerContext>::format(
    const Pol::Bscript::CompilerContext& c, fmt::format_context& ctx ) const
{
  return fmt::formatter<std::string>::format(
      fmt::format( "File: {}, Line {}", c.filename, c.line ), ctx );
}
