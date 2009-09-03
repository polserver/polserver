/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"
#include <assert.h>
#include <string.h>

#include "clib/endian.h"

#include "polcfg.h"
#include "ucrypto.h"

/* 
Workings of Client -> Server Encryption
=======================================
The first four bytes sent by the client, to either a loginserver or
a gameserver, is an encryption seed.  Sometimes this is the client's
IP address, sometimes it's another number.  Anyway, these bytes
are used to transform the master key into the first 64 bytes of
an XOR key.  The second 64 bytes are the ones-complement of the 
first.

The session key ends up being a 128 byte XOR key, but I duplicate
the key, so I have a 256-byte key.  This gives some
small performance gain, which I bet doesn't matter.  The cost is
192 bytes per client.  
*/


ClientEncryptionEngine::ClientEncryptionEngine() :
	key_idx(0)
{
	memset( xor_key, 0, sizeof xor_key );
	memset( cryptseed, 0, sizeof cryptseed );
}

/*
// Client version 1.25.33
static unsigned char ClientEncryptionEngine::basetable[64] = 
{
    0xAA, 0x93, 0x0F, 0xC1, 0xA6, 0x95, 0x8C, 0x80,   0x86, 0x05, 0x44, 0xE4, 0xB4, 0x1C, 0x48, 0x62, 
	0x77, 0xFD, 0xB8, 0x9A, 0x8B, 0x03, 0xC7, 0xA5,   0x94, 0x8C, 0x00, 0x46, 0xE5, 0x34, 0x5C, 0xE8, 
	0xB2, 0x9F, 0x09, 0x42, 0xE7, 0xB5, 0x9C, 0x88,   0x82, 0x87, 0x85, 0x04, 0xC4, 0x24, 0x54, 0x6C, 
	0x70, 0xFE, 0x39, 0x5A, 0xEB, 0xB3, 0x9F, 0x89,   0x02, 0x47, 0xE5, 0xB4, 0x9C, 0x08, 0xC2, 0x27
};

// Client version 1.25.34
const unsigned char ClientEncryptionEngine::master_key[64] = 
{
	0xAA, 0x93, 0x0F, 0xC1, 0x26, 0xD5, 0x2C, 0x50,   0xEE,	0x31, 0xDE, 0x29, 0x52, 0xEF, 0xB1, 0x1E, 
	0x49, 0xE2, 0xB7, 0x9D, 0x88, 0x02, 0xC7, 0xA5,   0x94,	0x8C, 0x00, 0x46, 0xE5, 0x34, 0xDC, 0xA8, 
	0x92, 0x8f, 0x01, 0xC6, 0x25, 0xD4, 0xAC, 0x90,   0x8E,	0x01, 0xC6, 0x25, 0xD4, 0xAC, 0x90, 0x8E, 
	0x01, 0xC6, 0x25, 0x54, 0xEC, 0xB0, 0x9E, 0x89,   0x02,	0x47, 0xE5, 0xB4, 0x9C, 0x08, 0xC2, 0x27 
};
*/

// FIXME this should be config-file driven.
#define CLIENTVERSION 0

#if CLIENTVERSION == 0
#define MASTERKEY1 config.masterkey1
#define MASTERKEY2 config.masterkey2
#endif   
    // - Beosil & Westy
#if CLIENTVERSION == 37
#define MASTERKEY1 0x378757DC
#define MASTERKEY2 0x0595DCC6
#endif

#if CLIENTVERSION == 36
#define MASTERKEY1 0x387fc5cc
#define MASTERKEY2 0x35ce9581
#define MASTERKEY3 0x07afcc37
#define MASTERKEY4 0x021510c6
#define MASTERKEY5 0x4c3a1353
#define MASTERKEY6 0x16ef783f
#endif



#if CLIENTVERSION == 35
#define MASTERKEY1 0x383477BC
#define MASTERKEY2 0x02345CC6
#endif

#if CLIENTVERSION == 34
#define MASTERKEY1 0x38ECEDAC
#define MASTERKEY2 0x025720C6
#endif

// UOXREWRITE:init_encryption_table(gentable) copied pretty much verbatim from UOX v0.67 sources...

// This is ugly.  A 32-bit is constructed, than XORed with something,
// then half of the result is discarded! Anyway, original UOX source almost
// so needs a rewrite anyway.
// not sure what happens on big-endian architecture.
    
