
#include "ucfg.h"

#include "../pol/cfgrepos.h"
namespace Pol {
  namespace Core {

	ConfigurationBuffer configurationbuffer;

	ConfigurationBuffer::ConfigurationBuffer() :
	  cfgfiles(),
	  oldcfgfiles()
	{
	}

	ConfigurationBuffer::~ConfigurationBuffer()
	{}

	void ConfigurationBuffer::deinialize()
	{
	  // ToDo: we have to think over... it's a problem with script-inside references
	  oldcfgfiles.clear();
	  cfgfiles.clear();
	}
  }
}