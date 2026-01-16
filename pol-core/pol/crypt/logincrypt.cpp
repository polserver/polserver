#include <string.h>

#include "logincrypt.h"


namespace Pol::Crypt
{
// Macro Definitions ( to avoid big-/little-endian problems )
#define N2L( C, LL )                                                                             \
  LL = ( (unsigned int)( *( ( C )++ ) ) ) << 24, LL |= ( (unsigned int)( *( ( C )++ ) ) ) << 16, \
  LL |= ( (unsigned int)( *( ( C )++ ) ) ) << 8, LL |= ( (unsigned int)( *( ( C )++ ) ) )

// Constructor / Destructor
LoginCrypt::LoginCrypt() : m_k1( 0 ), m_k2( 0 )
{
  memset( &lkey, 0, sizeof( lkey ) );
}
LoginCrypt::~LoginCrypt() = default;

// Member Functions
void LoginCrypt::Init( unsigned char* lseed, unsigned int k1, unsigned int k2 )
{
  unsigned int seed;
  N2L( lseed, seed );

  lkey[0] = ( ( ( ~seed ) ^ 0x00001357 ) << 16 ) | ( ( seed ^ 0xffffaaaa ) & 0x0000ffff );
  lkey[1] = ( ( seed ^ 0x43210000 ) >> 16 ) | ( ( ( ~seed ) ^ 0xabcdffff ) & 0xffff0000 );

  m_k1 = k1;
  m_k2 = k2;
}

void LoginCrypt::Decrypt( unsigned char* in, unsigned char* out, int len )
{
  for ( int i = 0; i < len; i++ )
  {
    out[i] = in[i] ^ (unsigned char)lkey[0];

    unsigned int table[2];
    table[0] = lkey[0];
    table[1] = lkey[1];

    lkey[1] =
        ( ( ( ( ( table[1] >> 1 ) | ( table[0] << 31 ) ) ^ m_k1 ) >> 1 ) | ( table[0] << 31 ) ) ^
        m_k1;
    lkey[0] = ( ( table[0] >> 1 ) | ( table[1] << 31 ) ) ^ m_k2;
  }
}

void LoginCrypt::Decrypt_Old( unsigned char* in, unsigned char* out, int len )
{
  for ( int i = 0; i < len; i++ )
  {
    out[i] = in[i] ^ (unsigned char)lkey[0];

    unsigned int table[2];
    table[0] = lkey[0];
    table[1] = lkey[1];

    lkey[0] = ( ( table[0] >> 1 ) | ( table[1] << 31 ) ) ^ m_k2;
    lkey[1] = ( ( table[1] >> 1 ) | ( table[0] << 31 ) ) ^ m_k1;
  }
}

void LoginCrypt::Decrypt_1_25_36( unsigned char* in, unsigned char* out, int len )
{
  for ( int i = 0; i < len; i++ )
  {
    out[i] = in[i] ^ (unsigned char)lkey[0];

    unsigned int table[2];
    table[0] = lkey[0];
    table[1] = lkey[1];

    lkey[1] = ( m_k1 >> ( ( 5 * table[1] * table[1] ) & 0xff ) ) + ( table[1] * m_k1 ) +
              ( table[0] * table[0] * 0x35ce9581 ) + 0x07afcc37;
    lkey[0] = ( m_k2 >> ( ( 3 * table[0] * table[0] ) & 0xff ) ) + ( table[0] * m_k2 ) -
              ( lkey[1] * lkey[1] * 0x4c3a1353 ) + 0x16ef783f;
  }
}
}  // namespace Pol::Crypt
