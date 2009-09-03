/*
History
=======


Notes
=======

*/

#ifndef __UCRYPTO_H
#define __UCRYPTO_H

class Client;

class ClientEncryptionEngine
{
public:
	ClientEncryptionEngine();
	
	void setseed( const unsigned char cryptseed[4] );
	
	void encrypt( unsigned char *dst, const unsigned char *src, unsigned int nbytes );
	void decrypt( unsigned char *dst, const unsigned char *src, unsigned int nbytes ) { encrypt(dst, src, nbytes ); }

protected:
	unsigned char cryptseed[4];

	unsigned char xor_key[256];
	unsigned char key_idx;

	static const unsigned char master_key[ 64 ];

    unsigned long cryptmask[2];

};

void svr_decrypt( unsigned char *dst, int *lenout, 
				  const unsigned char *src, int lenin );

#endif
