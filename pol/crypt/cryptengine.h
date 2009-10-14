// History
//   2006/09/26 Shinigami: GCC 3.4.x fix - added "this" to NewCryptEngineTmpl

#ifndef CRYPTENGINE_H
#define CRYPTENGINE_H

class CryptEngine
{
protected:
    CryptEngine() {};
public:
    virtual ~CryptEngine() {}
    virtual void Decrypt(void *pvIn, void *pvOut, int len) = 0;
    virtual void Init(void *pvSeed) = 0;

};

template<class T>
class CryptEngineTmpl : public CryptEngine
{
public:
    CryptEngineTmpl() : crypt() {}
    virtual void Decrypt(void *pvIn, void *pvOut, int len) 
    {
        crypt.Decrypt( pvIn, pvOut, len );
    }
    virtual void Init(void *pvSeed)
    {
        crypt.Init( pvSeed );
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
