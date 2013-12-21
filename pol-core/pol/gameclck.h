/*
History
=======


Notes
=======

*/

#ifndef GAMECLCK_H
#define GAMECLCK_H

namespace Pol {
  namespace Core {
	typedef unsigned int gameclock_t;

	void start_gameclock();
	void stop_gameclock();
	void update_gameclock();
	gameclock_t read_gameclock();

	extern std::mutex _gameclock_mutex;
  }
}
#endif
