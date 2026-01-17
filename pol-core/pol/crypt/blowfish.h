//////////////////////////////////////////////////////////////////////
//
// crypt/blowfish.h
//
//////////////////////////////////////////////////////////////////////

#ifndef __BLOWFISH_H__
#define __BLOWFISH_H__

#include "cryptbase.h"

namespace Pol::Crypt
{
class BlowFish
{
  // Constructor / Destructor
public:
  BlowFish();
  ~BlowFish();

  // Member Functions

public:
  void Init();
  void Decrypt( unsigned char* in, unsigned char* out, int len );

protected:
  static bool tables_ready;
  unsigned char game_seed[CRYPT_GAMESEED_LENGTH];
  int table_index;
  int block_pos;
  int stream_pos;

  static void InitTables();
  static void RawDecrypt( unsigned int* values, int table );
};
}  // namespace Pol::Crypt

#endif  //__BLOWFISH_H__
