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

// Ripped out of UOInjection_01's UOKeys.cfg ;)
// About time we made the damn hackers' time work -for- us. :P

// Encryption Types
enum ECryptType
{
	CRYPT_NONE      = 0,	//- no encryption
	CRYPT_CLIENT    = 1,	//- the same encryption that client uses
	CRYPT_BLOWFISH  = 2,	//- <=2.0.0 (BlowFish)
	CRYPT_BOTH      = 3,	//- 2.0.3 (BlowFish+TwoFish)
	CRYPT_TWOFISH   = 4		//- >2.0.3 (TwoFish)
};

// Encryption keys and type
typedef struct _CryptInfo
{
	unsigned int uiKey1;
	unsigned int uiKey2;
	ECryptType   eType;
	bool         bEnabled;
} TCryptInfo;

// STL map of [client->encryption info]
typedef std::map<std::string, TCryptInfo> CryptInfoList;

// Crypt class
class CCryptInfo
{
public:
	CCryptInfo();
	~CCryptInfo();

	bool LookupClient( std::string sClient, TCryptInfo & refInfo );

protected:
	void InitList( void );
	bool AddClient( std::string sClient,
						   unsigned int uiKey1,
						   unsigned int uiKey2,
						   ECryptType eType,
						   bool bEnabled = true);

private:
	CryptInfoList CryptList;
};

#endif //__CRYPTKEY_H__
