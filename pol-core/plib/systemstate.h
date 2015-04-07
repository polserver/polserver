
#ifndef PLIB_SYTEMSTATE_H
#define PLIB_SYTEMSTATE_H

#include <boost/noncopyable.hpp>
#include <map>
#include <vector>
#if !defined(_WIN32)
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "../clib/rawtypes.h"
#include "../clib/maputil.h"
#include "../pol/polcfg.h"


namespace Pol {
  namespace Core {
	class Tile;
  }
  namespace Plib {
	class Package;

	typedef std::vector<Package*> Packages;
	typedef std::map<std::string, Package*, Clib::ci_cmp_pred> PackagesByName;

	class SystemState : boost::noncopyable
	{
	public:
	  SystemState();
	  ~SystemState();
	  void setExecutable(std::string name);
	  void setWorkingDirectrory(std::string dir);
	  std::string getWorkingDirectory();
	  std::string getStartTime();
	  void deinitialize();

	  Packages packages;
	  PackagesByName packages_byname;

	  bool accounts_txt_dirty;
	  struct stat accounts_txt_stat;
	  
	  Core::PolConfig config;
	  std::string executable;
	  std::string working_dir;
	  std::string start_time;
	  Core::Tile *tile;
	  bool tiles_loaded;

      size_t estimatedSize() const;

	private:
	
	};

	extern SystemState systemstate;
  }
}
#endif
