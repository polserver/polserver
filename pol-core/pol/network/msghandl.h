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
        V2 = 2, // Used to handle packets which were redefined after 6.0.1.7
        Default = V1
    };
  
    typedef void(*PktHandlerFunc)(Client *client, void *msg);
       
    typedef struct
    {
      int msglen; // if 0, no message handler defined.
      PktHandlerFunc func;
    } MSG_HANDLER;

    // Class for keeping the message handlers
    class PacketRegistry {
    public:
        // Returns true if the message is defined in either original or v2 packet handlers
        static bool is_defined(unsigned char msgid);

        // Returns the message length (or 0 if undefined)
        static int msglen(unsigned char msgid);
        
        // Returns the message length (or 0 if undefined) for a v2 packet
        static int msglen_v2(unsigned char msgid);
           
        // Registers a handler for a certain message type
        static void set_handler(unsigned char msgid, int len, PktHandlerFunc func, PacketVersion version = PacketVersion::V1);

        // Returns the callback function for the handler of a certain message type
        static PktHandlerFunc get_callback(unsigned char msgid, PacketVersion version = PacketVersion::V1);

        // Finds the appropriate handler for a message type according to client version
        static MSG_HANDLER find_handler(unsigned char msgid, const Client *client);

        // Handles the specific message type by invoking the callback according to client version
        static void handle_msg(unsigned char msgid, Client *client, void *data);
    };

    // Helper class for registering message handlers
    class MessageHandler
    {
    public:
        MessageHandler(unsigned char msgtype, int msglen, PktHandlerFunc func, PacketVersion version);
    };

    // Preprocessor macros for registering message handlers:
    //    example:
    //               MESSAGE_HANDLER( PKTBI_2C, handle_resurrect_menu );
    //
    //  will register the fixed-length packet 2C with callback "handle_ressurect_menu"

#define MSGLEN_2BYTELEN_DATA -2

#define MESSAGE_HANDLER( type, func ) \
  Network::MessageHandler type##_handler( type##_ID, sizeof( type ), reinterpret_cast<Network::PktHandlerFunc>( func ), Network::PacketVersion::V1 )

#define MESSAGE_HANDLER_VARLEN( type, func ) \
  Network::MessageHandler type##_handler( type##_ID, MSGLEN_2BYTELEN_DATA, (Network::PktHandlerFunc) func, Network::PacketVersion::V1 )

#define MESSAGE_HANDLER_V2( type, func ) \
  Network::MessageHandler type##_handler_v2( type##_ID, sizeof( type ), reinterpret_cast<Network::PktHandlerFunc>( func ), Network::PacketVersion::V2 )

#define MESSAGE_HANDLER_VARLEN_V2( type, func ) \
  Network::MessageHandler type##_handler_v2( type##_ID, MSGLEN_2BYTELEN_DATA, (Network::PktHandlerFunc) func, Network::PacketVersion::V2 )

  }
}
#endif
