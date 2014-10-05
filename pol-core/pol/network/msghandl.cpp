#include "msghandl.h"

#include "../../clib/passert.h"
#include "../network/client.h"

namespace Pol {
    namespace Core {

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
            pktRegistry.set_handler_v2(msgtype, msglen, func);
        }

        MSG_HANDLER PacketRegistry::find_handler(unsigned char msgid, const Network::Client* client) {
            if (client->might_use_v2_handler() && this->msglen_v2(msgid))
                return handler_v2[msgid];
            
            return handler[msgid];
        }

        MSG_HANDLER PacketRegistry::get_handler(unsigned char msgid)
        {
            return handler[msgid];
        }

        MSG_HANDLER PacketRegistry::get_handler_v2(unsigned char msgid)
        {
            return handler_v2[msgid];
        }

        PktHandlerFunc PacketRegistry::get_handler_callback(unsigned char msgid)
        {
            return handler[msgid].func;
        }

        PktHandlerFunc PacketRegistry::get_handler_callback_v2(unsigned char msgid)
        {
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

        void PacketRegistry::set_handler(unsigned char msgid, int len, PktHandlerFunc func)
        {
            passert(len != 0);
            handler[msgid].func = func;
            handler[msgid].msglen = len;
        }

        void PacketRegistry::set_handler_v2(unsigned char msgid, int len, PktHandlerFunc func)
        {
            passert(len != 0);
            handler_v2[msgid].func = func;
            handler_v2[msgid].msglen = len;
        }

        bool PacketRegistry::is_defined(unsigned char msgid)
        {
            return handler[msgid].msglen || handler_v2[msgid].msglen;
        }

    }
}