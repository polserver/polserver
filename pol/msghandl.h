/*
History
=======
2009/08/03 MuadDib:   Renamed secondary handler class to *_V2 for naming convention

Notes
=======

*/

#ifndef MSGHANDL_H
#define MSGHANDL_H

class Client;

class MessageHandler
{
public:
	MessageHandler( unsigned char msgtype,
					int msglen,
					void (*func)(Client *client, void *msg) );
};
#define MSGLEN_2BYTELEN_DATA -2

#define MESSAGE_HANDLER( type, func ) \
	void (*f_dummy_##type)(Client *, type *) = func; \
	MessageHandler type##_handler( type##_ID, sizeof(type), reinterpret_cast<void (*)(Client *, void *)>(func))

#define MESSAGE_HANDLER_VARLEN( type, func ) \
	void (*f_dummy_##type)(Client *, type *) = func; \
	MessageHandler type##_handler( type##_ID, MSGLEN_2BYTELEN_DATA, (void (*)(Client *, void *)) func)

class MessageHandler_V2
{
public:
	MessageHandler_V2( unsigned char msgtype,
					int msglen,
					void (*func)(Client *client, void *msg) );
};

#define MESSAGE_HANDLER_V2( type, func ) \
	void (*f_dummy_##type)(Client *, type *) = func; \
	MessageHandler_V2 type##_handler_v2( type##_ID, sizeof(type), reinterpret_cast<void (*)(Client *, void *)>(func))

#define MESSAGE_HANDLER_VARLEN_V2( type, func ) \
	void (*f_dummy_##type)(Client *, type *) = func; \
	MessageHandler_V2 type##_handler_v2( type##_ID, MSGLEN_2BYTELEN_DATA, (void (*)(Client *, void *)) func)

#endif
