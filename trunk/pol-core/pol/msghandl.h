/*
History
=======
2009/08/03 MuadDib:   Renamed secondary handler class to *_V2 for naming convention

Notes
=======

*/

#ifndef MSGHANDL_H
#define MSGHANDL_H
namespace Pol {
  namespace Network {
	class Client;
  }
  namespace Core {

	class MessageHandler
	{
	public:
	  MessageHandler( unsigned char msgtype,
					  int msglen,
					  void( *func )( Network::Client *client, void *msg ) );
	};
#define MSGLEN_2BYTELEN_DATA -2

#define MESSAGE_HANDLER( type, func ) \
  void( *f_dummy_##type )( Network::Client *, type * ) = func; \
  MessageHandler type##_handler( type##_ID, sizeof( type ), reinterpret_cast<void( *)( Network::Client *, void * )>( func ) )

#define MESSAGE_HANDLER_VARLEN( type, func ) \
  void( *f_dummy_##type )( Network::Client *, type * ) = func; \
  MessageHandler type##_handler( type##_ID, MSGLEN_2BYTELEN_DATA, ( void( *)( Network::Client *, void * ) ) func )

	class MessageHandler_V2
	{
	public:
	  MessageHandler_V2( unsigned char msgtype,
						 int msglen,
						 void( *func )( Network::Client *client, void *msg ) );
	};

#define MESSAGE_HANDLER_V2( type, func ) \
  void( *f_dummy_##type )( Network::Client *, type * ) = func; \
  MessageHandler_V2 type##_handler_v2( type##_ID, sizeof( type ), reinterpret_cast<void( *)( Network::Client *, void * )>( func ) )

#define MESSAGE_HANDLER_VARLEN_V2( type, func ) \
  void( *f_dummy_##type )( Network::Client *, type * ) = func; \
  MessageHandler_V2 type##_handler_v2( type##_ID, MSGLEN_2BYTELEN_DATA, ( void( *)( Network::Client *, void * ) ) func )

    /*
    handler[] is used for storing the core MESSAGE_HANDLER
    calls.
    */
    typedef struct
    {
      int msglen; // if 0, no message handler defined.
      void( *func )( Network::Client *client, void *msg );
    } MSG_HANDLER;
    extern MSG_HANDLER handler[256];

    /*
    handler_v2[] is used for storing the core MESSAGE_HANDLER
    calls for packets that was changed in client 6.0.1.7 (or technically
    any version where a second handler is required due to changed incoming
    packet structure).
    */
    typedef struct
    {
      int msglen; // if 0, no message handler defined.
      void( *func )( Network::Client *client, void *msg );
    } MSG_HANDLER_V2;
    extern MSG_HANDLER_V2 handler_v2[256];

  }
}
#endif
