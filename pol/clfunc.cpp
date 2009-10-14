/*
History
=======
2009-03-03 Nando - private_say_above_cl(), say_above_cl(), send_sysmessage_cl() Crash fix when arguments are NULL

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
#include "pktoutid.h"
#include "sockio.h"
#include "clfunc.h"

void send_sysmessage_cl(Client *client, /*Character *chr_from, ObjArray* oText,*/
						unsigned int cliloc_num, const u16 *arguments,
						unsigned short font, unsigned short color )
{
    PKTOUT_C1 msg;
	unsigned textlen = 0, msglen;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
		textlen += 1;
	}

	if (textlen > (sizeof msg.arguments / sizeof(msg.arguments[0])))
		textlen = (sizeof msg.arguments / sizeof(msg.arguments[0]));
	msglen = offsetof( PKTOUT_C1, arguments ) + textlen*sizeof(msg.arguments[0]);

	if (msglen <= sizeof msg)
	{
	    msg.msgtype = PKTOUT_C1_ID;
		msg.msglen = ctBEu16( msglen );
		msg.serial = 0xFFFFFFFF;
		msg.body = 0xFFFF;
		msg.type = 6;
		msg.hue = ctBEu16( color );
		msg.font = ctBEu16( font );
		msg.msgnumber = ctBEu32( cliloc_num );
		memset( msg.name, '\0', sizeof msg.name );
	    strcpy( msg.name, "System" );
		for(unsigned i=0; i < textlen; i++)
			msg.arguments[i] = ctLEu16(arguments[i]);
		msg.arguments[textlen] = (u16)0L;
		transmit( client, &msg, msglen );
	}
    else
    {
		// MuadDib - FIXME need to handle this better, boooo.
		Log( "send_sysmessage: unicode text is too long\n");
    }

}

void say_above_cl(UObject *obj, unsigned int cliloc_num,
				  const u16 *arguments, unsigned short font,
				  unsigned short color )
{
    PKTOUT_C1 msg;
	unsigned textlen = 0, msglen;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
		textlen += 1;
	}
	if (textlen > (sizeof msg.arguments / sizeof(msg.arguments[0])))
		textlen = (sizeof msg.arguments / sizeof(msg.arguments[0]));
	msglen = offsetof( PKTOUT_C1, arguments ) + textlen*sizeof(msg.arguments[0]);

	if (msglen <= sizeof msg)
	{
	    msg.msgtype = PKTOUT_C1_ID;
		msg.msglen = ctBEu16( msglen );
		msg.serial = obj->serial_ext;
		msg.body = obj->graphic_ext;
		msg.type = 7;
		msg.hue = ctBEu16( color );
		msg.font = ctBEu16( font );
		msg.msgnumber = ctBEu32( cliloc_num );
		memset( msg.name, '\0', sizeof msg.name );
	    strcpy( msg.name, "System" );
		for(unsigned i=0; i < textlen; i++)
			msg.arguments[i] = ctLEu16(arguments[i]);
		msg.arguments[textlen] = (u16)0L;

		//  MuadDib - FIXME: only send to those that I'm visible to. 
	    transmit_to_inrange( obj, &msg, msglen, false, false );
	}
    else
    {
		// MuadDib - FIXME: need to handle this better
        Log( "send_sysmessage: unicode text is too long\n");
    }

}

void private_say_above_cl(Character *chr, const UObject* obj,
						unsigned int cliloc_num, const u16 *arguments,
						unsigned short font, unsigned short color )
{
	PKTOUT_C1 msg;
	unsigned textlen = 0, msglen;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
		textlen += 1;
	}
	if (textlen > (sizeof msg.arguments / sizeof(msg.arguments[0])))
		textlen = (sizeof msg.arguments / sizeof(msg.arguments[0]));
	msglen = offsetof( PKTOUT_C1, arguments ) + textlen*sizeof(msg.arguments[0]);

	if (msglen <= sizeof msg)
	{
	    msg.msgtype = PKTOUT_C1_ID;
		msg.msglen = ctBEu16( msglen );
		msg.serial = obj->serial_ext;
		msg.body = obj->graphic_ext;
		msg.type = 7;
		msg.hue = ctBEu16( color );
		msg.font = ctBEu16( font );
		msg.msgnumber = ctBEu32( cliloc_num );
		memset( msg.name, '\0', sizeof msg.name );
	    strcpy( msg.name, "System" );
		for(unsigned i=0; i < textlen; i++)
			msg.arguments[i] = ctLEu16(arguments[i]);
		msg.arguments[textlen] = (u16)0L;
		chr->client->transmit( &msg, msglen );
	}
    else
    {
		// MuadDib - FIXME: need to handle this better
        Log( "send_sysmessage: unicode text is too long\n");
    }
}


