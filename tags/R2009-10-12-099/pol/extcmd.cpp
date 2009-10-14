/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"

#include <stddef.h>

#include "../clib/fdump.h"
#include "../clib/endian.h"

#include "msghandl.h"
#include "pktin.h"
#include "uvars.h"
#include "extcmd.h"

struct ExtMsgHandler {
	void (*func)(Client *client, PKTIN_12 *msg);
};

static ExtMsgHandler handler_table[ 256 ];

ExtendedMessageHandler::ExtendedMessageHandler( UEXTMSGID submsgtype,
												void (*func)(Client *client, PKTIN_12 *msg) )
{
	if (handler_table[ submsgtype ].func)
	{
		fprintf( stderr, "Extended Message Handler %d (0x%x) multiply defined.\n", 
					submsgtype, submsgtype );
		throw runtime_error( "Extended Message Handler multiply defined." );
	}
	handler_table[ submsgtype ].func = func;
}

void handle_extended_cmd( Client *client, PKTIN_12 *msg )
{
	u16 msglen = cfBEu16( msg->msglen );

	if (msglen > sizeof *msg)			// sanity-check the message.
		return;
	if (msg->data[ msglen - offsetof(PKTIN_12,data) - 1] != '\0')				// the string must be null-terminated.
		return;

	if (handler_table[ msg->submsgtype ].func)
		handler_table[ msg->submsgtype ].func(client,msg);
}

MESSAGE_HANDLER_VARLEN( PKTIN_12, handle_extended_cmd );
