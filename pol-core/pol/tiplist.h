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
	typedef std::vector<std::string> TipFilenames;
	extern TipFilenames tipfilenames;

	void load_tips();

	bool send_tip( Network::Client* client, const char* tipname );

  }
}
#endif
