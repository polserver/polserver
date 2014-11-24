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

// NOCRYPT
namespace Pol {
  namespace Crypt {
	class CCryptNoCrypt : public CCryptBase
	{
	  // Constructor / Destructor
	public:
	  typedef CCryptBase base;

	  CCryptNoCrypt();
	  ~CCryptNoCrypt();

	  // Member Functions
	public:
	  int		Receive( void *buffer, int max_expected, SOCKET socket );
	  void	Init( void *pvSeed, int type = CCryptBase::typeAuto );
	};

	// BLOWFISH

	class CCryptBlowfish : public CCryptBaseCrypt
	{
	  // Constructor / Destructor
	public:
	  typedef CCryptBaseCrypt base;

	  CCryptBlowfish();
	  CCryptBlowfish( unsigned int masterKey1, unsigned int masterKey2 );
	  ~CCryptBlowfish();

	  BlowFish bfish;

	  // Member Functions
	public:
	  virtual int		Receive( void *buffer, int max_expected, SOCKET socket ) POL_OVERRIDE;
	  virtual void	Init( void *pvSeed, int type = CCryptBase::typeAuto ) POL_OVERRIDE;
	  virtual void	SetMasterKeys( unsigned int masterKey1, unsigned int masterKey2 ) POL_OVERRIDE;

	protected:
	  virtual void	Decrypt( void *pvIn, void *pvOut, int len ) POL_OVERRIDE;
	};

	// BLOWFISH OLD

	class CCryptBlowfishOld : public CCryptBlowfish
	{
	  // Constructor / Destructor
	public:
	  CCryptBlowfishOld();
	  CCryptBlowfishOld( unsigned int masterKey1, unsigned int masterKey2 );
	  ~CCryptBlowfishOld();

	protected:
	  void	Decrypt( void *pvIn, void *pvOut, int len );
	};

	// BLOWFISH 1.25.36

	class CCrypt12536 : public CCryptBlowfish
	{
	  // Constructor / Destructor
	public:
	  CCrypt12536();
	  CCrypt12536( unsigned int masterKey1, unsigned int masterKey2 );
	  ~CCrypt12536();

	protected:
	  void	Decrypt( void *pvIn, void *pvOut, int len );
	};

	// BLOWFISH + TWOFISH

	class CCryptBlowfishTwofish : public CCryptBaseCrypt
	{
	public:
	  CCryptBlowfishTwofish();
	  CCryptBlowfishTwofish( unsigned int masterKey1, unsigned int masterKey2 );
	  ~CCryptBlowfishTwofish();

	  BlowFish bfish;
	  TwoFish tfish;

	public:
	  int		Receive( void *buffer, int max_expected, SOCKET socket );
	  void	Init( void *pvSeed, int type = CCryptBase::typeAuto );
	  void	SetMasterKeys( unsigned int masterKey1, unsigned int masterKey2 );

	protected:
	  void	Decrypt( void *pvIn, void *pvOut, int len );
	};

	// TWOFISH

	class CCryptTwofish : public CCryptBaseCrypt
	{
	public:
	  CCryptTwofish();
	  CCryptTwofish( unsigned int masterKey1, unsigned int masterKey2 );
	  ~CCryptTwofish();

	  TwoFish tfish;
	  MD5Crypt md5;

	public:
	  int		Receive( void *buffer, int max_expected, SOCKET socket );
	  void	Init( void *pvSeed, int type = CCryptBase::typeAuto );
	  void	SetMasterKeys( unsigned int masterKey1, unsigned int masterKey2 );
	  void	Encrypt( void *pvIn, void *pvOut, int len );

	protected:
	  void	Decrypt( void *pvIn, void *pvOut, int len );
	};
  }
}

#endif //__CRYPT_H__
