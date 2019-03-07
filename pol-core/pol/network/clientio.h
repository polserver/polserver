#pragma once

namespace Pol
{
namespace Network
{
class Client;
void transmit( Client* client, const void* data, int len );
}  // namespace Network
}  // namespace Pol