void ClientEncryptionEngine::setseed( const unsigned char i_cryptseed[4] )
{
	int i;
	
    unsigned long seed = (i_cryptseed[0] << 24) |
                         (i_cryptseed[1] << 16) |
                         (i_cryptseed[2] << 8) |
                         (i_cryptseed[3]);
    unsigned long mask0, mask1;

    mask0 = ((~seed ^ 0x1357) << 16)    | ((seed ^ 0xAAAA) & 0x0000FFFFLu);
    mask1 = ((seed ^ 0x43210000) >> 16) | ((~seed ^ 0xABcdFFffLu) & 0xFFFF0000Lu);

    cryptmask[0] = mask0;
    cryptmask[1] = mask1;

    /* An attempt to see what's going on here:
    unsigned long mask0a, mask1a;
    mask0a = ((~i_cryptseed[1] ^ 0x13) << 24) | 
             ((~i_cryptseed[0] ^ 0x57) << 16) |
             ((i_cryptseed[1]  ^ 0xAA) << 8 ) |
             ( i_cryptseed[0]  ^ 0xAA       );
    mask1a = ((~i_cryptseed[3] ^ 0xAB) << 24) |
             ((~i_cryptseed[2] ^ 0xCD) << 16) |
             ((i_cryptseed[3]  ^ 0x43) << 8 ) |
             ( i_cryptseed[2]  ^ 0x21       );
*/

    
    for(i = 0; i < 256; i++)
    {
        unsigned long tmask0, tmask1;
        tmask0 = mask0;
        tmask1 = mask1;

#if CLIENTVERSION == 36
        mask1  = (MASTERKEY1 >> ((5 * tmask1 * tmask1) & 0xff))
	             + (tmask1 * MASTERKEY1)
	             + (tmask0 * tmask0 * MASTERKEY2)
	             + MASTERKEY3;

        
        mask0 =  (MASTERKEY4 >> ((3 * tmask0 * tmask0) & 0xff))
	             + (tmask0 * MASTERKEY4)
	             - (mask1 * mask1 * MASTERKEY5)
	             + MASTERKEY6;
#else
        if (config.client_version >= 37)
        {
            mask1 = (((((tmask1 >> 1) | (tmask0 << 31)) ^ MASTERKEY1) >> 1) | (tmask0 << 31)) ^ MASTERKEY1;
            mask0 = ((tmask0 >> 1) | (tmask1 << 31)) ^ MASTERKEY2;
        }
        else
        {
            mask1 = ((tmask1 >> 1) | (tmask0 << 31)) ^ MASTERKEY1;
            mask0 = ((tmask0 >> 1) | (tmask1 << 31)) ^ MASTERKEY2;
        }
#endif
        xor_key[i] = (unsigned char) tmask0; 
    }

/*        
	unsigned long mod;
    cryptseed[0] = i_cryptseed[0];
	cryptseed[1] = i_cryptseed[1];
	cryptseed[2] = i_cryptseed[2];
	cryptseed[3] = i_cryptseed[3];

	mod = 0;
	memcpy( xor_key, master_key, sizeof xor_key );

	for( i = 0; i < 64; i++ )
	{
		switch( i )
		{
			case 0x20: case 0x30:
				mod += cryptseed[0] << 8;
				break;
			case 0x18: case 0x28:
				mod += cryptseed[1] << 8;
				break;
			case 0x00: case 0x10:
				mod += cryptseed[2] << 8;
				break;
			case 0x08: case 0x38:
				mod += cryptseed[3] << 8;
				break;
			default:
				break;
		}
		xor_key[i] ^= (unsigned char) (mod & 0xFF);
		mod >>= 1;
	}
	
	for( i = 0; i < 64; i++ )
	{
		if (i == 0x31)
			mod += 0x100;
		xor_key[i] ^= (unsigned char) (mod & 0xFF);
		mod >>= 1;
	}
*/
	for( i = 0; i < 64; i++ )
	{
		xor_key[ i+64 ] = xor_key[ i ] ^ 0xFF;
	}

	memcpy( &xor_key[128], &xor_key[0], 128 );

	key_idx = 0;
}

void ClientEncryptionEngine::encrypt( unsigned char *dst,
									  const unsigned char *src,
									  unsigned nbytes )
{
#if CLIENTVERSION == 36
	while (nbytes--)
	{
             unsigned int mask[2];
             mask[0] = cryptmask[0];
             mask[1] = cryptmask[1];

             cryptmask[1] =
                    (MASTERKEY1 >> ((5 * mask[1] * mask[1]) & 0xff))
                    + (mask[1] * MASTERKEY1)
                    + (mask[0] * mask[0] * MASTERKEY2)
                    + MASTERKEY3;
             cryptmask[0] =
                    (MASTERKEY4 >> ((3 * mask[0] * mask[0]) & 0xff))
                    + (mask[0] * MASTERKEY4)
                    - (cryptmask[1] * cryptmask[1] * MASTERKEY5)
                    + MASTERKEY6;

        *dst = *src ^ (unsigned char) mask[0];

		++dst;
		++src;
	}
#elif CLIENTVERSION == 0
        while (nbytes--)
        {
            unsigned int mask[2];
            mask[0] = cryptmask[0];
            mask[1] = cryptmask[1];

            cryptmask[1] = (((((mask[1] >> 1) | (mask[0] << 31)) ^ MASTERKEY1) >> 1) | (mask[0] << 31)) ^ MASTERKEY1;
            cryptmask[0] = ((mask[0] >> 1) | (mask[1] << 31)) ^ MASTERKEY2;
            *dst = *src ^ (unsigned char) mask[0];

	        ++dst;
	        ++src;
        }
#else
        while (nbytes--)
	    {
		    *dst = *src ^ xor_key[ key_idx ];

		    ++dst;
		    ++src;
		    ++key_idx; // NOTE, automatically wraps from 255 to 0.
	    }
#endif
}
