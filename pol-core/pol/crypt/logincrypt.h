//////////////////////////////////////////////////////////////////////
//
// crypt/logincrypt.h
//
//////////////////////////////////////////////////////////////////////

#ifndef __LOGINCRYPT_H__
#define __LOGINCRYPT_H__

namespace Pol::Crypt
{
class LoginCrypt
{
  // Constructor / Destructor
public:
  LoginCrypt();
  ~LoginCrypt();

  // Member Functions

public:
  void Init( unsigned char* lseed, unsigned int k1, unsigned int k2 );
  void Decrypt( unsigned char* in, unsigned char* out, int len );
  void Decrypt_Old( unsigned char* in, unsigned char* out, int len );
  void Decrypt_1_25_36( unsigned char* in, unsigned char* out, int len );

  unsigned int lkey[2];

protected:
  unsigned int m_k1, m_k2;
};
}  // namespace Pol::Crypt

#endif  //__LOGINCRYPT_H__
