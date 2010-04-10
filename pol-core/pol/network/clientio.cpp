/*
History
=======
2009/12/04 Turley:    Crypto cleanup - Tomi

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include "../ctable.h"
#include "../uvars.h"
#include "../sockio.h"

#define OPT_LOG_CLIENT_DATA 0

#include "../config.h"

#include "../../clib/logfile.h"
#include "../../clib/fdump.h"
#include "../../clib/passert.h"

#include "client.h"
#include "iostats.h"
#include "packethooks.h"
#include "../polsig.h"
#include "../polstats.h"
#include "../ucfg.h"

string Client::ipaddrAsString() const
{
    return AddressToString( const_cast<struct sockaddr*>(&ipaddr) );
}

void Client::recv_remaining( int total_expected)
{
	int count;
    int max_expected = total_expected - bytes_received;

	count = cryptengine->Receive(&buffer[bytes_received],max_expected,csocket);

	if (count > 0)
	{
        passert( count <= max_expected );
		
		bytes_received += count;
        counters.bytes_received += count;
        polstats.bytes_received += count;
	}
    else if (count == 0) // graceful close
    {
        disconnect = true;
    }
    else 
    {
        int errn = socket_errno;
        if (errn != SOCKET_ERRNO( EWOULDBLOCK ))
            disconnect = true;
    }
}

void Client::recv_remaining_nocrypt( int total_expected)
{
	int count;

	count=recv(csocket, 
		       (char *) &buffer[ bytes_received ], 
			   total_expected - bytes_received, 0);

	if (count > 0)
    {
        bytes_received += count;
        counters.bytes_received += count;
        polstats.bytes_received += count;
    }
    else if (count == 0) // graceful close
    {
        disconnect = true;
    }
    else
    {
        int errn = socket_errno;
        if (errn != SOCKET_ERRNO( EWOULDBLOCK ))
            disconnect = true;
    }
}


/* NOTE: If this changes, code in client.cpp must change - pause() and restart() use
   pre-encrypted values of 33 00 and 33 01.
*/
void Client::transmit_encrypted( const void *data, int len )
{
    THREAD_CHECKPOINT( active_client, 100 );
	const unsigned char *cdata = (const unsigned char *) data;
	unsigned char *pch;
	int i;
	int bidx; // Offset in output byte
	
	pch = xoutbuffer;
	bidx=0;
    THREAD_CHECKPOINT( active_client, 101 );
	for (i=0; i < len; i++)
	{
        THREAD_CHECKPOINT( active_client, 102 );
		unsigned char ch = cdata[i];
		int nbits = keydesc[ ch ].nbits;
		unsigned short inval = keydesc[ ch ].bits_reversed;

        THREAD_CHECKPOINT( active_client, 103 );

		while (nbits--)
		{
            THREAD_CHECKPOINT( active_client, 104 );
			*pch <<= 1;
			if (inval & 1) *pch |= 1;
			bidx++;
			if (bidx == 8)
			{
                THREAD_CHECKPOINT( active_client, 105 );
				pch++;
				bidx = 0;
			}
            THREAD_CHECKPOINT( active_client, 106 );

			inval >>= 1;
		}
        THREAD_CHECKPOINT( active_client, 107 );

	}
    THREAD_CHECKPOINT( active_client, 108 );
	do {
		int nbits = keydesc[ 0x100 ].nbits;
		unsigned short inval = keydesc[ 0x100 ].bits_reversed;

        THREAD_CHECKPOINT( active_client, 109 );

		while (nbits--)
		{
            THREAD_CHECKPOINT( active_client, 110 );
			*pch <<= 1;
			if (inval & 1) *pch |= 1;
			bidx++;
			THREAD_CHECKPOINT( active_client, 111 );
            if (bidx == 8)
			{
				pch++;
				bidx = 0;
			}
	        THREAD_CHECKPOINT( active_client, 112 );

			inval >>= 1;
		}
	} while (0);
    THREAD_CHECKPOINT( active_client, 113 );

	if (bidx==0)
	{
		pch--;
	}
	else
	{
		*pch <<= (8-bidx);
	}
    THREAD_CHECKPOINT( active_client, 114 );

    passert( pch-xoutbuffer+1 <= int(sizeof xoutbuffer) );
    THREAD_CHECKPOINT( active_client, 115 );
	xmit( xoutbuffer, static_cast<unsigned short>(pch-xoutbuffer+1) );
    THREAD_CHECKPOINT( active_client, 116 );
}
#include "../packetscrobj.h"
void Client::transmit( const void *data, int len )
{
    ref_ptr<BPacket> p;
    bool handled = false;
    //see if the outgoing packet has a SendFunction installed. If so call it. It may or may not
    //want us to continue sending the packet. If it does, handled will be false, and data, len, and p
    //will be altered. data has the new packet data to send, len the new length, and p, a ref counted
    //pointer to the packet object.
    //
    //If there is no outgoing packet script, handled will be false, and the passed params will be unchanged.
    CallOutgoingPacketExportedFunction(this, data, len, p, handled);
    if(handled)
        return;

    unsigned char msgtype = * (const char*) data;

	if (fpLog != NULL)
    {
		fprintf( fpLog, "Server -> Client: 0x%X, %d bytes\n", msgtype, len );
		fdump( fpLog, data, len );
		fprintf( fpLog, "\n" );
    }

	if (last_xmit_buffer)
    {
        queuedmode_iostats.sent[ msgtype ].count++;
        queuedmode_iostats.sent[ msgtype ].bytes += len;
    }
    iostats.sent[ msgtype ].count++;
    iostats.sent[ msgtype ].bytes += len;

	if (encrypt_server_stream)
	{
        pause();
		transmit_encrypted( data, len );
	}
	else
	{
		xmit( data, static_cast<unsigned short>(len) );
		// _xmit( client->csocket, data, len );
	}
}

void Client::transmitmore( const void *data, int len )
{
    if (fpLog != NULL)
    {
        fprintf( fpLog, "Server -> Client (%d bytes)\n", len );
        fdump( fpLog, data, len );
    }
    
	if (encrypt_server_stream)
	{
        pause();
		transmit_encrypted( data, len );
	}
	else
	{
		xmit( data, static_cast<unsigned short>(len) );
		// _xmit( client->csocket, data, len );
	}
}

void transmit( Client* client, const void *data, int len )
{
    client->transmit( data, len );
}
