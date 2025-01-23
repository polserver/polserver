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
unsigned int Token::_instances = 0;

#if STORE_INSTANCELIST
set<Token*> Token::_instancelist;
#endif

unsigned int Token::instances()
{
  return _instances;
}
void Token::show_instances()
{
#if STORE_INSTANCELIST
  for ( Instances::iterator itr = _instancelist.begin(), end = _instancelist.end(); itr != end;
        ++itr )
  {
    Token* tkn = ( *itr );
    cout << tkn << ": " << ( *tkn ) << endl;
  }
#endif
}

void Token::register_instance()
{
  ++_instances;
#if STORE_INSTANCELIST
  _instancelist.insert( this );
#endif
}

void Token::unregister_instance()
{
  --_instances;
#if STORE_INSTANCELIST
  _instancelist.erase( this );
#endif
}

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
      userfunc( nullptr ),
      deprecated( false ),
      ownsStr( false ),
      module( Mod_Basic ),
      token( nullptr )
{
  register_instance();
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
      userfunc( tok.userfunc ),
      deprecated( tok.deprecated ),
      ownsStr( false ),
      module( tok.module ),
      token( nullptr )
{
  register_instance();
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

  userfunc = tok.userfunc;

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
      userfunc( nullptr ),
      deprecated( false ),
      ownsStr( false ),
      module( static_cast<unsigned char>( i_module ) ),
      token( nullptr )
{
  register_instance();
}

Token::Token( BTokenId i_id, BTokenType i_type )
    : id( i_id ),
      type( i_type ),
      dval( 0.0 ),
      precedence( -1 ),
      dbg_filenum( 0 ),
      dbg_linenum( 0 ),
      lval( 0 ),
      userfunc( nullptr ),
      deprecated( false ),
      ownsStr( false ),
      module( Mod_Basic ),
      token( nullptr )
{
  register_instance();
}

Token::Token( ModuleID i_module, BTokenId i_id, BTokenType i_type, UserFunction* i_userfunc )
    : id( i_id ),
      type( i_type ),
      dval( 0.0 ),
      precedence( -1 ),
      dbg_filenum( 0 ),
      dbg_linenum( 0 ),
      lval( 0 ),
      userfunc( i_userfunc ),
      deprecated( false ),
      ownsStr( false ),
      module( static_cast<unsigned char>( i_module ) ),
      token( nullptr )
{
  register_instance();
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
  unregister_instance();
}
}  // namespace Bscript
}  // namespace Pol
