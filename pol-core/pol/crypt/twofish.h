//////////////////////////////////////////////////////////////////////
//
// crypt/twofish.h
//
//////////////////////////////////////////////////////////////////////

#ifndef __TWOFISH_H__
#define __TWOFISH_H__

namespace Pol
{
namespace Crypt
{
// Structs for TwoFish

struct KeyInstance
{
  unsigned char direction;
  int keyLen;
  int numRounds;
  char keyMaterial[68];
  unsigned int keySig;
  unsigned int key32[8];
  unsigned int sboxKeys[4];
  unsigned int subKeys[40];
};

struct CipherInstance
{
  unsigned char mode;
  unsigned char IV[16];
  unsigned int cipherSig;
  unsigned int iv32[4];
};

class TwoFish
{
  // Constructor / Destructor
public:
  TwoFish();
  ~TwoFish();

  // Member Functions

public:
  void Init( unsigned char* gseed );
  void Decrypt( unsigned char* in, unsigned char* out, int len );

  unsigned char subData3[256];

protected:
  static unsigned int RS_MDS_Encode( unsigned int k0, unsigned int k1 );
  static unsigned int F32( unsigned int x, unsigned int* k32, int keyLen );
  static void ReKey( KeyInstance* key );
  static void CipherInit( CipherInstance* cipher, unsigned char mode, char* IV );
  void MakeKey( KeyInstance* key, unsigned char direction, int keyLen, char* keyMaterial );
  static void BlockEncrypt( CipherInstance* cipher, KeyInstance* key, unsigned char* input,
                            int inputLen, unsigned char* outBuffer );

  KeyInstance ki;
  CipherInstance ci;
  unsigned char tabUsed[256];
  unsigned int seed;
  unsigned int dwIndex;
  int tabEnable;
  int pos;
  int numRounds[4];
};

// TWOFISH Definitions

#define p8( N ) P8x8[P_##N]
#define RS_rem( x )                                                                \
  {                                                                                \
    unsigned char b = (unsigned char)( x >> 24 );                                  \
    unsigned int g2 = ( ( b << 1 ) ^ ( ( b & 0x80 ) ? 0x14D : 0 ) ) & 0xFF;        \
    unsigned int g3 = ( ( b >> 1 ) & 0x7F ) ^ ( ( b & 1 ) ? 0x14D >> 1 : 0 ) ^ g2; \
    x = ( x << 8 ) ^ ( g3 << 24 ) ^ ( g2 << 16 ) ^ ( g3 << 8 ) ^ b;                \
  }

#define LFSR1( x ) ( ( ( x ) >> 1 ) ^ ( ( ( x ) & 0x01 ) ? 0x169 / 2 : 0 ) )
#define LFSR2( x ) \
  ( ( ( x ) >> 2 ) ^ ( ( ( x ) & 0x02 ) ? 0x169 / 2 : 0 ) ^ ( ( ( x ) & 0x01 ) ? 0x169 / 4 : 0 ) )
#define Mx_1( x ) ( (unsigned int)( x ) )
#define Mx_X( x ) ( (unsigned int)( ( x ) ^ LFSR2( x ) ) )
#define Mx_Y( x ) ( (unsigned int)( ( x ) ^ LFSR1( x ) ^ LFSR2( x ) ) )
#define M00 Mul_1
#define M01 Mul_Y
#define M02 Mul_X
#define M03 Mul_X
#define M10 Mul_X
#define M11 Mul_Y
#define M12 Mul_Y
#define M13 Mul_1
#define M20 Mul_Y
#define M21 Mul_X
#define M22 Mul_1
#define M23 Mul_Y
#define M30 Mul_Y
#define M31 Mul_1
#define M32 Mul_Y
#define M33 Mul_X
#define Mul_1 Mx_1
#define Mul_X Mx_X
#define Mul_Y Mx_Y
#define P_00 1
#define P_01 0
#define P_02 0
#define P_03 ( P_01 ^ 1 )
#define P_04 1
#define P_10 0
#define P_11 0
#define P_12 1
#define P_13 ( P_11 ^ 1 )
#define P_14 0
#define P_20 1
#define P_21 1
#define P_22 0
#define P_23 ( P_21 ^ 1 )
#define P_24 0
#define P_30 0
#define P_31 1
#define P_32 1
#define P_33 ( P_31 ^ 1 )
#define P_34 1
#define ROL( x, n ) ( ( ( x ) << ( ( n ) & 0x1F ) ) | ( ( x ) >> ( 32 - ( ( n ) & 0x1F ) ) ) )
#define ROR( x, n ) ( ( ( x ) >> ( ( n ) & 0x1F ) ) | ( ( x ) << ( 32 - ( ( n ) & 0x1F ) ) ) )
#define Bswap( x ) ( x )
#define _b( x, N ) ( ( (unsigned char*)&x )[( ( N ) & 3 ) ^ 0] )
}  // namespace Crypt
}  // namespace Pol
#endif  //__TWOFISH_H__
