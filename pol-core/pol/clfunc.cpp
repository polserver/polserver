/*
History
=======
2009-03-03 Nando - private_say_above_cl(), say_above_cl(), send_sysmessage_cl() Crash fix when arguments are NULL
2009/12/04 Turley: if arguments are NULL still add the terminator

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <assert.h>
#include <stddef.h>

#include "../clib/clib.h"
#include "../clib/endian.h"
#include "../clib/logfile.h"
#include "../clib/passert.h"

#include "mobile/charactr.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/clienttransmit.h"
#include "pktoutid.h"
#include "sockio.h"
#include "clfunc.h"

void send_sysmessage_cl(Client *client, /*Character *chr_from, ObjArray* oText,*/
						unsigned int cliloc_num, const u16 *arguments,
						unsigned short font, unsigned short color )
{
	PktOut_C1* msg = REQUESTPACKET(PktOut_C1,PKTOUT_C1_ID);
	msg->offset+=2;
	unsigned textlen = 0;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
	}

	if (textlen > (SPEECH_MAX_LEN))
		textlen = SPEECH_MAX_LEN;

	msg->Write(static_cast<u32>(0xFFFFFFFF)); // serial
	msg->Write(static_cast<u16>(0xFFFF));     // body
	msg->Write(static_cast<u8>(6));           // type 6 lower left, 7 on player
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	msg->WriteFlipped(cliloc_num);
	msg->Write("System",30,false);
	if (arguments != NULL)
		msg->Write(arguments,static_cast<u16>(textlen),true); //ctLEu16
	else
		msg->offset+=2;
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	transmit( client, &msg->buffer, len );
	READDPACKET(msg);
}

void say_above_cl(UObject *obj, unsigned int cliloc_num,
				  const u16 *arguments, unsigned short font,
				  unsigned short color )
{
	PktOut_C1* msg = REQUESTPACKET(PktOut_C1,PKTOUT_C1_ID);
	msg->offset+=2;
	unsigned textlen = 0;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
	}

	if (textlen > (SPEECH_MAX_LEN))
		textlen = SPEECH_MAX_LEN;

	msg->Write(obj->serial_ext);
	msg->Write(obj->graphic_ext);     // body
	msg->Write(static_cast<u8>(7));           // type 6 lower left, 7 on player
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	msg->WriteFlipped(cliloc_num);
	msg->Write("System",30,false);
	if (arguments != NULL)
		msg->Write(arguments,static_cast<u16>(textlen),true); //ctLEu16
	else
		msg->offset+=2;
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	//  MuadDib - FIXME: only send to those that I'm visible to. 
	transmit_to_inrange( obj, &msg->buffer, len, false, false );
	READDPACKET(msg);
}

void private_say_above_cl(Character *chr, const UObject* obj,
						unsigned int cliloc_num, const u16 *arguments,
						unsigned short font, unsigned short color )
{
	PktOut_C1* msg = REQUESTPACKET(PktOut_C1,PKTOUT_C1_ID);
	msg->offset+=2;
	unsigned textlen = 0;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
	}

	if (textlen > (SPEECH_MAX_LEN))
		textlen = SPEECH_MAX_LEN;

	msg->Write(obj->serial_ext);
	msg->Write(obj->graphic_ext);     // body
	msg->Write(static_cast<u8>(7));           // type 6 lower left, 7 on player
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	msg->WriteFlipped(cliloc_num);
	msg->Write("System",30,false);
	if (arguments != NULL)
		msg->Write(arguments,static_cast<u16>(textlen),true); //ctLEu16
	else
		msg->offset+=2;
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	ADDTOSENDQUEUE(chr->client, &msg->buffer, len );
	READDPACKET(msg);
}


void send_sysmessage_cl_affix(Client *client, unsigned int cliloc_num, const char* affix,
							  bool prepend, const u16 *arguments, unsigned short font,
							  unsigned short color )
{
	PktOut_CC* msg = REQUESTPACKET(PktOut_CC,PKTOUT_CC_ID);
	msg->offset+=2;
	unsigned textlen = 0, affix_len = 0;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
	}
	affix_len = strlen(affix)+1;
	if (affix_len > SPEECH_MAX_LEN+1)
		affix_len = SPEECH_MAX_LEN+1;

	if (textlen > (SPEECH_MAX_LEN))
		textlen = SPEECH_MAX_LEN;

	msg->Write(static_cast<u32>(0xFFFFFFFF)); // serial
	msg->Write(static_cast<u16>(0xFFFF));     // body
	msg->Write(static_cast<u8>(6));           // type 6 lower left, 7 on player
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	msg->WriteFlipped(cliloc_num);
	msg->Write(static_cast<u8>((prepend) ? 1 : 0));
	msg->Write("System",30,false);
	msg->Write(affix,static_cast<u16>(affix_len));
	if (arguments != NULL)
		msg->WriteFlipped(arguments,static_cast<u16>(textlen),true);
	else
		msg->offset+=2;
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);

	ADDTOSENDQUEUE(client,&msg->buffer, len);
	READDPACKET(msg);
}

