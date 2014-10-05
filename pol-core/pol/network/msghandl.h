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

    enum class PacketVersion {
        V1 = 1, // The default, used by the earlier client versions
        V2 = 2, // Used to handle packets which were redefined after 6.0.1.5
        Default = V1
    };
  
    typedef void(*PktHandlerFunc)(Client *client, void *msg);

	class MessageHandler
	{
	public:
	  MessageHandler( unsigned char msgtype,
					  int msglen,
                      PktHandlerFunc func);
	};
#define MSGLEN_2BYTELEN_DATA -2

#define MESSAGE_HANDLER( type, func ) \
  Network::MessageHandler type##_handler( type##_ID, sizeof( type ), reinterpret_cast<Network::PktHandlerFunc>( func ) )

#define MESSAGE_HANDLER_VARLEN( type, func ) \
  Network::MessageHandler type##_handler( type##_ID, MSGLEN_2BYTELEN_DATA, (Network::PktHandlerFunc) func )

	class MessageHandler_V2
	{
	public:
	  MessageHandler_V2( unsigned char msgtype,
						 int msglen,
                         PktHandlerFunc func );
	};

#define MESSAGE_HANDLER_V2( type, func ) \
  Network::MessageHandler_V2 type##_handler_v2( type##_ID, sizeof( type ), reinterpret_cast<Network::PktHandlerFunc>( func ) )

#define MESSAGE_HANDLER_VARLEN_V2( type, func ) \
  Network::MessageHandler_V2 type##_handler_v2( type##_ID, MSGLEN_2BYTELEN_DATA, (Network::PktHandlerFunc) func )

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
    extern MSG_HANDLER handler_v2[256];

    // This class will be responsible for tracking the handlers. For now, it's only using the previously 
    // defined arrays.
    class PacketRegistry {
    public:
        PktHandlerFunc get_callback(unsigned char msgid, PacketVersion version = PacketVersion::V1);

        int msglen(unsigned char msgid);
        int msglen_v2(unsigned char msgid);
                
        void set_handler(unsigned char msgid, int len, PktHandlerFunc func, PacketVersion version = PacketVersion::V1);
        
        bool is_defined(unsigned char msgid);

        // This finds the appropriate handler for the client
        MSG_HANDLER find_handler(unsigned char msgid, const Client *client);
        void handle_msg(unsigned char msgid, Client *client, void *data);
    };
    extern PacketRegistry pktRegistry;
  }
}
#endif