void send_sysmessage_cl_affix(Client *client, unsigned int cliloc_num, const char* affix,
							  bool prepend, const u16 *arguments, unsigned short font,
							  unsigned short color )
{
    PKTOUT_CC msg;

	unsigned textlen = 0, affix_len = 0, msglen;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
		textlen += 1; // to have L'\0' in the array ;) 
	}

	affix_len = strlen(affix)+1; // to have space for '\0'

	size_t offset = offsetof( PKTOUT_CC, affix ); // Position of msg.affix

	msglen = offset + affix_len*sizeof(msg.affix[0]) + textlen*sizeof(msg.arguments[0]);

    msg.msgtype = PKTOUT_CC_ID;
	msg.msglen = ctBEu16( msglen );
	msg.serial = 0xFFFFFFFF;
	msg.body = 0xFFFF;
	msg.type = 6;
	msg.hue = ctBEu16( color );
	msg.font = ctBEu16( font );
	msg.msgnumber = ctBEu32( cliloc_num );
	msg.flags = (prepend) ? 1 : 0;
	memset( msg.name, '\0', sizeof msg.name );
    strcpy( msg.name, "System" );

	char *tmp_msg = new char[msglen];
	
	memset(tmp_msg, '\0', msglen);
	memcpy(tmp_msg, &msg, offset); // copy until before affix
	memcpy(tmp_msg + offset, affix, affix_len);

	u16 *args = (u16*)(tmp_msg + offset + affix_len); // args will now point after the end of affix :)
	for (unsigned i=0; i < textlen; i++)
		args[i] = ctBEu16(arguments[i]);

	client->transmit(tmp_msg, msglen);

	delete [] tmp_msg;
	tmp_msg = NULL;
}

void say_above_cl_affix(UObject *obj, unsigned int cliloc_num, const char* affix,
						bool prepend, const u16 *arguments,
						unsigned short font, unsigned short color) 
{
	PKTOUT_CC msg;

	unsigned textlen = 0, affix_len = 0, msglen;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
		textlen += 1; // to have L'\0' in the array ;) 
	}

	affix_len = strlen(affix)+1; // to have space for '\0'

	size_t offset = offsetof( PKTOUT_CC, affix ); // Position of msg.affix

	msglen = offset + affix_len*sizeof(msg.affix[0]) + textlen*sizeof(msg.arguments[0]);

    msg.msgtype = PKTOUT_CC_ID;
	msg.msglen = ctBEu16( msglen );
	msg.serial = obj->serial_ext;
	msg.body = obj->graphic_ext;
	msg.type = 7;
	msg.hue = ctBEu16( color );
	msg.font = ctBEu16( font );
	msg.msgnumber = ctBEu32( cliloc_num );
	msg.flags = (prepend) ? 1 : 0;
	memset( msg.name, '\0', sizeof msg.name );
    strcpy( msg.name, "System" );

	char *tmp_msg = new char[msglen]; // Create buffer for the msg	
	
	memset(tmp_msg, '\0', msglen);
	memcpy(tmp_msg, &msg, offset); // copy until before affix
	memcpy(tmp_msg + offset, affix, affix_len);

	u16 *args = (u16*)(tmp_msg + offset + affix_len); // args will now point after the end of affix :)
	for (unsigned i=0; i < textlen; i++)
		args[i] = ctBEu16(arguments[i]);

	// MuadDib - FIXME: only send to those that I'm visible to.
	transmit_to_inrange( obj, tmp_msg, msglen, false, false );

	delete [] tmp_msg;
	tmp_msg = NULL;
}

