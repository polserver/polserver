// History
//   2006/09/26 Shinigami: GCC 3.4.x fix - added "this" to NewCryptEngineTmpl

#ifndef CRYPTENGINE_H
#define CRYPTENGINE_H

#include "../sockets.h"

class CryptEngine
{
protected:
	CryptEngine() {};
public:
	enum e_crypttype { typeLogin, typeGame, typeAuto } crypttype;

	virtual ~CryptEngine() {}
	virtual int Receive(void *buffer, int max_expected, SOCKET socket) = 0;
	virtual void Init(void *pvSeed, int type=typeAuto) = 0;
};

template<class T>
class CryptEngineTmpl : public CryptEngine
{
public:
	CryptEngineTmpl() : crypt() {}
	virtual int Receive(void *buffer, int max_expected, SOCKET socket)
	{
		return crypt.Receive(buffer, max_expected, socket);
	}
	virtual void Init(void *pvSeed, int type)
	{
		crypt.Init( pvSeed, type );
	}

protected:
	T crypt;
};

template<class T>
class NewCryptEngineTmpl : public CryptEngineTmpl<T>
{
public:
	NewCryptEngineTmpl(unsigned int Key1, unsigned int Key2)
	{
		this->crypt.SetMasterKeys(Key1, Key2);
	}
};

CryptEngine* create_crypt_engine( const std::string& name );

#endif
