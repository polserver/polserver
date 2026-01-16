/** @file
 *
 * @par History
 * - 2006/10/06 Shinigami: malloc.h -> stdlib.h
 */

#include "token.h"

#include <cstdio>
#include <cstring>


namespace Pol::Bscript
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
      module( Mod_Basic ),
      token()
{
}

/**
 * Erases all the content from the String (s), that becomes empty
 */
void Token::nulStr()
{
  token.clear();
}

void Token::setStr( std::string str )
{
  token = std::move( str );
}

}  // namespace Pol::Bscript
