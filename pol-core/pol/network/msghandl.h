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

      typedef void(*PktHandlerFunc)(Network::Client *client, void *msg);

	class MessageHandler
	{
	public:
	  MessageHandler( unsigned char msgtype,
					  int msglen,
                      PktHandlerFunc func);
	};
#define MSGLEN_2BYTELEN_DATA -2

#define MESSAGE_HANDLER( type, func ) \
  MessageHandler type##_handler( type##_ID, sizeof( type ), reinterpret_cast<PktHandlerFunc>( func ) )

#define MESSAGE_HANDLER_VARLEN( type, func ) \
  MessageHandler type##_handler( type##_ID, MSGLEN_2BYTELEN_DATA, (PktHandlerFunc) func )

	class MessageHandler_V2
	{
	public:
	  MessageHandler_V2( unsigned char msgtype,
						 int msglen,
                         PktHandlerFunc func );
	};

#define MESSAGE_HANDLER_V2( type, func ) \
  MessageHandler_V2 type##_handler_v2( type##_ID, sizeof( type ), reinterpret_cast<PktHandlerFunc>( func ) )

#define MESSAGE_HANDLER_VARLEN_V2( type, func ) \
  MessageHandler_V2 type##_handler_v2( type##_ID, MSGLEN_2BYTELEN_DATA, (PktHandlerFunc) func )

    /*
    handler[] is used for storing the core MESSAGE_HANDLER
    calls.
    */
    typedef struct
    {
      int msglen; // if 0, no message handler defined.
      PktHandlerFunc func;
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
      PktHandlerFunc func;
    } MSG_HANDLER_V2;
    extern MSG_HANDLER_V2 handler_v2[256];

    // This class will be responsible for tracking the handlers. For now, it's only using the previously 
    // defined arrays.
    class PacketRegistry {
        MSG_HANDLER get_handler(unsigned char msgid) {
            return handler[msgid];
        }

        MSG_HANDLER_V2 get_handler_v2(unsigned char msgid) {
            return handler_v2[msgid];
        }

        bool isDefined(unsigned char msgid) {
            return get_handler(msgid).msglen || get_handler_v2(msgid).msglen;
        }
    };

  }
}
#endif
