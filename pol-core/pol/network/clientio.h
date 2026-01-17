#pragma once


namespace Pol::Network
{
class Client;
void transmit( Client* client, const void* data, int len );
}  // namespace Pol::Network
