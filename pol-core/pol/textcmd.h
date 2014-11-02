/*
History
=======


Notes
=======

*/
#ifndef __TEXTCMD_H
#define __TEXTCMD_H

#include <cstddef>
#include "../clib/rawtypes.h"

namespace Pol {
    namespace Network {
        class Client;
    }

    namespace Core {
        bool process_command(Network::Client *client, const char *text, const u16* wtext = nullptr, const char* lang = nullptr);
    }
}
#endif
