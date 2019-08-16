//////////////////////////////////////////////////////////////////////
//
// crypt/crypt.h
//
//////////////////////////////////////////////////////////////////////

#ifndef __CRYPT_H__
#define __CRYPT_H__

#include "../../clib/network/sockets.h"
#include "blowfish.h"
#include "cryptbase.h"
#include "md5.h"
#include "twofish.h"

// NOCRYPT
namespace Pol
{
namespace Crypt
{
class CCryptNoCrypt final : public CCryptBase
{
  // Constructor / Destructor
public:
  typedef CCryptBase base;

  CCryptNoCrypt();
  virtual ~CCryptNoCrypt() = default;

  // Member Functions
public:
  virtual int Receive( void* buffer, int max_expected, SOCKET socket ) override;
  virtual void Init( void* pvSeed, int type = CCryptBase::typeAuto ) override;
};

// BLOWFISH

class CCryptBlowfish : public CCryptBaseCrypt
{
  // Constructor / Destructor
public:
  typedef CCryptBaseCrypt base;

  CCryptBlowfish();
  CCryptBlowfish( unsigned int masterKey1, unsigned int masterKey2 );
  virtual ~CCryptBlowfish() = default;

  BlowFish bfish;

  // Member Functions
public:
  virtual int Receive( void* buffer, int max_expected, SOCKET socket ) override;
  virtual void Init( void* pvSeed, int type = CCryptBase::typeAuto ) override;

protected:
  virtual void Decrypt( void* pvIn, void* pvOut, int len ) override;
};

// BLOWFISH OLD

class CCryptBlowfishOld final : public CCryptBlowfish
{
  // Constructor / Destructor
public:
  CCryptBlowfishOld();
  CCryptBlowfishOld( unsigned int masterKey1, unsigned int masterKey2 );
  virtual ~CCryptBlowfishOld() = default;

protected:
  virtual void Decrypt( void* pvIn, void* pvOut, int len ) override;
};

// BLOWFISH 1.25.36

class CCrypt12536 final : public CCryptBlowfish
{
  // Constructor / Destructor
public:
  CCrypt12536();
  CCrypt12536( unsigned int masterKey1, unsigned int masterKey2 );
  virtual ~CCrypt12536() = default;

protected:
  virtual void Decrypt( void* pvIn, void* pvOut, int len ) override;
};

// BLOWFISH + TWOFISH

class CCryptBlowfishTwofish final : public CCryptBaseCrypt
{
public:
  CCryptBlowfishTwofish();
  CCryptBlowfishTwofish( unsigned int masterKey1, unsigned int masterKey2 );
  virtual ~CCryptBlowfishTwofish() = default;

  BlowFish bfish;
  TwoFish tfish;

public:
  virtual int Receive( void* buffer, int max_expected, SOCKET socket ) override;
  virtual void Init( void* pvSeed, int type = CCryptBase::typeAuto ) override;

protected:
  virtual void Decrypt( void* pvIn, void* pvOut, int len ) override;
};

// TWOFISH

class CCryptTwofish final : public CCryptBaseCrypt
{
public:
  CCryptTwofish();
  CCryptTwofish( unsigned int masterKey1, unsigned int masterKey2 );
  virtual ~CCryptTwofish() = default;

  TwoFish tfish;
  MD5Crypt md5;

public:
  virtual int Receive( void* buffer, int max_expected, SOCKET socket ) override;
  virtual void Init( void* pvSeed, int type = CCryptBase::typeAuto ) override;
  virtual void Encrypt( void* pvIn, void* pvOut, int len ) override;

protected:
  virtual void Decrypt( void* pvIn, void* pvOut, int len ) override;
};
}  // namespace Crypt
}  // namespace Pol

#endif  //__CRYPT_H__
