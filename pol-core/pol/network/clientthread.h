#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

namespace Pol::Network
{
class Client;
class ThreadedClient;
}  // namespace Pol::Network

namespace Pol::Core
{
// What a wait on a client's socket reported. Whoever does the waiting fills
// this in: a client with its own thread polls its own socket, while login
// clients are all waited on together by the listener that owns them.
struct SocketReadiness
{
  bool incoming = false;
  bool writable = false;
  bool error = false;
  // polled, but the socket had nothing to say
  bool timed_out = false;
};

// Runs a connected client's socket until it disconnects. Returns false, having
// cleaned the client up, once it is done.
bool client_io_thread( Network::Client* client );

// Runs one pass for a client still on the login server, using readiness the
// caller has already collected. Returns false, having cleaned the client up,
// if it is finished with the login server.
bool client_io_login_step( Network::Client* client, SocketReadiness ready );

bool process_data( Network::ThreadedClient* client );
bool check_inactivity( Network::ThreadedClient* session );

void handle_unknown_packet( Network::ThreadedClient* session );
void handle_undefined_packet( Network::ThreadedClient* session );
void handle_humongous_packet( Network::ThreadedClient* session, unsigned int reported_size );
}  // namespace Pol::Core

#endif  // CLIENTTHREAD_H
