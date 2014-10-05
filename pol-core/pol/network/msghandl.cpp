#include "msghandl.h"

#include "../../clib/passert.h"
#include "../network/client.h"

namespace Pol {
    namespace Network {

        PacketRegistry pktRegistry;

        // handler[] is used for storing the core MSG_HANDLER calls.
        static MSG_HANDLER handler[256];

        /*
        handler_v2[] is used for storing the core MSG_HANDLER calls for packets that
        were changed in client 6.0.1.7 (or any newer version where a second handler is
        required due to changed incoming packet structure).
        */
        static MSG_HANDLER handler_v2[256];

        MessageHandler::MessageHandler(unsigned char msgtype,
            int msglen,
            PktHandlerFunc func,
            PacketVersion version)
        {
            passert(msglen != 0);
            pktRegistry.set_handler(msgtype, msglen, func, version);
        }

        void PacketRegistry::handle_msg(unsigned char msgid, Client *client, void *msg) {
            MSG_HANDLER msghandler = find_handler(msgid, client);
            msghandler.func(client, msg);
        }

        MSG_HANDLER PacketRegistry::find_handler(unsigned char msgid, const Client* client) {
            if (client->might_use_v2_handler() && this->msglen_v2(msgid))
                return handler_v2[msgid];
            
            return handler[msgid];
        }

        PktHandlerFunc PacketRegistry::get_callback(unsigned char msgid, PacketVersion version /*= PacketVersion::V1*/)
        {
            if (version == PacketVersion::V1)
                return handler[msgid].func;

            return handler_v2[msgid].func;
        }

        int PacketRegistry::msglen(unsigned char msgid)
        {
            return handler[msgid].msglen;
        }

        int PacketRegistry::msglen_v2(unsigned char msgid)
        {
            return handler_v2[msgid].msglen;
        }

        void PacketRegistry::set_handler(unsigned char msgid, int len, PktHandlerFunc func, PacketVersion version /*= PacketVersion::V1*/)
        {
            passert(len != 0);

            if (version == PacketVersion::V1)
            {
                handler[msgid].func = func;
                handler[msgid].msglen = len;
            }
            else {
                handler_v2[msgid].func = func;
                handler_v2[msgid].msglen = len;
            }
        }

        bool PacketRegistry::is_defined(unsigned char msgid)
        {
            return handler[msgid].msglen || handler_v2[msgid].msglen;
        }

    }
}