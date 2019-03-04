//////////////////////////////////////////////////////////////////////
//
// crypt/cryptbase.h
//
// Author:    Beosil <beosil@swileys.com>
// Date:      18. Apr. 2000
//
// Converted: Myrathi <tjh@myrathi.co.uk>
// Date:      03. Jan. 2003
//
// Copyright (C) 1999-2000 Bruno 'Beosil' Heidelberger
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//
// History:
// - 03. Jan. 2003 : Updated for use alongside POL's multi-encryption code.
// - 18. Apr. 2000 : Keys updated for client 2.0.0
// - 27. Jan. 2000 : Keys updated for client 1.26.4
// - 18. Jan. 2000 : Keys updated for client 1.26.3
// - 23. Nov. 1999 : Keys updated for client 1.26.2 and some compatibility fixes
// - 21. Sep. 1999 : Full redesign to fix the "21036 bytes"-bug
// -  9. Sep. 1999 : Keys updated for client 1.26.1
// -  2. Sep. 1999 : Keys and boxes updated for client 1.26.0b and minor bugfixes
// - 13. Aug. 1999 : First release, working with client 1.26.0(a)
//
//////////////////////////////////////////////////////////////////////

#ifndef __CRYPTBASE_H__
#define __CRYPTBASE_H__

#define CRYPT_AUTO_VALUE 0x80

#define CRYPT_GAMEKEY_LENGTH 6
#define CRYPT_GAMEKEY_COUNT 25

#define CRYPT_GAMESEED_LENGTH 8
#define CRYPT_GAMESEED_COUNT 25

#define CRYPT_GAMETABLE_START 1
#define CRYPT_GAMETABLE_STEP 3
#define CRYPT_GAMETABLE_MODULO 11
#define CRYPT_GAMETABLE_TRIGGER 21036

// Macro Definitions ( to avoid big-/little-endian problems )

#define N2L( C, LL )                                                                             \
  LL = ( (unsigned int)( *( ( C )++ ) ) ) << 24, LL |= ( (unsigned int)( *( ( C )++ ) ) ) << 16, \
  LL |= ( (unsigned int)( *( ( C )++ ) ) ) << 8, LL |= ( (unsigned int)( *( ( C )++ ) ) )
#define L2N( LL, C )                                                    \
  *( ( C )++ ) = (unsigned char)( ( ( LL ) >> 24 ) & 0xff ),            \
             *( ( C )++ ) = (unsigned char)( ( ( LL ) >> 16 ) & 0xff ), \
             *( ( C )++ ) = (unsigned char)( ( ( LL ) >> 8 ) & 0xff ),  \
             *( ( C )++ ) = (unsigned char)( ( ( LL ) ) & 0xff )
#define ROUND( LL, R, S, P )                                        \
  LL ^= P;                                                          \
  LL ^= ( ( S[( R >> 24 )] + S[0x0100 + ( ( R >> 16 ) & 0xff )] ) ^ \
          S[0x0200 + ( ( R >> 8 ) & 0xff )] ) +                     \
        S[0x0300 + ( (R)&0xff )]


#include "../../clib/network/sockets.h"
#include "../../plib/uconst.h"
#include "logincrypt.h"
namespace Pol
{
namespace Crypt
{
// basic class only used directly by NoCrypt
class CCryptBase
{
  // Constructor / Destructor
public:
  CCryptBase() = default;
  virtual ~CCryptBase() = default;

  enum e_crypttype
  {
    typeLogin,
    typeGame,
    typeAuto
  };

  // Member Functions
public:
  virtual int Receive( void* buffer, int max_expected, SOCKET socket ) = 0;
  virtual void Init( void* pvSeed, int type = typeAuto ) = 0;
  virtual void Encrypt( void* pvIn, void* pvOut, int len )
  {
    /* Do nothing. */
    (void)pvIn;
    (void)pvOut;
    (void)len;
  };
};

// crypt class

class CCryptBaseCrypt : public CCryptBase
{
  // Constructor / Destructor
public:
  CCryptBaseCrypt();
  virtual ~CCryptBaseCrypt() = default;

  LoginCrypt lcrypt;

  // Member Variables
protected:
  int m_type;
  unsigned int m_masterKey[2];
  unsigned char encrypted_data[MAXBUFFER];

  // Member Functions
protected:
  void SetMasterKeys( unsigned int masterKey1, unsigned int masterKey2 );
  virtual void Decrypt( void* pvIn, void* pvOut, int len ) = 0;
};
}  // namespace Crypt
}  // namespace Pol
#endif  //__CRYPTBASE_H__
