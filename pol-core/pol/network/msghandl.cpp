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
    }
}