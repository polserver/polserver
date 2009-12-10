/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/logfile.h"
#include "../clib/strutil.h"

#include "crypt/cryptengine.h"
#include "crypt/nocrypt.h"
#include "crypt/blowfish.h"
#include "crypt/twofish.h"

#include "polcfg.h"

#include "crypt/cryptkey.h"

CCryptInfo g_CryptInfo;

CryptEngine* create_nocrypt_engine()
{
	return new CryptEngineTmpl<CCryptNoCrypt>;
}

CryptEngine* create_crypt_blowfish_engine(unsigned int uiKey1, unsigned int uiKey2)
{
	return new NewCryptEngineTmpl<CCryptBlowfish>(uiKey1, uiKey2);
}

CryptEngine* create_crypt_twofish_engine(unsigned int uiKey1, unsigned int uiKey2)
{
	// This is actually just the Blowfish engine until I work out how to impliment the new crap :/
	return new NewCryptEngineTmpl<CCryptTwofish>(uiKey1, uiKey2);
}

CryptEngine* create_crypt_blowfish_twofish_engine(unsigned int uiKey1, unsigned int uiKey2)
{
	// temp kludge until I work out how the f#%$ this new encryption scheme's meant to work! :-s
#define CCryptBlowfishTwofish CCryptBlowfish
	return new NewCryptEngineTmpl<CCryptBlowfishTwofish>(uiKey1, uiKey2);
}

// Defaults :)
CryptEngine* create_crypt_ignition_engine()
{
	return create_nocrypt_engine();
}

CryptEngine* create_crypt_engine( const string& name )
{
	TCryptInfo infoCrypt;
	if ( g_CryptInfo.LookupClient(name, infoCrypt) )
	{
		if ( !infoCrypt.bEnabled )
		{
			cerr << "Unsupported ClientEncryptionVersion "
				 << name
				 << ", using Ignition encryption engine"
				 << endl;
			Log("Unsupported ClientEncryptionVersion %s, using Ignition encryption engine\n",
				name.c_str() );
			return create_crypt_ignition_engine();
		}

		switch( infoCrypt.eType )
		{
			case CRYPT_NONE:
				return create_nocrypt_engine();
			case CRYPT_CLIENT:
				cerr << "Unsupported ClientEncryptionVersion "
					<< name
					<< ", using Ignition encryption engine"
					<< endl;
				Log("Unsupported ClientEncryptionVersion %s, using Ignition encryption engine\n",
					name.c_str() );
				return create_crypt_ignition_engine();
			case CRYPT_BLOWFISH:
				return create_crypt_blowfish_engine(infoCrypt.uiKey1, infoCrypt.uiKey2);
			case CRYPT_TWOFISH:
				return create_crypt_twofish_engine(infoCrypt.uiKey1, infoCrypt.uiKey2);
			case CRYPT_BOTH:
				return create_crypt_blowfish_twofish_engine(infoCrypt.uiKey1, infoCrypt.uiKey2);
			default:
				cerr << "Unknown encryption engine found in internal lookup table "
					<< "(" << name << "->" << infoCrypt.eType << ") "
					<< ", using Ignition encryption engine"
					<< endl;
				Log( "Unknown ClientEncryptionVersion %s, using Ignition encryption engine\n", 
					name.c_str() );
				return create_crypt_ignition_engine();
		}
	}
    else
    {
        cerr << "Unknown ClientEncryptionVersion " 
             << name 
             << ", using Ignition encryption engine"
             << endl;
        Log( "Unknown ClientEncryptionVersion %s, using Ignition encryption engine\n", 
              name.c_str() );
        return create_crypt_ignition_engine();
    }
}
