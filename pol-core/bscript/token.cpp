/** @file
 *
 * @par History
 * - 2006/10/06 Shinigami: malloc.h -> stdlib.h
 */

#include "token.h"

#include <cstdio>
#include <cstring>

namespace Pol
{
namespace Bscript
{
/**
 * Initializes an empty token
 */
Token::Token()
    : id( TOK_TERM ),
      type( TYP_TERMINATOR ),
      dval( 0.0 ),
      precedence( -1 ),
      dbg_filenum( 0 ),
      dbg_linenum( 0 ),
      lval( 0 ),
      deprecated( false ),
      ownsStr( false ),
      module( Mod_Basic ),
      token( nullptr )
{
}

/**
 * Initializes a token copying data from a given one
 */
Token::Token( const Token& tok )
    : id( tok.id ),
      type( tok.type ),
      dval( tok.dval ),
      precedence( tok.precedence ),
      dbg_filenum( tok.dbg_filenum ),
      dbg_linenum( tok.dbg_linenum ),
      lval( tok.lval ),
      deprecated( tok.deprecated ),
      ownsStr( false ),
      module( tok.module ),
      token( nullptr )
{
  if ( tok.token )
  {
    if ( !tok.ownsStr )
      setStr( tok.token );
    else
      copyStr( tok.token );
  }
}

/**
 * Assigns the values from a given token to this one
 */
Token& Token::operator=( const Token& tok )
{
  module = tok.module;
  id = tok.id;
  type = tok.type;
  precedence = tok.precedence;
  deprecated = tok.deprecated;

  nulStr();
  ownsStr = false;
  if ( tok.token )
  {
    if ( !tok.ownsStr )
      setStr( tok.token );
    else
      copyStr( tok.token );
  }
  dval = tok.dval;
  lval = tok.lval;

  dbg_filenum = tok.dbg_filenum;
  dbg_linenum = tok.dbg_linenum;

  return *this;
}

Token::Token( ModuleID i_module, BTokenId i_id, BTokenType i_type )
    : id( i_id ),
      type( i_type ),
      dval( 0.0 ),
      precedence( -1 ),
      dbg_filenum( 0 ),
      dbg_linenum( 0 ),
      lval( 0 ),
      deprecated( false ),
      ownsStr( false ),
      module( static_cast<unsigned char>( i_module ) ),
      token( nullptr )
{
}

Token::Token( BTokenId i_id, BTokenType i_type )
    : id( i_id ),
      type( i_type ),
      dval( 0.0 ),
      precedence( -1 ),
      dbg_filenum( 0 ),
      dbg_linenum( 0 ),
      lval( 0 ),
      deprecated( false ),
      ownsStr( false ),
      module( Mod_Basic ),
      token( nullptr )
{
}

/**
 * Erases all the content from the String (s), that becomes empty
 */
void Token::nulStr()
{
  if ( token && ownsStr )
  {
    char* tmp = (char*)token;
    delete[] tmp;
  }
  token = nullptr;
}

void Token::setStr( const char* s )
{
  nulStr();
  ownsStr = false;
  token = s;
}

/**
 * Copies value form the given null terminated char array into the String (s)
 */
void Token::copyStr( const char* s )
{
  nulStr();
  ownsStr = true;
  size_t len = strlen( s );
  auto tmp = new char[len + 1];
  if ( tmp )
  {
    memcpy( tmp, s, len + 1 );
    token = tmp;
  }
  else
  {
    token = nullptr;
  }
}

void Token::copyStr( const char* s, int len )
{
  nulStr();
  ownsStr = true;
  auto tmp = new char[static_cast<size_t>( len + 1 )];
  if ( tmp )
  {
    memcpy( tmp, s, len );
    tmp[len] = '\0';
    token = tmp;
  }
  else
  {
    token = nullptr;
  }
}

Token::~Token()
{
  nulStr();
}
}  // namespace Bscript
}  // namespace Pol
