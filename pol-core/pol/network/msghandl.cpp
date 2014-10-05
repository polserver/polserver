#include "msghandl.h"

#include "../../clib/passert.h"
#include "../network/client.h"

namespace Pol {
    namespace Network {

        PacketRegistry pktRegistry;
        
        MSG_HANDLER handler[256];
        MSG_HANDLER handler_v2[256];

        MessageHandler::MessageHandler(unsigned char msgtype,
            int msglen,
            PktHandlerFunc func)
        {
            passert(msglen != 0);
            pktRegistry.set_handler(msgtype, msglen, func);
        }

        MessageHandler_V2::MessageHandler_V2(unsigned char msgtype,
            int msglen,
            PktHandlerFunc func)
        {
            passert(msglen != 0);
            pktRegistry.set_handler(msgtype, msglen, func, PacketVersion::V2);
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