/*
History
=======


Notes
=======

*/

#ifndef TIPLIST_H
#define TIPLIST_H

#include <vector>
#include <string>

namespace Pol {
  namespace Network {
    class Client;
  }
  namespace Core {
	void load_tips();

	bool send_tip( Network::Client* client, const char* tipname );

  }
}
#endif
