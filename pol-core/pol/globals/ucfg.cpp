
#include "ucfg.h"

#include "../cfgrepos.h"
#include "../module/datastoreimp.h"

namespace Pol {
  namespace Core {

	ConfigurationBuffer configurationbuffer;

	ConfigurationBuffer::ConfigurationBuffer() :
	  cfgfiles(),
	  oldcfgfiles(),
	  datastore(),
	  file_access_rules()
	{
	}

	ConfigurationBuffer::~ConfigurationBuffer()
	{}

	void ConfigurationBuffer::deinitialize()
	{
	  // ToDo: we have to think over... it's a problem with script-inside references
	  oldcfgfiles.clear();
	  cfgfiles.clear();

	  for ( Core::DataStore::iterator itr = datastore.begin(); itr != datastore.end(); ++itr )
	  {
		Module::DataStoreFile* dsf = ( *itr ).second;
		delete dsf;

	  }
	  datastore.clear();
	  file_access_rules.clear();
	}
  }
}