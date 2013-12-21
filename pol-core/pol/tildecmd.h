/*
History
=======


Notes
=======

*/
namespace Pol {
  namespace Network {
    class Client;
  }
  namespace Core {
    bool process_tildecommand( Network::Client* client, const char* textbuf );
    bool process_tildecommand( Network::Client* client, const u16* wtextbuf );
  }
}