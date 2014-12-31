
#include "systemstate.h"
#include "../pol/tiles.h"
#include "../clib/stlutil.h"
#include "pkg.h"

#include <sys/stat.h>

namespace Pol {
  namespace Plib {

	SystemState systemstate;


	SystemState::SystemState() :
	  packages(),
	  packages_byname(),
	  accounts_txt_dirty(false),
	  accounts_txt_stat(),
	  config(),
	  tile(nullptr),
	  tiles_loaded(false)
	{}
	SystemState::~SystemState()
	{}

	void SystemState::deinitialize()
	{
	  Clib::delete_all( packages );
	  packages_byname.clear();
	  if (tile != nullptr)
		delete[] tile;
	}
  }
}
