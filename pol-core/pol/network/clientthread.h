#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

namespace Pol::Network
{
class Client;
class ThreadedClient;
}  // namespace Pol::Network

namespace Pol::Core
{
bool client_io_thread( Network::Client* client, bool login );
bool process_data( Network::Client* client );
bool check_inactivity( Network::Client* client );

void handle_unknown_packet( Network::ThreadedClient* session );
void handle_undefined_packet( Network::ThreadedClient* session );
void handle_humongous_packet( Network::ThreadedClient* session, unsigned int reported_size );
}  // namespace Pol::Core

#endif  // CLIENTTHREAD_H
