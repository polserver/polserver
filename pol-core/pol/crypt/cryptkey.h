/********************************************************************
** cryptkey.h : Encryption keys (and 'type' codes)
**
** Created: 3/1/2003, 2:59
** Author:  TJ Houston <tjh@myrathi.co.uk>
**
*********************************************************************/
#ifndef __CRYPTKEY_H__
#define __CRYPTKEY_H__

#include <fmt/format.h>
#include <string>


namespace Pol::Crypt
{
// Encryption Types
enum ECryptType
{
  CRYPT_NOCRYPT = 0,  //- no encryption
  CRYPT_OLD_BLOWFISH = 1,
  CRYPT_1_25_36 = 2,
  CRYPT_BLOWFISH = 3,          //- <=2.0.0 (BlowFish)
  CRYPT_BLOWFISH_TWOFISH = 4,  //- 2.0.3 (BlowFish+TwoFish)
  CRYPT_TWOFISH = 5            //- >2.0.3 (TwoFish)
};
inline auto format_as( ECryptType c )
{
  return fmt::underlying( c );
}

// Encryption keys and type
struct TCryptInfo
{
  unsigned int uiKey1;
  unsigned int uiKey2;
  ECryptType eType;
};

void CalculateCryptKeys( const std::string& name, TCryptInfo& infoCrypt );
}  // namespace Pol::Crypt


#endif  //__CRYPTKEY_H__
