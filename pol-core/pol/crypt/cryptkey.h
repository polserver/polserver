/********************************************************************
** cryptkey.h : Encryption keys (and 'type' codes)
**
** Created: 3/1/2003, 2:59
** Author:  TJ Houston <tjh@myrathi.co.uk>
**
*********************************************************************/
#ifdef _MSC_VER
#pragma once
#endif

#ifndef __CRYPTKEY_H__
#define __CRYPTKEY_H__
namespace Pol {
  namespace Crypt {
	// Encryption Types
	enum ECryptType
	{
	  CRYPT_NOCRYPT = 0,	//- no encryption
	  CRYPT_OLD_BLOWFISH = 1,
	  CRYPT_1_25_36 = 2,
	  CRYPT_BLOWFISH = 3,	//- <=2.0.0 (BlowFish)
	  CRYPT_BLOWFISH_TWOFISH = 4,	//- 2.0.3 (BlowFish+TwoFish)
	  CRYPT_TWOFISH = 5		//- >2.0.3 (TwoFish)
	};

	// Encryption keys and type
	typedef struct _CryptInfo
	{
	  unsigned int uiKey1;
	  unsigned int uiKey2;
	  ECryptType   eType;
	} TCryptInfo;

	void CalculateCryptKeys( const std::string& name, TCryptInfo& infoCrypt );
  }
}

#endif //__CRYPTKEY_H__
