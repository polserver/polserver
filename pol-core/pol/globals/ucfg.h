/*
History
=======
2009/11/11 Turley:   Changed MAX_CONTAINER_ITEMS too highest possible value (3200)

Notes
=======

*/

#ifndef UCFG_H
#define UCFG_H

#include "../../clib/maputil.h"
#include "../../clib/refptr.h"

#include "../module/fileaccess.h"

#include <boost/noncopyable.hpp>
#include <vector>
#include <map>
#include <string>

namespace Pol {
  namespace Module {
	class DataStoreFile;
  }
  namespace Core {
  class StoredConfigFile;
  typedef ref_ptr<StoredConfigFile> ConfigFileRef;

  typedef std::map<std::string, ConfigFileRef> CfgFiles;
  typedef std::vector<std::string> OldCfgFiles; // we've multiple older instances

  typedef std::map< std::string, Module::DataStoreFile*, Clib::ci_cmp_pred > DataStore;
  // used for Pol and RunEcl

  class ConfigurationBuffer : boost::noncopyable
  {
	public:
	  ConfigurationBuffer();
	  ~ConfigurationBuffer();

	  void deinitialize();

	  CfgFiles cfgfiles;
	  OldCfgFiles oldcfgfiles;
	  DataStore datastore;
	  std::vector<Module::FileAccess> file_access_rules;
	private:
  };

  extern ConfigurationBuffer configurationbuffer;
  }
}

#endif
