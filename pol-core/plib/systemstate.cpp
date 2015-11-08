
#include "systemstate.h"
#include "../pol/tiles.h"
#include "../clib/stlutil.h"
#include "../clib/Debugging/LogSink.h"
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
	{
		start_time = Pol::Clib::Logging::LogSink::getTimeStamp();
	}
	SystemState::~SystemState()
	{}

	std::string SystemState::getStartTime()
	{
		return start_time;
	}

	void SystemState::deinitialize()
	{
	  Clib::delete_all( packages );
	  packages_byname.clear();
	  if (tile != nullptr)
		delete[] tile;
	}

    size_t SystemState::estimatedSize() const
    {
      size_t size = sizeof(SystemState);
      size += (config.max_tile_id + 1)*sizeof(Core::Tile);

      size += 3 * sizeof(Package**)+packages.capacity() * sizeof( Package* );
      for (const auto& pkg : packages )
        if (pkg != nullptr)
          size += pkg->estimateSize();

      for (const auto& pkg_pair : packages_byname)
        size += pkg_pair.first.capacity() + sizeof(Package*) + ( sizeof(void*) * 3 + 1 ) / 2;
      return size;
    }
  }
}
