//////////////////////////////////////////////////////////////////////
//
// crypt/crypt.h
//
//////////////////////////////////////////////////////////////////////

#ifndef __CRYPT_H__
#define __CRYPT_H__

#include "cryptbase.h"
#include "twofish.h"
#include "blowfish.h"
#include "md5.h"
#include "../../clib/compilerspecifics.h"

// NOCRYPT
namespace Pol
{
namespace Crypt
{
class CCryptNoCrypt : public CCryptBase
{
  // Constructor / Destructor
public:
  typedef CCryptBase base;

  CCryptNoCrypt();
  virtual ~CCryptNoCrypt();

  // Member Functions
public:
  virtual int Receive( void* buffer, int max_expected, SOCKET socket ) POL_OVERRIDE;
  virtual void Init( void* pvSeed, int type = CCryptBase::typeAuto ) POL_OVERRIDE;
};

// BLOWFISH

class CCryptBlowfish : public CCryptBaseCrypt
{
  // Constructor / Destructor
public:
  typedef CCryptBaseCrypt base;

  CCryptBlowfish();
  CCryptBlowfish( unsigned int masterKey1, unsigned int masterKey2 );
  virtual ~CCryptBlowfish();

  BlowFish bfish;

  // Member Functions
public:
  virtual int Receive( void* buffer, int max_expected, SOCKET socket ) POL_OVERRIDE;
  virtual void Init( void* pvSeed, int type = CCryptBase::typeAuto ) POL_OVERRIDE;
  virtual void SetMasterKeys( unsigned int masterKey1, unsigned int masterKey2 ) POL_OVERRIDE;

protected:
  virtual void Decrypt( void* pvIn, void* pvOut, int len ) POL_OVERRIDE;
};

// BLOWFISH OLD

class CCryptBlowfishOld : public CCryptBlowfish
{
  // Constructor / Destructor
public:
  CCryptBlowfishOld();
  CCryptBlowfishOld( unsigned int masterKey1, unsigned int masterKey2 );
  virtual ~CCryptBlowfishOld();

protected:
  virtual void Decrypt( void* pvIn, void* pvOut, int len ) POL_OVERRIDE;
};

// BLOWFISH 1.25.36

class CCrypt12536 : public CCryptBlowfish
{
  // Constructor / Destructor
public:
  CCrypt12536();
  CCrypt12536( unsigned int masterKey1, unsigned int masterKey2 );
  virtual ~CCrypt12536();

protected:
  virtual void Decrypt( void* pvIn, void* pvOut, int len ) POL_OVERRIDE;
};

// BLOWFISH + TWOFISH

class CCryptBlowfishTwofish : public CCryptBaseCrypt
{
public:
  CCryptBlowfishTwofish();
  CCryptBlowfishTwofish( unsigned int masterKey1, unsigned int masterKey2 );
  virtual ~CCryptBlowfishTwofish();

  BlowFish bfish;
  TwoFish tfish;

public:
  virtual int Receive( void* buffer, int max_expected, SOCKET socket ) POL_OVERRIDE;
  virtual void Init( void* pvSeed, int type = CCryptBase::typeAuto ) POL_OVERRIDE;
  virtual void SetMasterKeys( unsigned int masterKey1, unsigned int masterKey2 ) POL_OVERRIDE;

protected:
  virtual void Decrypt( void* pvIn, void* pvOut, int len ) POL_OVERRIDE;
};

// TWOFISH

class CCryptTwofish : public CCryptBaseCrypt
{
public:
  CCryptTwofish();
  CCryptTwofish( unsigned int masterKey1, unsigned int masterKey2 );
  virtual ~CCryptTwofish();

  TwoFish tfish;
  MD5Crypt md5;

public:
  virtual int Receive( void* buffer, int max_expected, SOCKET socket ) POL_OVERRIDE;
  virtual void Init( void* pvSeed, int type = CCryptBase::typeAuto ) POL_OVERRIDE;
  virtual void SetMasterKeys( unsigned int masterKey1, unsigned int masterKey2 ) POL_OVERRIDE;
  virtual void Encrypt( void* pvIn, void* pvOut, int len ) POL_OVERRIDE;

protected:
  virtual void Decrypt( void* pvIn, void* pvOut, int len ) POL_OVERRIDE;
};
}
}

#endif  //__CRYPT_H__