void say_above_cl_affix(UObject *obj, unsigned int cliloc_num, const char* affix,
						bool prepend, const u16 *arguments,
						unsigned short font, unsigned short color) 
{
	PktOut_CC* msg = REQUESTPACKET(PktOut_CC,PKTOUT_CC_ID);
	msg->offset+=2;
	unsigned textlen = 0, affix_len = 0;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
	}
	affix_len = strlen(affix)+1;
	if (affix_len > SPEECH_MAX_LEN+1)
		affix_len = SPEECH_MAX_LEN+1;

	if (textlen > (SPEECH_MAX_LEN))
		textlen = SPEECH_MAX_LEN;

	msg->Write(obj->serial_ext); // serial
	msg->Write(obj->graphic_ext);     // body
	msg->Write(7);           // type 6 lower left, 7 on player
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	msg->WriteFlipped(cliloc_num);
	msg->Write(static_cast<u8>((prepend) ? 1 : 0));
	msg->Write("System",30,false);
	msg->Write(affix,static_cast<u16>(affix_len));
	if (arguments != NULL)
		msg->WriteFlipped(arguments,static_cast<u16>(textlen),true); //ctLEu16
	else
		msg->offset+=2;
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);

	// MuadDib - FIXME: only send to those that I'm visible to.
	transmit_to_inrange( obj, &msg->buffer, len, false, false );
	READDPACKET(msg);
}

void private_say_above_cl_affix(Character *chr, const UObject* obj, unsigned int cliloc_num,
								const char* affix, bool prepend, const u16 *arguments,
								unsigned short font, unsigned short color) 
{
	PktOut_CC* msg = REQUESTPACKET(PktOut_CC,PKTOUT_CC_ID);
	msg->offset+=2;
	unsigned textlen = 0, affix_len = 0;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
	}
	affix_len = strlen(affix)+1;
	if (affix_len > SPEECH_MAX_LEN+1)
		affix_len = SPEECH_MAX_LEN+1;

	if (textlen > (SPEECH_MAX_LEN))
		textlen = SPEECH_MAX_LEN;

	msg->Write(obj->serial_ext); // serial
	msg->Write(obj->graphic_ext);     // body
	msg->Write(7);           // type 6 lower left, 7 on player
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	msg->WriteFlipped(cliloc_num);
	msg->Write(static_cast<u8>((prepend) ? 1 : 0));
	msg->Write("System",30,false);
	msg->Write(affix,static_cast<u16>(affix_len));
	if (arguments != NULL)
		msg->WriteFlipped(arguments,static_cast<u16>(textlen),true);
	else
		msg->offset+=2;
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);

	ADDTOSENDQUEUE(chr->client,&msg->buffer, len);
	READDPACKET(msg);
}

void build_sysmessage_cl(PktOut_C1* msg, unsigned int cliloc_num, const u16 *arguments,
						unsigned short font, unsigned short color )
{
	msg->offset+=2;
	unsigned textlen = 0;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
	}

	if (textlen > (SPEECH_MAX_LEN))
		textlen = SPEECH_MAX_LEN;

	msg->Write(static_cast<u32>(0xFFFFFFFF)); // serial
	msg->Write(static_cast<u16>(0xFFFF));     // body
	msg->Write(static_cast<u8>(6));           // type 6 lower left, 7 on player
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	msg->WriteFlipped(cliloc_num);
	msg->Write("System",30,false);
	if (arguments != NULL)
		msg->Write(arguments,static_cast<u16>(textlen),true); //ctLEu16
	else
		msg->offset+=2;
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	msg->offset=len;
}

void build_sysmessage_cl_affix(PktOut_CC* msg,unsigned int cliloc_num, const char* affix,
							  bool prepend, const u16 *arguments, unsigned short font,
							  unsigned short color )
{
	msg->offset+=2;
	unsigned textlen = 0, affix_len = 0;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
	}
	affix_len = strlen(affix)+1;
	if (affix_len > SPEECH_MAX_LEN+1)
		affix_len = SPEECH_MAX_LEN+1;

	if (textlen > (SPEECH_MAX_LEN))
		textlen = SPEECH_MAX_LEN;

	msg->Write(static_cast<u32>(0xFFFFFFFF)); // serial
	msg->Write(static_cast<u16>(0xFFFF));     // body
	msg->Write(static_cast<u8>(6));           // type 6 lower left, 7 on player
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	msg->WriteFlipped(cliloc_num);
	msg->Write(static_cast<u8>((prepend) ? 1 : 0));
	msg->Write("System",30,false);
	msg->Write(affix,static_cast<u16>(affix_len));
	if (arguments != NULL)
		msg->WriteFlipped(arguments,static_cast<u16>(textlen),true);
	else
		msg->offset+=2;
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	msg->offset=len;
}

