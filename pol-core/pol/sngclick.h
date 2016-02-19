#ifndef SNGCLICK_H
#define SNGCLICK_H

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Network
{
class Client;
}
namespace Core
{
void singleclick( Network::Client* client, u32 serial );
}
}

#endif