void private_say_above_cl_affix(Character *chr, const UObject* obj, unsigned int cliloc_num,
								const char* affix, bool prepend, const u16 *arguments,
								unsigned short font, unsigned short color) 
{
	PKTOUT_CC msg;

	unsigned textlen = 0, affix_len = 0, msglen;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
		textlen += 1; // to have L'\0' in the array ;) 
	}

	affix_len = strlen(affix)+1; // to have space for '\0'

	size_t offset = offsetof( PKTOUT_CC, affix ); // Position of msg.affix

	msglen = offset + affix_len*sizeof(msg.affix[0]) + textlen*sizeof(msg.arguments[0]);

    msg.msgtype = PKTOUT_CC_ID;
	msg.msglen = ctBEu16( msglen );
	msg.serial = obj->serial_ext;
	msg.body = obj->graphic_ext;
	msg.type = 7;
	msg.hue = ctBEu16( color );
	msg.font = ctBEu16( font );
	msg.msgnumber = ctBEu32( cliloc_num );
	msg.flags = (prepend) ? 1 : 0;
	memset( msg.name, '\0', sizeof msg.name );
    strcpy( msg.name, "System" );

	char *tmp_msg = new char[msglen]; // Create buffer for the msg	
	
	memset(tmp_msg, '\0', msglen);
	memcpy(tmp_msg, &msg, offset); // copy until before affix
	memcpy(tmp_msg + offset, affix, affix_len);

	u16 *args = (u16*)(tmp_msg + offset + affix_len); // args will now point after the end of affix :)
	for (unsigned i=0; i < textlen; i++)
		args[i] = ctBEu16(arguments[i]);

	chr->client->transmit(tmp_msg, msglen);

	delete [] tmp_msg;
	tmp_msg = NULL;
}

unsigned char* build_sysmessage_cl(unsigned* msglen,unsigned int cliloc_num, const u16 *arguments,
						unsigned short font, unsigned short color )
{
	unsigned textlen = 0;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
		textlen += 1;
	}

	if (textlen > (SPEECH_MAX_LEN+1))
		textlen = SPEECH_MAX_LEN+1;
	*msglen = offsetof( PKTOUT_C1, arguments ) + textlen*2;

	PKTOUT_C1 msg;
	msg.msgtype = PKTOUT_C1_ID;
	msg.msglen = ctBEu16( *msglen );
	msg.serial = 0xFFFFFFFF;
	msg.body = 0xFFFF;
	msg.type = 6;
	msg.hue = ctBEu16( color );
	msg.font = ctBEu16( font );
	msg.msgnumber = ctBEu32( cliloc_num );
	memset( msg.name, '\0', sizeof msg.name );
	strcpy( msg.name, "System" );

	// dont know why the heck this works but "normal" way leads to a crash on delete
	unsigned char *tmp_msg = new unsigned char[*msglen]; // Create buffer for the msg	
	
	memset(tmp_msg, '\0', *msglen);
	memcpy(tmp_msg, &msg, offsetof( PKTOUT_C1, arguments )); // copy until before arguments

	u16 *args = (u16*)(tmp_msg + offsetof( PKTOUT_C1, arguments ));
	for (unsigned i=0; i < textlen; i++)
		args[i] = ctBEu16(arguments[i]);

	return tmp_msg;
}

unsigned char* build_sysmessage_cl_affix(unsigned* msglen,unsigned int cliloc_num, const char* affix,
							  bool prepend, const u16 *arguments, unsigned short font,
							  unsigned short color )
{
	PKTOUT_CC msg;

	unsigned textlen = 0, affix_len = 0;

	if (arguments != NULL)
	{
		while( arguments[textlen] != L'\0' )
			++textlen;
		textlen += 1; // to have L'\0' in the array ;) 
	}

	affix_len = strlen(affix)+1; // to have space for '\0'

	size_t offset = offsetof( PKTOUT_CC, affix ); // Position of msg.affix

	*msglen = offset + affix_len*sizeof(msg.affix[0]) + textlen*sizeof(msg.arguments[0]);

    msg.msgtype = PKTOUT_CC_ID;
	msg.msglen = ctBEu16( *msglen );
	msg.serial = 0xFFFFFFFF;
	msg.body = 0xFFFF;
	msg.type = 6;
	msg.hue = ctBEu16( color );
	msg.font = ctBEu16( font );
	msg.msgnumber = ctBEu32( cliloc_num );
	msg.flags = (prepend) ? 1 : 0;
	memset( msg.name, '\0', sizeof msg.name );
    strcpy( msg.name, "System" );

	unsigned char *tmp_msg = new unsigned char[*msglen]; // Create buffer for the msg	
	
	memset(tmp_msg, '\0', *msglen);
	memcpy(tmp_msg, &msg, offset); // copy until before affix
	memcpy(tmp_msg + offset, affix, affix_len);

	u16 *args = (u16*)(tmp_msg + offset + affix_len); // args will now point after the end of affix :)
	for (unsigned i=0; i < textlen; i++)
		args[i] = ctBEu16(arguments[i]);

	return tmp_msg;

}