#include "msghandl.h"

#include <string> // needed for passert
#include "../../clib/passert.h"

namespace Pol {
    namespace Core {

        //PacketRegistry pktRegistry;
        MSG_HANDLER handler[256];
        MSG_HANDLER_V2 handler_v2[256];

        MessageHandler::MessageHandler(unsigned char msgtype,
            int msglen,
            PktHandlerFunc func)
        {
            passert(msglen != 0);

            handler[msgtype].func = func;
            handler[msgtype].msglen = msglen;
        }

        MessageHandler_V2::MessageHandler_V2(unsigned char msgtype,
            int msglen,
            PktHandlerFunc func)
        {
            passert(msglen != 0);

            handler_v2[msgtype].func = func;
            handler_v2[msgtype].msglen = msglen;
        }

    }
}