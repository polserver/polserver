#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

namespace Pol
{
namespace Network
{
class Client;
}
namespace Core
{
bool client_io_thread( Network::Client* client, bool login );
bool process_data( Network::Client* client );
bool check_inactivity( Network::Client* client );

void handle_unknown_packet( Network::Client* client );
void handle_undefined_packet( Network::Client* client );
void handle_humongous_packet( Network::Client* client, unsigned int reported_size );
}
}

#endif  // CLIENTTHREAD_H
