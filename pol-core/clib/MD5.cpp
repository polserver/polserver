/*
History
=======
2003/06/29 Dave: MD5_Encrypt(), changed the order. the "safe" method was also really slow, so do it only if the "fast" method fails.
                 also, store provider handle for further speed improvements

Notes
=======

*/

#include "stl_inc.h"

#include "stlutil.h"
#include "MD5.h"

#ifdef _WIN32
#define _WIN32_WINNT 0x0400 // need this for wincrypt.h to be even looked at
#include <windows.h>
#include <wincrypt.h>
#undef _WIN32_WINNT

HCRYPTPROV hProv = NULL;

bool MD5_Encrypt(const string& in, string& out)
{
	//bool bResult = true;

    //HCRYPTKEY hKey = NULL;
	//HCRYPTKEY hXchgKey = NULL;
    HCRYPTHASH hHash = NULL;

	if(!hProv)
    {
	    if (! CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	    {
		    if (! CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0))
		    {
			    cerr << "Error " << GetLastError() << " acquiring crypt context" << std::endl;
			    return false;
		    }
        }
    }

	if (! CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
	{
		cerr << "Error " << GetLastError() << " creating hash" << std::endl;
		return false;
    }

	// Hash password string.
	if (!CryptHashData(hHash, (const unsigned char*)(in.data()), static_cast<unsigned long>(in.length()), 0))
	{
		cerr << "Error " << GetLastError() << " adding data to hash" << std::endl;
		return false;
    }
    unsigned long len = 16;
	unsigned char buf[16];
    if (! CryptGetHashParam(hHash, HP_HASHVAL, buf, &len, 0))
	{
		cerr << "Error " << GetLastError() << " getting hash value" << std::endl;
		return false;
    }

	ostringstream os;
	for(unsigned int i=0; i<len; i++)
	{
		os << std::setfill('0') << std::setw(2) <<  hex << (int)buf[i];
	}
	out = os.str();

    CryptDestroyHash(hHash);
	return true;
}

void MD5_Cleanup()
{
    CryptReleaseContext(hProv, 0);
}

#elif defined(HAVE_OPENSSL)

#include <openssl/md5.h>

bool MD5_Encrypt(const string& in, string& out)
{
	unsigned char sum[16];

    MD5( reinterpret_cast<const unsigned char*>(in.c_str()), in.length(), sum );

	ostringstream os;
	for(unsigned int i=0; i<sizeof(sum); i++)
	{
		os << std::setfill('0') << std::setw(2) <<  hex << (int)sum[i];
	}
	out = os.str();

	return true;
}
void MD5_Cleanup()
{
    //OpenSSL cleanup, if any
}

#else
extern "C"{
#include "MD5.h"
}

bool MD5_Encrypt(const string& in, string& out)
{
	struct md5_ctx ctx;
	unsigned char sum[16];

    __md5_init_ctx (&ctx);
    __md5_process_bytes (in.c_str(), in.length(), &ctx);
    __md5_finish_ctx (&ctx, sum);

	ostringstream os;
	for(unsigned int i=0; i<sizeof(sum); i++)
	{
		os << std::setfill('0') << std::setw(2) <<  hex << (int)sum[i];
	}
	out = os.str();

	return true;
}

void MD5_Cleanup()
{
    //MD5 cleanup, if any. Consider storing ctx until server shutdown.
}

#endif

bool MD5_Compare(const string& a, const string& b)
{
	bool ret = false;
	if( stringicmp(a,b) == 0)
		ret = true;

	return ret;
}
