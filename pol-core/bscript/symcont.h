/** @file
 *
 * @par History
 */


#ifndef __SYMCONT_H
#define __SYMCONT_H

#include <cstdio>

#include "../clib/rawtypes.h"

#ifndef __TOKENS_H
#include "tokens.h"
#endif

namespace Pol
{
namespace Bscript
{
#ifdef _MSC_VER
#pragma pack( push, 1 )
#endif

// bitfields
class StoredToken
{
public:
  unsigned char type;
  unsigned char id;
  unsigned short offset;
  unsigned char module;

  explicit StoredToken( unsigned char aModule = 0, int aID = CTRL_STATEMENTBEGIN,
                        BTokenType aType = TYP_CONTROL,
                        unsigned aOffset = 0  //,
                        // unsigned aNargs = 0
                        );
} POSTSTRUCT_PACK;
#ifdef _MSC_VER
#pragma pack( pop )
#endif

class SymbolContainer
{
protected:
  char* s;
  unsigned usedLen;
  unsigned allocLen;
  unsigned growBy;

public:
  explicit SymbolContainer( int growBy = 512 );
  virtual ~SymbolContainer();
  virtual void pack( void ) {}
  void erase();

  virtual void resize( unsigned lengthToAdd );
  bool findexisting( const void* data, int datalen, unsigned& position );
  void append( const char* string, unsigned& position );
  void append( int lvalue, unsigned& position );
  void append( double dvalue, unsigned& position );
  void append( void* data, unsigned datalen, unsigned& position );

  unsigned length( void ) const { return usedLen; }
  void setlength( unsigned newLen ) { usedLen = newLen; }
  const char* array( void ) const { return s; }
  virtual void* detach( void )
  {
    char* t = s;
    s = NULL;
    usedLen = allocLen = 0;
    return t;
  }
  virtual void write( FILE* fp );
  virtual void write( char* fname );
  unsigned int get_write_length() const;
  virtual void read( FILE* fp );
  virtual void read( char* fname );
};

/* talk to these in statement numbers */
class StoredTokenContainer : public SymbolContainer
{
public:
  StoredToken* ST;
  explicit StoredTokenContainer( int growBy = 512 ) : SymbolContainer( growBy ), ST( NULL ) {}
  virtual void pack( void ) POL_OVERRIDE;
  virtual void* detach( void ) POL_OVERRIDE;
  virtual void resize( unsigned lengthToAdd ) POL_OVERRIDE;
  void setcount( unsigned newCount ) { usedLen = newCount * sizeof( StoredToken ); }
  unsigned count( void ) const { return usedLen / sizeof( StoredToken ); }
  void append_tok( const StoredToken& token, unsigned* position = NULL );
  void atPut1( const StoredToken& token, unsigned position );
  void atGet1( unsigned position, StoredToken& token ) const;
  unsigned next( void ) { return length() / sizeof( StoredToken ); }
  virtual void read( FILE* fp ) POL_OVERRIDE;
};
}
}
#endif
