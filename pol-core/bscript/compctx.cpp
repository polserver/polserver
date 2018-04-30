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
  Clib::UnicodeStringIterator t = ctx.s;

  while ( ( *t != '\0' ) && ( *t != '\r' ) && ( *t != '\n' ) )
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
    if ( strncmp( tmp.s.ptr(), "*/", 2 ) == 0 )
    {
      tmp.s += 2;
      ctx = tmp;
      return 0;
    }
    else if ( strncmp( tmp.s.ptr(), "/*", 2 ) == 0 )  // nested comment
    {
      tmp.s += 2;
      int res = eatToCommentEnd( tmp );
      if ( res )
        return res;
    }
    else if ( strncmp( tmp.s.ptr(), "//", 2 ) == 0 )  // nested eol-comment
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
    : str( std::make_shared<UnicodeString>("") ), s( str->begin() ), s_begin( str->begin() ),
      line( 1 ), filename( "" ), dbg_filenum( 0 )
{
}

CompilerContext::CompilerContext( const std::string& filename, int dbg_filenum,
    const std::shared_ptr<UnicodeString>& str ) : str( str ), s( str->begin() ),
    s_begin( str->begin() ), line( 1 ), filename( filename ),
    dbg_filenum( dbg_filenum )
{
}

CompilerContext::CompilerContext( const CompilerContext& ctx )
    : str( ctx.str ),
      s( ctx.s ),
      s_begin( ctx.s_begin ),

      line( ctx.line ),
      filename( ctx.filename ),
      dbg_filenum( ctx.dbg_filenum )
{
}

CompilerContext& CompilerContext::operator=( const CompilerContext& rhs )
{
  str = rhs.str;
  s = rhs.s;
  s_begin = rhs.s_begin;

  line = rhs.line;
  filename = rhs.filename;
  dbg_filenum = rhs.dbg_filenum;

  return *this;
}


/**
 * Skips whitespaces. Moves the pointer forward until a non-whitespace is found
 */
void CompilerContext::skipws()
{
  while ( s->isSpace() )
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
    while ( tctx.s[0] && tctx.s->isSpace() )
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
    if ( strncmp( tctx.s.ptr(), "/*", 2 ) == 0 )  // got a start of comment
    {
      int res;
      tctx.s += 2;
      res = eatToCommentEnd( tctx );
      if ( res )
        return res;
    }
    else if ( strncmp( tctx.s.ptr(), "//", 2 ) == 0 )  // comment, one line only
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

/** Prints current file name and position on given stream */
void CompilerContext::printOn( std::ostream& os ) const
{
  os << "File: " << filename << ", Line " << line << std::endl;
}

/** Prints current file name and position on given writer */
void CompilerContext::printOn( fmt::Writer& writer ) const
{
  writer << "File: " << filename << ", Line " << line << "\n";
}

/** Prints current position only on given stream */
void CompilerContext::printOnShort( std::ostream& os ) const
{
  os << filename << ", Line " << line << std::endl;
}

/** Prints current position only on given writer */
void CompilerContext::printOnShort( fmt::Writer& writer ) const
{
  writer << filename << ", Line " << line << "\n";
}
}
}
