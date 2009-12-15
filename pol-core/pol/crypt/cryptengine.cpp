/*
History
=======

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include "../../clib/logfile.h"
#include "../../clib/strutil.h"

#include "cryptengine.h"
#include "nocrypt.h"
#include "blowfish.h"
#include "twofish.h"

#include "../polcfg.h"

#include "cryptkey.h"

CCryptInfo g_CryptInfo;

CCryptBase* create_nocrypt_engine()
{
	return new CCryptNoCrypt();
}

CCryptBase* create_crypt_blowfish_engine(unsigned int uiKey1, unsigned int uiKey2)
{
	return new CCryptBlowfish(uiKey1, uiKey2);
}

CCryptBase* create_crypt_twofish_engine(unsigned int uiKey1, unsigned int uiKey2)
{
	// This is actually just the Blowfish engine until I work out how to impliment the new crap :/
	return new CCryptTwofish(uiKey1, uiKey2);
}

CCryptBase* create_crypt_blowfish_twofish_engine(unsigned int uiKey1, unsigned int uiKey2)
{
	// temp kludge until I work out how the f#%$ this new encryption scheme's meant to work! :-s
#define CCryptBlowfishTwofish CCryptBlowfish
	return new CCryptBlowfishTwofish(uiKey1, uiKey2);
}

CCryptBase* create_crypt_engine( const string& name )
{
	TCryptInfo infoCrypt;
	if ( g_CryptInfo.LookupClient(name, infoCrypt) )
	{
		switch( infoCrypt.eType )
		{
			case CRYPT_NOCRYPT:
				return create_nocrypt_engine();
			case CRYPT_OLD_BLOWFISH:
				return create_crypt_blowfish_engine(infoCrypt.uiKey1, infoCrypt.uiKey2);
			case CRYPT_1_25_36:
				return create_crypt_blowfish_engine(infoCrypt.uiKey1, infoCrypt.uiKey2);
			case CRYPT_BLOWFISH:
				return create_crypt_blowfish_engine(infoCrypt.uiKey1, infoCrypt.uiKey2);
			case CRYPT_TWOFISH:
				return create_crypt_twofish_engine(infoCrypt.uiKey1, infoCrypt.uiKey2);
			case CRYPT_BLOWFISH_TWOFISH:
				return create_crypt_blowfish_twofish_engine(infoCrypt.uiKey1, infoCrypt.uiKey2);
			default:
				cerr << "Unknown encryption engine found in internal lookup table "
					<< "(" << name << "->" << infoCrypt.eType << ") "
					<< ", using Ignition encryption engine"
					<< endl;
				Log( "Unknown ClientEncryptionVersion %s, using Ignition encryption engine\n", 
					name.c_str() );
				return create_nocrypt_engine();
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
        return create_nocrypt_engine();
    }
}
